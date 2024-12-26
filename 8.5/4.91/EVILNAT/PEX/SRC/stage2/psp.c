#include <lv2/lv2.h>
#include <lv2/error.h>
#include <lv2/io.h>
#include <lv2/memory.h>
#include <lv2/modules.h>
#include <lv2/object.h>
#include <lv2/thread.h>
#include <lv2/synchronization.h>
#include <lv2/time.h>
#include <lv2/libc.h>
#include <lv2/patch.h>
#include "common.h"
#include "psp.h"
#include "modulespatch.h"

#define SPRX_NUM			90
#define NUM_SCE_PSP_MODULES	5

uint32_t psp_tag;
uint8_t psp_keys[16];
uint8_t psp_code;
//char pspemu_path[36];
//char psptrans_path[37];

static uint32_t base_offset = 0;

static char *sce_psp_modules[NUM_SCE_PSP_MODULES] =
{
	"sceFont_Library",
	"scePsmfP_library",
	"sceHeap_Library",
	"sceMpeg_library",
	"sceATRAC3plus_Library",
};

char *umd_file;
static mutex_t mutex;
static int user_mutex;
static int umd_fd=-1;
static char psp_id[11];
static uint64_t pemucorelib_base;
static uint64_t emulator_api_base;
static uint32_t *savedata_param;

/* vsh.self */
static SprxPatch psp_drm_patches[] =
{
	{ cex_psp_drm_patch1, LI(R3,  0) 	},
	{ cex_psp_drm_patch2, LI(R3,  0) 	},
	{ cex_psp_drm_patch3, LI(R3,  0) 	},
	{ cex_psp_drm_patch4, LI(R0,  0) 	},
	{ cex_psp_drm_patchA, LI(R0,  0) 	},
	{ cex_psp_drm_patchB, LI(R31, 0) 	},
	{ cex_psp_drm_patchC, LI(R3,  0) 	},	
	{ cex_psp_drm_patchD, NOP 			},
	{ cex_psp_drm_patchE, LI(R3,  0) 	},
	{ cex_psp_drm_patchF, LI(R3,  0) 	},
	{ 0 }
};

static SprxPatch psp_drm_dex_patches[] =
{
	{ dex_psp_drm_patch1, LI(R3,  0) 	},
	{ dex_psp_drm_patch2, LI(R3,  0) 	},
	{ dex_psp_drm_patch3, LI(R3,  0) 	},
	{ dex_psp_drm_patch4, LI(R0,  0) 	},
	{ dex_psp_drm_patchA, LI(R0,  0) 	},
	{ dex_psp_drm_patchB, LI(R31, 0) 	},
	{ dex_psp_drm_patchC, LI(R3,  0) 	},	
	{ dex_psp_drm_patchD, NOP 			},
	{ dex_psp_drm_patchE, LI(R3,  0) 	},
	{ dex_psp_drm_patchF, LI(R3,  0) 	},
	{ 0 }
};

static SprxPatch *patches_backup;

static INLINE uint32_t swap32(uint32_t data)
{
	uint32_t ret = (((data) & 0xff) << 24);
	ret |= (((data) & 0xff00) << 8);
	ret |= (((data) & 0xff0000) >> 8);
	ret |= (((data) >> 24) & 0xff);

	return ret;
}

static INLINE uint16_t swap16(uint16_t data)
{
	uint32_t ret = (data << 8) & 0xFF00;
	ret |= ((data >> 8) & 0xFF);

	return ret;
}

static INLINE __attribute__((unused)) int is_sce_psp_module(char *name)
{
	for (int i = 0; i < NUM_SCE_PSP_MODULES; i++)
	{
		if (strcmp(sce_psp_modules[i], name) == 0)
			return 1;
	}

	return SUCCEEDED;
}

