#include <lv2/lv2.h>
#include <lv2/libc.h>
#include <lv2/memory.h>
#include <lv2/patch.h>
#include <lv2/syscall.h>
#include <lv2/io.h>
#include <lv2/error.h>
#include <lv2/security.h>
#include "common.h"
#include "mappath.h"
#include "modulespatch.h"
#include "syscall8.h"
#include "ps3mapi_core.h"
#include "homebrew_blocker.h"
#include "make_rif.h"

#define MAX_TABLE_ENTRIES 32

static unsigned char crap_pants[13] = {"///no_exists"};
static int8_t avoid_recursive_calls = 0;
uint8_t photo_gui = 1;
char ntfs_iso_path[256];

typedef struct _MapEntry
{
	char *oldpath;
	char *newpath;
	int  newpath_len;
	uint32_t flags;
} MapEntry;

MapEntry map_table[MAX_TABLE_ENTRIES];

// aldostool's prevention of accidental deletion of important paths
static int init_map_entry(uint8_t index)
{
	if(map_table[index].newpath
		&& (map_table[index].newpath_len > 3)
		&& (map_table[index].newpath[0] == '/')
		&& (map_table[index].newpath[1] == '.')
		&& (map_table[index].newpath[2] == '/'))
			return 1; // protect from deletion existing newpath like "/./*"

	return SUCCEEDED;
}

// TODO: map_path and open_path_hook should be mutexed...
int map_path(char *oldpath, char *newpath, uint32_t flags)
{
    int i, firstfree = -1;

    if (!oldpath || strlen(oldpath) == 0)
        return -1;

    DPRINTF("Map path: %s -> %s\n", oldpath, newpath);

    if (newpath && strcmp(oldpath, newpath) == 0)
        newpath = NULL;

    if (strcmp(oldpath, "/dev_bdvd") == 0)
        condition_apphome = (newpath != NULL);

    for (i = 0; i < MAX_TABLE_ENTRIES; i++)
    {
        if (map_table[i].oldpath)
        {
            if (strcmp(oldpath, map_table[i].oldpath) == 0)
            {
                if (newpath && strlen(newpath))
                {
                    strncpy(map_table[i].newpath, newpath, MAX_PATH - 1);
                    map_table[i].newpath[MAX_PATH - 1] = 0;
                    map_table[i].newpath_len = strlen(newpath);
                    map_table[i].flags = (map_table[i].flags&FLAG_COPY) | (flags&(~FLAG_COPY));
                }
                else
                {
                    if(init_map_entry(i))
                        continue;

                    if (map_table[i].flags & FLAG_COPY)
                        free(map_table[i].oldpath);

                    free(map_table[i].newpath);
                    map_table[i].oldpath = NULL;
                    map_table[i].newpath = NULL;
                    map_table[i].flags = 0;
                }

                break;
            }
        }
        else if (firstfree < 0)
            firstfree = i;
    }
    
    if (i == MAX_TABLE_ENTRIES)
    {
        if (firstfree < 0)
            return EKRESOURCE;

        if (!newpath || strlen(newpath) == 0)
            return SUCCEEDED;

        map_table[firstfree].flags = flags;

        int len = strlen(oldpath); 

        if(len >= MAX_PATH)
        	return -1;
        
        char *oldpath_buf = malloc(len + 1);

        if(!oldpath_buf)
            return -1;

        if (flags & FLAG_COPY)
        {                       
            map_table[firstfree].oldpath = (char *)oldpath_buf;
            strncpy(map_table[firstfree].oldpath, oldpath, len);
            map_table[firstfree].oldpath[len] = 0;
        }
        else
            map_table[firstfree].oldpath = oldpath;

        char *newpath_buf = malloc(MAX_PATH);

        if(!newpath_buf)
        {
            free(oldpath_buf);
            return -1;
        }

        map_table[firstfree].newpath = (char *)newpath_buf;
        strncpy(map_table[firstfree].newpath, newpath, MAX_PATH - 1);
        map_table[firstfree].newpath[MAX_PATH - 1] = 0;
        map_table[firstfree].newpath_len = strlen(newpath);
    }

    return SUCCEEDED;
}

int map_path_user(char *oldpath, char *newpath, uint32_t flags)
{
	char *oldp, *newp;

	DPRINTF("map_path_user, called by process %s: %s -> %s\n", get_process_name(get_current_process_critical()), oldpath, newpath);

	if (oldpath == 0)
		return -1;

	int ret = pathdup_from_user(get_secure_user_ptr(oldpath), &oldp);
	if (ret != 0)
		return ret;

	if (newpath == 0)
		newp = NULL;
	else
	{
		ret = pathdup_from_user(get_secure_user_ptr(newpath), &newp);
		if (ret != 0)
		{
			free(oldp);
			return ret;
		}
	}

	ret = map_path(oldp, newp, flags | FLAG_COPY);

	free(oldp);

	if (newp)
		free(newp);

	return ret;
}

LV2_SYSCALL2(int, sys_map_path, (char *oldpath, char *newpath))
{
	extend_kstack(0);
	return map_path_user(oldpath, newpath, 0);
}

