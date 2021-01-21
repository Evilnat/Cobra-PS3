#include <lv2/lv2.h>
#include <lv2/libc.h>
#include <lv2/interrupt.h>
#include <lv2/modules.h>
#include <lv2/process.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/pad.h>
#include <lv2/symbols.h>
#include <lv2/patch.h>
#include <lv2/error.h>
#include <lv2/security.h>
#include <lv2/thread.h>
#include <lv2/syscall.h>
#include <lv1/lv1.h>
#include <lv1/lv1call.h>
#include <lv1/stor.h>
#include <lv1/patch.h>

#define MAX_VSH_PLUGINS 			7
#define BOOT_PLUGINS_FILE			"/dev_hdd0/boot_plugins_nocobra.txt"
#define BOOT_PLUGINS_KERNEL_FILE	"/dev_hdd0/boot_plugins_kernel_nocobra.txt"
#define BOOT_PLUGINS_FIRST_SLOT 	1
#define MAX_BOOT_PLUGINS			(MAX_VSH_PLUGINS-BOOT_PLUGINS_FIRST_SLOT)
#define MAX_BOOT_PLUGINS_KERNEL		5

#define PS2EMU_STAGE2_FILE			"/dev_hdd0/vm/pm0"

process_t vsh_process;
sys_prx_id_t vsh_plugins[MAX_VSH_PLUGINS];
uint8_t safe_mode;

int read_text_line(int fd, char *line, unsigned int size, int *eof)
{
	int i = 0;
	int line_started = 0;

	if (size == 0)
		return -1;

	*eof = 0;

	while (i < (size-1))
	{
		uint8_t ch;
		uint64_t r;

		if (cellFsRead(fd, &ch, 1, &r) != 0 || r != 1)
		{
			*eof = 1;
			break;
		}

		if (!line_started)
		{
			if (ch > ' ')
			{
				line[i++] = (char)ch;
				line_started = 1;
			}
		}
		else
		{
			if (ch == '\n' || ch == '\r')
				break;

			line[i++] = (char)ch;
		}
	}

	line[i] = 0;

	// Remove space chars at end
	for (int j = i-1; j >= 0; j--)
	{
		if (line[j] <= ' ')
		{
			line[j] = 0;
			i = j;
		}
		else		
			break;		
	}

	return i;
}

uint64_t load_plugin_kernel(char *path)
{
	CellFsStat stat;
	int file;
	int (* func)(void);
	uint64_t read;

	if(cellFsStat(path, &stat)==0)
	{
		if(stat.st_size>4)
		{
			if(cellFsOpen(path, CELL_FS_O_RDONLY, &file, 0, NULL, 0)==0)
			{
				void *skprx=alloc(stat.st_size,0x27);
				if(skprx)
				{
					if(cellFsRead(file, skprx, stat.st_size, &read)==0)
					{	
						f_desc_t f;
						f.addr=skprx;
						f.toc=(void *)MKA(TOC);
						func=(void *)&f;
						func();
						uint64_t resident=(uint64_t)skprx;
						return resident;
					}
					else					
						dealloc(skprx, 0x27);					
				}
			}
		}
	}
	return -1;
}