int sys_psp_read_header(int fd, char *buf, uint64_t nbytes, uint64_t *nread)
{
	int ret;
	uint32_t n, unk2;
	uint64_t umd_size;
	sys_prx_id_t *list;
	uint32_t *unk;
	process_t process;

	//DPRINTF("umd read header: %p %lx\n", buf, nbytes);

	buf = get_secure_user_ptr(buf);
	nread = get_secure_user_ptr(nread);

	if (!umd_file || nbytes != 0x100)
		return EABORT;

	pemucorelib_base = 0;
	emulator_api_base = 0;
	list = kalloc(SPRX_NUM * sizeof(sys_prx_module_info_t));
	unk = kalloc(SPRX_NUM * sizeof(uint32_t));
	process = get_current_process();

	ret = prx_get_module_list(process, list, unk, SPRX_NUM, &n, &unk2);
	if (ret == 0)
	{
		char *filename = kalloc(256);
		sys_prx_segment_info_t *segments = kalloc(sizeof(sys_prx_segment_info_t));

		for (int i = 0; i < n; i++)
		{
			sys_prx_module_info_t modinfo;

			memset(&modinfo, 0, sizeof(sys_prx_module_info_t));
			modinfo.filename_size = 256;
			modinfo.segments_num = 1;

			if (prx_get_module_info(process, list[i], &modinfo, filename, segments) == 0)
			{
				if (strstr(filename, "/emulator_api.sprx"))
				{
					emulator_api_base = segments[0].base;
					//DPRINTF("emulator_api base = %08lx\n", emulator_api_base);
				}
				else if (strstr(filename, "/PEmuCoreLib.sprx"))
				{
					pemucorelib_base = segments[0].base;
					//DPRINTF("PEmuCoreLib base = %08lx\n", pemucorelib_base);
				}
			}
		}

		if (pemucorelib_base == 0 || emulator_api_base == 0)
			ret = EABORT;

		kfree(filename);
		kfree(segments);
	}

	kfree(list);
	kfree(unk);

	if (ret)
		return ret;

	ret = cellFsOpen(umd_file, CELL_FS_O_RDONLY, &umd_fd, 0, NULL, 0);
	if (ret)
		return ret;

	cellFsLseek(umd_fd, 0, SEEK_END, &umd_size);

	umd_size -= base_offset;

	// Fake header. We will write only values actually used
	memset(buf, 0, 0x100);
	*(uint32_t *)(buf + 0x0c) = 0x10;
	*(uint32_t *)(buf + 0x64) = (umd_size / 0x800) - 1; // Last sector of umd
	strncpy(buf + 0x70, psp_id, 10);

	//DPRINTF("ID: %s\n", psp_id);

	if (mutex && user_mutex)
	{
		mutex = NULL;
		user_mutex = 0;
	}

	*nread = 0x100;
	return SUCCEEDED;
}

int sys_psp_read_umd(int unk, void *buf, uint64_t sector, uint64_t ofs, uint64_t size)
{
	uint64_t offset, dummy;
	int ret;

	//DPRINTF("umd read %lx %lx %lx\n", sector, ofs, size);

	if (!mutex)
	{
		object_handle_t obj_handle;
		void *object_table = get_current_process()->object_table;

		int ret = open_kernel_object(object_table, *(uint32_t *)(emulator_api_base+umd_mutex_offset), (void **)&mutex, &obj_handle, SYS_MUTEX_OBJECT);

		if (ret)
		{
			//DPRINTF("Cannot open user mutex, using an own one\n");
			mutex_create(&mutex, SYNC_PRIORITY, SYNC_NOT_RECURSIVE);
			user_mutex = 0;
		}
		else
		{
			//DPRINTF("user mutex opened succesfully\n");
			user_mutex = 1;
			close_kernel_object_handle(object_table, obj_handle);
		}
	}

	mutex_lock(mutex, 0);
	offset = sector * 0x800;

	if (ofs)
		offset = offset + 0x800 - ofs;

	ret = cellFsLseek(umd_fd, base_offset + offset, SEEK_SET, &dummy);
	if (ret)
	{
		mutex_unlock(mutex);
		return ret;
	}

	ret = cellFsRead(umd_fd, get_secure_user_ptr(buf), size, &dummy);
	mutex_unlock(mutex);

	if (ret == 0)
		ret = (int)size;

	return ret;
}