int sys_map_paths(char *paths[], char *new_paths[], unsigned int num)
{
	uint32_t *u_paths = (uint32_t *)get_secure_user_ptr(paths);
	uint32_t *u_new_paths = (uint32_t *)get_secure_user_ptr(new_paths);
	int unmap = 0;
	int ret = 0;

	if (!u_paths)
		unmap = 1;
	else
	{
		if (!u_new_paths)
			return EINVAL;

		for (unsigned int i = 0; i < num; i++)
		{
			ret = map_path_user((char *)(uint64_t)u_paths[i], (char *)(uint64_t)u_new_paths[i], FLAG_TABLE);
			if (ret != 0)
			{
				unmap = 1;
				break;
			}
		}
	}

	if (unmap)
	{
		for (int i = 0; i < MAX_TABLE_ENTRIES; i++)
		{
			if (map_table[i].flags & FLAG_TABLE)
			{
				if(init_map_entry(i))
					continue;

				if (map_table[i].flags & FLAG_COPY)
					free(map_table[i].oldpath);

				free(map_table[i].newpath);
				map_table[i].oldpath = NULL;
				map_table[i].newpath = NULL;
				map_table[i].flags = 0;
			}
		}
	}

	return ret;
}

static uint8_t libft2d_access = 0;
LV2_HOOKED_FUNCTION_POSTCALL_2(void, open_path_hook, (char *path0, int mode))
{
	// Avoid recursive calls by aldostools
	if(avoid_recursive_calls) 
		return;

	avoid_recursive_calls = 1;

	// Save path when a ISO is mounted from NTFS partition (By Evilnat)
	if(strstr(path0, "ntfs[PS3ISO]"))
		strcpy(ntfs_iso_path, path0);

	// Let's now block homebrews if the "allow" flag is false
	if(!block_homebrew(path0))
	{
		avoid_recursive_calls = 0;
		set_patched_func_param(1, (uint64_t)crap_pants);
		return;
	}

	make_rif(path0);
	restore_syscalls(path0);
	check_signin(path0);

	if (path0[0] == '/')
	{
		char *path = path0;

		if(path[1] == '/')
			path++;

		// Photo_GUI integration with webMAN MOD - DeViL303 & AV
		if(!libft2d_access)
			libft2d_access = photo_gui && !strcmp(path, "/dev_flash/sys/internal/libft2d.sprx");
		else
		{
			libft2d_access = 0;

			if(!strncmp(path, "/dev_hdd0/photo/", 16))
			{
				char *photo = path + 16;
				int len = strlen(photo);
				uint64_t size = (len + 16);

				if (len >= 8)
				{
					photo += len - 4;
					if(!strcmp(photo, ".PNG") || !strcmp(photo, ".JPG") || !strcmp(photo -4, "_COV.JPG") || !strncasecmp(photo -4, ".iso.jpg", 8) || !strncasecmp(photo -4, ".iso.png", 8))
					{
						//DPRINTF("CREATING /dev_hdd0/tmp/wm_request\n");

						int fd;
						if (cellFsOpen("/dev_hdd0/tmp/wm_request", CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0) == 0)
						{
							cellFsWrite(fd, path, size, &size);
							cellFsClose(fd);
						}
					}
				}
			}
		}

		for (int i = MAX_TABLE_ENTRIES - 1; i >= 0; i--)
		{
			if (map_table[i].oldpath)
			{
				int len = strlen(map_table[i].oldpath);

				if (path && strncmp(path, map_table[i].oldpath, len) == 0)
				{
					strcpy(map_table[i].newpath + map_table[i].newpath_len, path + len);

					// aldostool's partial map path
					// -- AV: use partial folder remapping when newpath starts with double '/' like //dev_hdd0/blah...
					if(map_table[i].newpath[1] == '/') // double //
					{
						CellFsStat stat;
						if(cellFsStat(map_table[i].newpath, &stat) != 0)
						{
							avoid_recursive_calls = 0;
							DPRINTF("open_path %s\n", path0);
							return; // Do not remap / Use the original file when redirected file does not exist
						}
					}

					set_patched_func_param(1, (uint64_t)map_table[i].newpath);

					break;
				}
			}
		}

		DPRINTF("open_path %s\n", path);
	}
	
	avoid_recursive_calls = 0;
}

int sys_aio_copy_root(char *src, char *dst)
{
	int len;

	src = get_secure_user_ptr(src);
	dst = get_secure_user_ptr(dst);

	// Begin original function implementation
	if (!src)
		return EFAULT;

	len = strlen(src);

	if (len >= 0x420 || len <= 1 || src[0] != '/')
		return EINVAL;

	strcpy(dst, src);

	for (int i = 1; i < len; i++)
	{
		if (dst[i] == 0)
			break;

		if (dst[i] == '/')
		{
			dst[i] = 0;
			break;
		}
	}

	if (strlen(dst) >= 0x20)
		return EINVAL;

	// Here begins custom part of the implementation
	if (condition_apphome && (strcmp(dst, "/dev_bdvd") == 0)) // if dev_bdvd and jb game mounted
	{
		// find /dev_bdvd
		for (int i = 0; i < MAX_TABLE_ENTRIES; i++)
		{
			if (map_table[i].oldpath && strcmp(map_table[i].oldpath, "/dev_bdvd") == 0)
			{
				for (int j = 1; j < map_table[i].newpath_len; j++)
				{
					dst[j] = map_table[i].newpath[j];

					if (dst[j] == 0)
						break;

					if (dst[j] == '/')
					{
						dst[j] = 0;
						break;
					}
				}

				//DPRINTF("AIO: root replaced by %s\n", dst);

				break;
			}
		}
	}

	return SUCCEEDED;
}

void unhook_all_map_path(void)
{
	suspend_intr();
	unhook_function_with_postcall(open_path_symbol, open_path_hook, 2);
	resume_intr();
}

void map_path_patches(int syscall)
{
	hook_function_with_postcall(open_path_symbol, open_path_hook, 2);

	if (syscall)
		create_syscall2(SYS_MAP_PATH, sys_map_path);
}