void load_boot_plugins_kernel(void)
{
	int fd;
	int current_slot_kernel = 0;
	int num_loaded_kernel = 0;
	
	if (cellFsOpen(BOOT_PLUGINS_KERNEL_FILE, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == 0)
	{
		while (num_loaded_kernel < MAX_BOOT_PLUGINS_KERNEL)
		{
			char path[128];
			int eof;			
			
			if (read_text_line(fd, path, sizeof(path), &eof) > 0)
			{
				uint64_t ret = load_plugin_kernel(path);
					
				if (ret >= 0)
				{
					current_slot_kernel++;
					num_loaded_kernel++;
				}			
			}
			
			if (eof)
				break;
		}

		cellFsClose(fd);
	}
}

int prx_load_vsh_plugin(unsigned int slot, char *path)
{
	sys_prx_id_t prx;
	int ret = -1;	
	
	if (slot >= MAX_VSH_PLUGINS)
		return ret;
	
	if (vsh_plugins[slot] != 0)	
		return ret;	
	
	prx = prx_load_module(vsh_process, 0, 0, path);
	
	if (prx < 0)
		return prx;	
	
	ret = prx_start_module_with_thread(prx, vsh_process, 0, 0);
	
	if (ret == 0)	
		vsh_plugins[slot] = prx;	
	else
	{
		prx_stop_module_with_thread(prx, vsh_process, 0, 0);
		prx_unload_module(prx, vsh_process);
	}
	
	return ret;
}

void load_boot_plugins(void)
{
	int fd;
	int current_slot = BOOT_PLUGINS_FIRST_SLOT;
	int num_loaded = 0;
	
	if (cellFsOpen(BOOT_PLUGINS_FILE, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) != 0)
		return;
	
	while (num_loaded < MAX_BOOT_PLUGINS)
	{
		char path[128];
		int eof;
		
		if (read_text_line(fd, path, sizeof(path), &eof) > 0)
		{
			int ret = prx_load_vsh_plugin(current_slot, path);
			
			if (ret >= 0)
			{
				current_slot++;
				num_loaded++;
			}
		}
		
		if (eof)
			break;
	}
	
	cellFsClose(fd);
}

enum
{
	PS2EMU_HW,
	PS2EMU_GX,
	PS2EMU_SW
};

static INLINE int get_ps2emu_type(void)
{
	uint8_t config[8];
	u64 v2;

	lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("sys", 0), FIELD("hw", 0), FIELD("config", 0), 0, (u64 *)config, &v2);

	if (config[6] & 1) // has emotion engine	
		return PS2EMU_HW;	
	else if (config[0] & 0x20) // has graphics synthesizer	
		return PS2EMU_GX;	

	return PS2EMU_SW;
}

char *ps2emu_stage2[] =
{
	"ps2hwemu_stage2.bin",
	"ps2gxemu_stage2.bin",
};

char *ps2_files[] =
{
	PS2EMU_STAGE2_FILE,
	"/dev_hdd0/tmp/loadoptical",
};

void copy_emus(int emu_type)
{
	char name[64];
	int src, dst;
	uint8_t *buf;

	if (emu_type < 0 || emu_type > PS2EMU_GX)
		return;

	page_allocate_auto(NULL, 0x10000, 0x2F, (void **)&buf);

	sprintf(name, "/dev_flash/ps2emu/%s", ps2emu_stage2[emu_type]);

	if (cellFsOpen(name, CELL_FS_O_RDONLY, &src, 0, NULL, 0) == 0)
	{
		uint64_t size;

		cellFsRead(src, buf, 0x10000, &size);
		cellFsClose(src);

		for(int i=0; i < 2; i++)
		{
			if(cellFsOpen(ps2_files[i], CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &dst, 0666, NULL, 0)==0)
			{
				cellFsWrite(dst, buf, size, &size);
				cellFsClose(dst);
				size=4;
			}
		}
		
	}

	page_free(NULL, buf, 0x2F);
}

LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_8(int, post_cellFsUtilMount, (const char *block_dev, const char *filesystem, const char *mount_point, int unk, int read_only, int unk2, char *argv[], int argc))
{
	if (strcmp(mount_point, "/dev_hdd0") == 0 && strcmp(filesystem, "CELL_FS_UFS") == 0)
	{
		unhook_function_on_precall_success(cellFsUtilMount_symbol, post_cellFsUtilMount, 8);
		
		if(vsh_process)
		{
			copy_emus(get_ps2emu_type());
			load_boot_plugins_kernel();
			load_boot_plugins();
		}
		else if(safe_mode) //our safe mode works only when it detects emer_init loading
		{
			cellFsUnlink(BOOT_PLUGINS_KERNEL_FILE);
			cellFsUnlink(BOOT_PLUGINS_FILE);
		}
			
	}
	return 0;
}

LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_8(int, load_process_hooked, (process_t process, int fd, char *path, int r6, uint64_t r7, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t sp_70))
{
	if (strcmp(path, "/dev_flash/vsh/module/vsh.self") == 0)
	{		
		vsh_process = process;
		unhook_function_on_precall_success(load_process_symbol, load_process_hooked, 9);
	}	
	else if (strcmp(path, "emer_init.self") == 0)	
		safe_mode = 1;		
	
	return 0;
}

void main(void)
{
	hook_function_on_precall_success(load_process_symbol, load_process_hooked, 9);
	hook_function_on_precall_success(cellFsUtilMount_symbol, post_cellFsUtilMount, 8);
}