int sys_psp_set_umdfile(char *file, char *id, int prometheus)
{
	base_offset = 0;

	int ret;

	file = get_secure_user_ptr(file);
	id = get_secure_user_ptr(id);

	if (file == NULL)
	{
		if (umd_file)
		{
			free(umd_file);
			umd_file = NULL;
		}
		if (mutex)
		{
			if (user_mutex)
				user_mutex = 0;
			else
				mutex_destroy(mutex);

			mutex = NULL;
		}
		if (patches_backup)
		{
			for (int i = 0; patches_backup[i].offset != 0; i++)
				copy_to_process(vsh_process, &patches_backup[i].data, (void *)(uint64_t)(0x10000 + patches_backup[i].offset), 4);

			free(patches_backup);
			patches_backup = NULL;
		}

		condition_psp_iso = 0;
		condition_psp_dec = 0;
		condition_psp_keys = 0;
		condition_psp_prometheus = 0;
		return SUCCEEDED;
	}

	strncpy(psp_id, id, 10);
	psp_id[10] = 0;

	if (strlen(psp_id) != 10)
		return EINVAL;

	ret = pathdup_from_user(file, &umd_file);
	if (ret)
		return ret;

	int len = strlen(umd_file);

	if(len > 4)	
		if(strcmp(file + (len - 4), ".PNG") == 0) 
			base_offset = 0x10000;	

	condition_psp_iso = 1;
	condition_psp_prometheus = prometheus;

	if (prometheus)
	{
		//DPRINTF("Using prometheus patch.\n");
	}

	if (!patches_backup)
	{
		switch(vsh_check)
		{
			case VSH_CEX_HASH:
				//DPRINTF("Now patching PSP DRM In Retail VSH..\n");

				patches_backup = malloc(sizeof(psp_drm_patches));

				memcpy(patches_backup, &psp_drm_patches, sizeof(psp_drm_patches));

				for (int i = 0; psp_drm_patches[i].offset != 0; i++)
				{
					//DPRINTF("Offset: 0x%08X | Data: 0x%08X\n", (uint32_t)psp_drm_patches[i].offset, (uint32_t)psp_drm_patches[i].data);

					copy_from_process(vsh_process, (void *)(uint64_t)(0x10000 + patches_backup[i].offset), &patches_backup[i].data, 4);

					if (copy_to_process(vsh_process, &psp_drm_patches[i].data, (void *)(uint64_t)(0x10000 + psp_drm_patches[i].offset), 4) != 0)
						fatal("copy_to_process failed, you forgot to make vsh text writable, retard!\n");
				}
			break;

			case VSH_DEX_HASH:
				//DPRINTF("Now patching PSP DRM In DEBUG VSH..\n");

				patches_backup = alloc(sizeof(psp_drm_dex_patches), 0x27);
			
				memcpy(patches_backup, &psp_drm_dex_patches, sizeof(psp_drm_dex_patches));
					
				for (int i = 0; psp_drm_dex_patches[i].offset != 0; i++)
				{
					//DPRINTF("Offset: 0x%08X | Data: 0x%08X\n", (uint32_t)psp_drm_dex_patches[i].offset, (uint32_t)psp_drm_dex_patches[i].data);
			
					copy_from_process(vsh_process, (void *)(uint64_t)(0x10000+patches_backup[i].offset), &patches_backup[i].data, 4);
				
					if (copy_to_process(vsh_process, &psp_drm_dex_patches[i].data, (void *)(uint64_t)(0x10000+psp_drm_dex_patches[i].offset), 4) != 0)					
						fatal("copy_to_process failed, you forgot to make vsh text writable, retard!\n");					
				}
			break;

			default:
				//DPRINTF("Unknown VSH HASH, PSP DRM was not patched!\n");
				break;
		}
	}

	return SUCCEEDED;
}

int sys_psp_set_decrypt_options(int decrypt_patch, uint32_t tag, uint8_t *keys, uint8_t code, uint32_t tag2, uint8_t *keys2, uint8_t code2)
{
	if (!umd_file)
		return EABORT;

	keys = get_secure_user_ptr(keys);

	if (keys)
	{
		if (copy_from_user(keys, psp_keys, 0x10) == 0)
		{
			psp_tag = tag;
			psp_code = code;
			condition_psp_keys = 1;
		}
	}

	condition_psp_dec = decrypt_patch;
	return SUCCEEDED;
}

int sys_psp_prx_patch(uint32_t *unk, uint8_t *elf_buf, void *link_register)
{
	unk = get_secure_user_ptr(unk);
	elf_buf = get_secure_user_ptr(elf_buf);

#ifdef DEBUG
	//DPRINTF("link_register = %p\n", link_register);

	if (link_register == (void *)(pemucorelib_base+prx_patch_call_lr))
	{
		if (*(uint32_t *)&elf_buf[0] == 0x7F454C46)
		{
			Elf32_Ehdr *ehdr;
			Elf32_Phdr *phdr;
			PspModuleInfo *modinfo;

			ehdr = (Elf32_Ehdr *)elf_buf;
			phdr = (Elf32_Phdr *)(elf_buf + swap32(ehdr->e_phoff));
			modinfo = (PspModuleInfo *)(elf_buf + swap32(phdr[0].p_paddr));
			//DPRINTF("Module %s (elf_buf=%p)\n", modinfo->modname, elf_buf);
		}
	}
#endif

	psp_func1(unk, elf_buf);
	return SUCCEEDED;
}

/*int sys_psp_set_emu_path(char *path)
{
	if (!path)
	{
		condition_psp_change_emu = 0;
		return SUCCEEDED;
	}

	//DPRINTF("sys_psp_set_emu_path has been deleted\n");

	//DPRINTF("pspemu path set to %s\n", path);

	//path = get_secure_user_ptr(path);
	//condition_psp_change_emu = 1;

	//snprintf(pspemu_path, sizeof(pspemu_path), "%s/psp_emulator.self", path);
	//snprintf(psptrans_path, sizeof(psptrans_path), "%s/psp_translator.self", path);

	return SUCCEEDED;
}*/

int sys_psp_post_savedata_initstart(int result, void *param)
{
	//DPRINTF("Savedata init start\n");

	if (result == 0)
		savedata_param = get_secure_user_ptr(param);

	return result;
}

int sys_psp_post_savedata_shutdownstart(void)
{
	//DPRINTF("Savedata shutdown start\n");

	if (savedata_param)
	{
		//DPRINTF("Original bind: %08X\n", savedata_param[0x34/4]);
			
		savedata_param[0x34/4] = swap32(1); // SCE_UTILITY_SAVEDATA_BIND_OK
		savedata_param = NULL;
	}

	return SUCCEEDED;
}

#ifdef DEBUG

/* mips_registers actually contains much more than mips registers
Known values:

0460 -> PC or current block to process
0478 -> PC too?
04D0 -> PC too?
0090 -> text offset in elf
0098 -> text size
*/

int sys_psp_sony_bug(uint32_t *mips_registers, void *unk, uint32_t mips_PC)
{
	DPRINTF("sys_psp_sony_bug, game is gonna crash\n");

	/*DPRINTF("PSP registers info:\n");
	DPRINTF("PC: %08X\n", mips_PC);
	DPRINTF("zr: %08X   at: %08X\n", mips_registers[0], mips_registers[1]);
	DPRINTF("v0: %08X   v1: %08X\n", mips_registers[2], mips_registers[3]);
	DPRINTF("a0: %08X   a1: %08X\n", mips_registers[4], mips_registers[5]);
	DPRINTF("a2: %08X   a3: %08X\n", mips_registers[6], mips_registers[7]);
	DPRINTF("t0: %08X   t1: %08X\n", mips_registers[8], mips_registers[9]);
	DPRINTF("t2: %08X   t3: %08X\n", mips_registers[10], mips_registers[11]);
	DPRINTF("t4: %08X   t5: %08X\n", mips_registers[12], mips_registers[13]);
	DPRINTF("t6: %08X   t7: %08X\n", mips_registers[14], mips_registers[15]);
	DPRINTF("s0: %08X   s1: %08X\n", mips_registers[16], mips_registers[17]);
	DPRINTF("s2: %08X   s3: %08X\n", mips_registers[18], mips_registers[19]);
	DPRINTF("s4: %08X   s5: %08X\n", mips_registers[20], mips_registers[21]);
	DPRINTF("s6: %08X   s7: %08X\n", mips_registers[22], mips_registers[23]);
	DPRINTF("t8: %08X   t9: %08X\n", mips_registers[24], mips_registers[25]);
	DPRINTF("k0: %08X   k1: %08X\n", mips_registers[26], mips_registers[27]);
	DPRINTF("gp: %08X   sp: %08X\n", mips_registers[28], mips_registers[29]);
	DPRINTF("s8: %08X   ra: %08X\n", mips_registers[30], mips_registers[31]);*/

	/*for (int i = 32; i < 1024; i += 2)
	{
		DPRINTF("%04X: %08X %08X\n", i*4, mips_registers[i], mips_registers[i+1]);
	}*/

	return SUCCEEDED;
}

#endif

