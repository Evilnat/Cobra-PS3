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
#include "common.h"
#include "modulespatch.h"
#include "permissions.h"
#include "crypto.h"
#include "config.h"
#include "storage_ext.h"
#include "psp.h"
#include "syscall8.h"
#include "self.h"

#define MAX_VSH_PLUGINS 			7
#define BOOT_PLUGINS_FILE			"/dev_hdd0/boot_plugins.txt"
#define BOOT_PLUGINS_KERNEL_FILE	"/dev_hdd0/boot_plugins_kernel.txt"
#define BOOT_PLUGINS_FIRST_SLOT 	1
#define MAX_BOOT_PLUGINS			(MAX_VSH_PLUGINS-BOOT_PLUGINS_FIRST_SLOT)
#define MAX_BOOT_PLUGINS_KERNEL		5
#define PRX_PATH					"/dev_flash/vsh/module/webftp_server.sprx"

LV2_EXPORT int decrypt_func(uint64_t *, uint32_t *);

typedef struct
{
	uint64_t hash;
	SprxPatch *patch_table;

} PatchTableEntry;

typedef struct
{
	uint8_t keys[16];
	uint64_t nonce;
} KeySet;


#define N_SPRX_KEYS_1 (sizeof(sprx_keys_set1)/sizeof(KeySet))

KeySet sprx_keys_set1[] =
{
	{
		{
			0xD6, 0xFD, 0xD2, 0xB9, 0x2C, 0xCC, 0x04, 0xDD,
			0x77, 0x3C, 0x7C, 0x96, 0x09, 0x5D, 0x7A, 0x3B
		},

		0xBA2624B2B2AA7461ULL
	},
};

// Keyset for pspemu, and for future vsh plugins or whatever is added later

#define N_SPRX_KEYS_2 (sizeof(sprx_keys_set2)/sizeof(KeySet))

KeySet sprx_keys_set2[] =
{
	{
		{
			0x7A, 0x9E, 0x0F, 0x7C, 0xE3, 0xFB, 0x0C, 0x09,
			0x4D, 0xE9, 0x6A, 0xEB, 0xA2, 0xBD, 0xF7, 0x7B
		},

		0x8F8FEBA931AF6A19ULL
	},

	{
		{
			0xDB, 0x54, 0x44, 0xB3, 0xC6, 0x27, 0x82, 0xB6,
			0x64, 0x36, 0x3E, 0xFF, 0x58, 0x20, 0xD9, 0x83
		},

		0xE13E0D15EF55C307ULL
	},
};


static uint8_t *saved_buf;
static void *saved_sce_hdr;

process_t vsh_process;
uint8_t safe_mode;

static uint32_t caller_process = 0;

static uint8_t condition_true = 1;
//static uint8_t condition_false = 0;
uint8_t condition_ps2softemu = 0;
uint8_t condition_apphome = 0;
uint8_t condition_disable_gameupdate = 0; // Disabled
uint8_t condition_psp_iso = 0;
uint8_t condition_psp_dec = 0;
uint8_t condition_psp_keys = 0;
uint8_t condition_psp_change_emu = 0;
uint8_t condition_psp_prometheus = 0;
uint8_t condition_pemucorelib = 1;
uint64_t vsh_check;
//uint8_t condition_game_ext_psx=0;
int bc_to_net_status=0;

//uint8_t block_peek = 0;

// Plugins
sys_prx_id_t vsh_plugins[MAX_VSH_PLUGINS];
static int loading_vsh_plugin;

SprxPatch cex_vsh_patches[] =
{
	// { cex_game_update_offset, LI(R3, -1), &condition_disable_gameupdate }, [DISABLED by DEFAULT since 4.46]
	{ cex_ps2tonet_patch, ORI(R3, R3, 0x8204), &condition_ps2softemu },
	{ cex_ps2tonet_size_patch, LI(R5, 0x40), &condition_ps2softemu },
	{ 0 }
};

SprxPatch dex_vsh_patches[] =
{
	//{ dex_game_update_offset, LI(R3, -1), &condition_disable_gameupdate },
	{ dex_ps2tonet_patch, ORI(R3, R3, 0x8204), &condition_ps2softemu },
	{ dex_ps2tonet_size_patch, LI(R5, 0x40), &condition_ps2softemu },
	//{ dex_enable_dlna_patch, LI(R4, 1), &condition_true },		
	//{ dex_fake_owner_popup_patch, NOP, &condition_true }, // Hard-coded in REX 4.81.2	
	{ 0 }
};

/*
SprxPatch basic_plugins_patches[] =
{
	//{ ps1emu_type_check_offset, NOP, &condition_true }, // Loads ps1_netemu.self
	//{ ps1emu_type_check_offset, 0x409E000C, &condition_true }, // Loads original ps1_emu.self
	{ 0 }
};
*/

SprxPatch explore_plugin_patches[] =
{
	{ app_home_offset, 0x2f646576, &condition_apphome   }, 
	{ app_home_offset+4, 0x5f626476, &condition_apphome }, 
	{ app_home_offset+8, 0x642f5053, &condition_apphome }, 
	{ ps2_nonbw_offset, LI(0, 1), &condition_ps2softemu },
	{ 0 }
};

SprxPatch explore_category_game_patches[] =
{
	{ ps2_nonbw_offset2, LI(R0, 1), &condition_ps2softemu },
	{ 0 }
};

SprxPatch bdp_disc_check_plugin_patches[] =
{
	{ dvd_video_region_check_offset, LI(R3, 1), &condition_true }, /* Kills standard dvd-video region protection (not RCE one) */
	{ 0 }
};

SprxPatch ps1_emu_patches[] =
{
	{ ps1_emu_get_region_offset, LI(R29, 0x82), &condition_true }, /* regions 0x80-0x82 bypass region check. */
	{ 0 }
};

SprxPatch ps1_netemu_patches[] =
{
	// Some rare titles such as Langrisser Final Edition are launched through ps1_netemu!
	{ ps1_netemu_get_region_offset, LI(R3, 0x82), &condition_true },
	{ 0 }
};

SprxPatch game_ext_plugin_patches[] =
{
	{ sfo_check_offset, NOP, &condition_true },
	{ ps2_nonbw_offset3, LI(R0, 1), &condition_ps2softemu },
	{ ps_region_error_offset, NOP, &condition_true }, // Needed sometimes... 
	//{ ps_video_error_offset, LI(R3, 0), &condition_game_ext_psx },
	//{ ps_video_error_offset+4, BLR, &condition_game_ext_psx }, // experimental, disabled due to its issue with remote play
	{ game_exit_popup_patch, NOP, &condition_true }, 
	{ 0 }
};

SprxPatch psp_emulator_patches[] =
{
	// Sets psp mode as opossed to minis mode. Increases compatibility, removes text protection and makes most savedata work
	{ psp_set_psp_mode_offset, LI(R4, 0), &condition_psp_iso },
	{ 0 }
};

SprxPatch emulator_api_patches[] =
{
	// Read umd patches
	{ psp_read, STDU(SP, 0xFF90, SP), &condition_psp_iso },
	{ psp_read+4, MFLR(R0), &condition_psp_iso },
	{ psp_read+8, STD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_read+0x0C, MR(R8, R7), &condition_psp_iso },
	{ psp_read+0x10, MR(R7, R6), &condition_psp_iso },
	{ psp_read+0x14, MR(R6, R5), &condition_psp_iso },
	{ psp_read+0x18, MR(R5, R4), &condition_psp_iso },
	{ psp_read+0x1C, MR(R4, R3), &condition_psp_iso },
	{ psp_read+0x20, LI(R3, SYSCALL8_OPCODE_READ_PSP_UMD), &condition_psp_iso },
	{ psp_read+0x24, LI(R11, 8), &condition_psp_iso },
	{ psp_read+0x28, SC, &condition_psp_iso },
	{ psp_read+0x2C, LD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_read+0x30, MTLR(R0), &condition_psp_iso },
	{ psp_read+0x34, ADDI(SP, SP, 0x70), &condition_psp_iso },
	{ psp_read+0x38, BLR, &condition_psp_iso },

	// Read header patches
	{ psp_read+0x3C, STDU(SP, 0xFF90, SP), &condition_psp_iso },
	{ psp_read+0x40, MFLR(R0), &condition_psp_iso },
	{ psp_read+0x44, STD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_read+0x48, MR(R7, R6), &condition_psp_iso },
	{ psp_read+0x4C, MR(R6, R5), &condition_psp_iso },
	{ psp_read+0x50, MR(R5, R4), &condition_psp_iso },
	{ psp_read+0x54, MR(R4, R3), &condition_psp_iso },
	{ psp_read+0x58, LI(R3, SYSCALL8_OPCODE_READ_PSP_HEADER), &condition_psp_iso },
	{ psp_read+0x5C, LI(R11, 8), &condition_psp_iso },
	{ psp_read+0x60, SC, &condition_psp_iso },
	{ psp_read+0x64, LD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_read+0x68, MTLR(R0), &condition_psp_iso },
	{ psp_read+0x6C, ADDI(SP, SP, 0x70), &condition_psp_iso },
	{ psp_read+0x70, BLR, &condition_psp_iso },
	{ psp_read_header, MAKE_CALL_VALUE(psp_read_header, psp_read+0x3C), &condition_psp_iso },

#if defined (FIRMWARE_4_84) || defined (FIRMWARE_4_84DEX)
	// Drm patches
	{ psp_drm_patch5, MAKE_JUMP_VALUE(psp_drm_patch5, psp_drm_patch6), &condition_psp_iso },
	{ psp_drm_patch7, LI(R6, 0), &condition_psp_iso },
	{ psp_drm_patch8, LI(R7, 0), &condition_psp_iso },
	{ psp_drm_patch9, NOP, &condition_psp_iso },
	{ psp_drm_patch11, LI(R6, 0), &condition_psp_iso },
	{ psp_drm_patch12, LI(R7, 0), &condition_psp_iso },

	// product id
	{ psp_product_id_patch1, NOP, &condition_psp_iso },
	{ psp_product_id_patch3, NOP, &condition_psp_iso },
#endif
	{ 0 }
};

SprxPatch pemucorelib_patches[] =
{
	{ psp_eboot_dec_patch, LI(R6, 0x110), &condition_psp_dec }, // -> makes unsigned psp eboot.bin run, 0x10 works too
	{ psp_prx_patch, STDU(SP, 0xFF90, SP), &condition_psp_iso },
	{ psp_prx_patch+4, MFLR(R6), &condition_psp_iso },
	{ psp_prx_patch+8, STD(R6, 0x80, SP), &condition_psp_iso },
	{ psp_prx_patch+0x0C, LI(R11, 8), &condition_psp_iso },
	{ psp_prx_patch+0x10, MR(R5, R4), &condition_psp_iso },
	{ psp_prx_patch+0x14, MR(R4, R3), &condition_psp_iso },
	{ psp_prx_patch+0x18, LI(R3, SYSCALL8_OPCODE_PSP_PRX_PATCH), &condition_psp_iso },
	{ psp_prx_patch+0x1C, SC, &condition_psp_iso },
	{ psp_prx_patch+0x20, LD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_prx_patch+0x24, MTLR(R0), &condition_psp_iso },
	{ psp_prx_patch+0x28, ADDI(SP, SP, 0x70), &condition_psp_iso },
	{ psp_prx_patch+0x2C, BLR, &condition_psp_iso },

	// Patch for savedata binding
	{ psp_savedata_bind_patch1, MR(R5, R19), &condition_psp_iso },
	{ psp_savedata_bind_patch2, MAKE_JUMP_VALUE(psp_savedata_bind_patch2, psp_prx_patch+0x30), &condition_psp_iso },
	{ psp_prx_patch+0x30, LD(R19, 0xFF98, SP), &condition_psp_iso },
	{ psp_prx_patch+0x34, STDU(SP, 0xFF90, SP), &condition_psp_iso },
	{ psp_prx_patch+0x38, MFLR(R0), &condition_psp_iso },
	{ psp_prx_patch+0x3C, STD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_prx_patch+0x40, LI(R11, 8), &condition_psp_iso },
	{ psp_prx_patch+0x44, MR(R4, R3), &condition_psp_iso },
	{ psp_prx_patch+0x48, LI(R3, SYSCALL8_OPCODE_PSP_POST_SAVEDATA_INITSTART), &condition_psp_iso },
	{ psp_prx_patch+0x4C, SC, &condition_psp_iso },
	{ psp_prx_patch+0x50, LD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_prx_patch+0x54, MTLR(R0), &condition_psp_iso },
	{ psp_prx_patch+0x58, ADDI(SP, SP, 0x70), &condition_psp_iso },
	{ psp_prx_patch+0x5C, BLR, &condition_psp_iso },
	{ psp_savedata_bind_patch3, MAKE_JUMP_VALUE(psp_savedata_bind_patch3, psp_prx_patch+0x60), &condition_psp_iso },
	{ psp_prx_patch+0x60, STDU(SP, 0xFF90, SP), &condition_psp_iso },
	{ psp_prx_patch+0x64, MFLR(R0), &condition_psp_iso },
	{ psp_prx_patch+0x68, STD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_prx_patch+0x6C, LI(R11, 8), &condition_psp_iso },
	{ psp_prx_patch+0x70, LI(R3, SYSCALL8_OPCODE_PSP_POST_SAVEDATA_SHUTDOWNSTART), &condition_psp_iso },
	{ psp_prx_patch+0x74, SC, &condition_psp_iso },
	{ psp_prx_patch+0x78, LD(R0, 0x80, SP), &condition_psp_iso },
	{ psp_prx_patch+0x7C, MTLR(R0), &condition_psp_iso },
	{ psp_prx_patch+0x80, ADDI(SP, SP, 0x70), &condition_psp_iso },
	{ psp_prx_patch+0x84, BLR, &condition_psp_iso },

	// Prometheus
	{ psp_prometheus_patch, '.OLD', &condition_psp_prometheus },

	/*#if defined(FIRMWARE_4_84) 
		// Extra save data patch required since some 3.60+ firmware
		{ psp_extra_savedata_patch, LI(R31, 1), &condition_psp_iso },
	#endif */
	{ 0 }
};

SprxPatch libsysutil_savedata_psp_patches[] =
{
#if defined(FIRMWARE_4_84) || defined(FIRMWARE_4_84DEX)
	{ psp_savedata_patch1, MAKE_JUMP_VALUE(psp_savedata_patch1, psp_savedata_patch2), &condition_psp_iso },
	{ psp_savedata_patch3, NOP, &condition_psp_iso },
	{ psp_savedata_patch4, NOP, &condition_psp_iso },
	{ psp_savedata_patch5, NOP, &condition_psp_iso },
	{ psp_savedata_patch6, NOP, &condition_psp_iso },
#endif
	{ 0 }
};

SprxPatch libfs_external_patches[] =
{
	// Redirect internal libfs function to kernel. If condition_apphome is 1, it means there is a JB game mounted
	{ aio_copy_root_offset, STDU(SP, 0xFF90, SP), &condition_apphome },
	{ aio_copy_root_offset+4, MFLR(R0), &condition_apphome },
	{ aio_copy_root_offset+8, STD(R0, 0x80, SP), &condition_apphome },
	{ aio_copy_root_offset+0x0C, MR(R5, R4), &condition_apphome },
	{ aio_copy_root_offset+0x10, MR(R4, R3), &condition_apphome },
	{ aio_copy_root_offset+0x14, LI(R3, SYSCALL8_OPCODE_AIO_COPY_ROOT), &condition_apphome },
	{ aio_copy_root_offset+0x18, LI(R11, 8), &condition_apphome },
	{ aio_copy_root_offset+0x1C, SC, &condition_apphome },
	{ aio_copy_root_offset+0x20, LD(R0, 0x80, SP), &condition_apphome },
	{ aio_copy_root_offset+0x24, MTLR(R0), &condition_apphome },
	{ aio_copy_root_offset+0x28, ADDI(SP, SP, 0x70), &condition_apphome },
	{ aio_copy_root_offset+0x2C, BLR, &condition_apphome },
	{ 0 }
};

PatchTableEntry patch_table[] =
{
	{ VSH_DEX_HASH, dex_vsh_patches },
	{ VSH_CEX_HASH, cex_vsh_patches },
	//{ BASIC_PLUGINS_HASH, basic_plugins_patches },
	{ EXPLORE_PLUGIN_HASH, explore_plugin_patches },
	{ EXPLORE_CATEGORY_GAME_HASH, explore_category_game_patches },
	{ BDP_DISC_CHECK_PLUGIN_HASH, bdp_disc_check_plugin_patches },
	{ PS1_EMU_HASH, ps1_emu_patches },
	{ PS1_NETEMU_HASH, ps1_netemu_patches },
	{ GAME_EXT_PLUGIN_HASH, game_ext_plugin_patches },
	{ PSP_EMULATOR_HASH, psp_emulator_patches },
	{ EMULATOR_API_HASH, emulator_api_patches },
	{ PEMUCORELIB_HASH, pemucorelib_patches },
	{ LIBSYSUTIL_SAVEDATA_PSP_HASH, libsysutil_savedata_psp_patches },
	{ LIBFS_EXTERNAL_HASH, libfs_external_patches },
};

#define N_PATCH_TABLE_ENTRIES	(sizeof(patch_table) / sizeof(PatchTableEntry))

#ifdef DEBUG

static char *hash_to_name(uint64_t hash)
{
    switch(hash)
	{
		case VSH_CEX_HASH:
			return "vsh.self";
		break;
		
        case VSH_DEX_HASH:
			return "vsh.self";
		break;
		
		case EXPLORE_PLUGIN_HASH:
			return "explore_plugin.sprx";
		break;
		
		case EXPLORE_CATEGORY_GAME_HASH:
			return "explore_category_game.sprx";
		break;
		
		case BDP_DISC_CHECK_PLUGIN_HASH:
			return "bdp_disccheck_plugin.sprx";
		break;
		
		case PS1_EMU_HASH:
			return "ps1_emu.self";
		break;
		
		case PS1_NETEMU_HASH:
			return "ps1_netemu.self";
		break;
		
		case GAME_EXT_PLUGIN_HASH:
			return "game_ext_plugin.sprx";
		break;
				
		case PSP_EMULATOR_HASH:
			return "psp_emulator.self";
		break;
		
		case EMULATOR_API_HASH:
			return "emulator_api.sprx";
		break;
		
		case PEMUCORELIB_HASH:
			return "PEmuCoreLib.sprx";
		break;
		
		case LIBFS_EXTERNAL_HASH:
			return "libfs.sprx";
		break;
		
		case LIBSYSUTIL_SAVEDATA_PSP_HASH:
			return "libsysutil_savedata_psp.sprx";
		break;
		
		/*case BASIC_PLUGINS_HASH:
			return "basic_plugins.sprx";
		break;*/
		
		default:
			return "UNKNOWN";
		break;		
	}
}

#endif

LV2_HOOKED_FUNCTION_PRECALL_2(int, post_lv1_call_99_wrapper, (uint64_t *spu_obj, uint64_t *spu_args))
{
	// This replaces an original patch of psjailbreak, since we need to do more things
	process_t process = get_current_process();

	saved_buf = (void *)spu_args[0x20/8];
	saved_sce_hdr = (void *)spu_args[8/8];

	if (process)
	{
		caller_process = process->pid;

		#ifdef	DEBUG
			//DPRINTF("caller_process = %08X\n", caller_process);
		#endif
	}

	return 0;
}

LV2_PATCHED_FUNCTION(int, modules_patching, (uint64_t *arg1, uint32_t *arg2))
{
	static unsigned int total = 0;
	static uint32_t *buf;
	static uint8_t keys[16];
	static uint64_t nonce = 0;

	SELF *self;
	uint64_t *ptr;
	uint32_t *ptr32;
	uint8_t *sce_hdr;

	ptr = (uint64_t *)(*(uint64_t *)MKA(TOC+decrypt_rtoc_entry_2));
	ptr = (uint64_t *)ptr[0x68/8];
	ptr = (uint64_t *)ptr[0x18/8];
	ptr32 = (uint32_t *)ptr;
	sce_hdr = (uint8_t *)saved_sce_hdr;
	self = (SELF *)sce_hdr;

	uint32_t *p = (uint32_t *)arg1[0x18/8];
	
	#ifdef	DEBUG
		//DPRINTF("Flags = %x	   %x\n", self->flags, (p[0x30/4] >> 16));
	#endif

	// +4.30 -> 0x13 (exact firmware since it happens is unknown)
	// 3.55 -> 0x29
#if defined(FIRMWARE_4_84) || defined(FIRMWARE_4_84DEX)
	if ((p[0x30/4] >> 16) == 0x13)
#endif
	{	
		#ifdef	DEBUG
			//DPRINTF("We are in decrypted module or in cobra encrypted\n");
		#endif		

		int last_chunk = 0;
		KeySet *keySet = NULL;

		if (((ptr[0x10/8] << 24) >> 56) == 0xFF)
		{
			ptr[0x10/8] |= 2;
			*arg2 = 0x2C;
			last_chunk = 1;
		}
		else
		{
			ptr[0x10/8] |= 3;
			*arg2 = 6;
		}

		uint8_t *enc_buf = (uint8_t *)ptr[8/8];
		uint32_t chunk_size = ptr32[4/4];
		SPRX_EXT_HEADER *extHdr = (SPRX_EXT_HEADER *)(sce_hdr+self->metadata_offset+0x20);
		uint64_t magic = extHdr->magic&SPRX_EXT_MAGIC_MASK;
		uint8_t keyIndex = extHdr->magic&0xFF;
		int dongle_decrypt = 0;

		if (magic == SPRX_EXT_MAGIC)
		{	
			if (keyIndex >= N_SPRX_KEYS_1)			
				DPRINTF("This key is not implemented yet: %lx:%x\n", magic, keyIndex);			
			else			
				keySet = &sprx_keys_set1[keyIndex];			
			
		}
		else if (magic == SPRX_EXT_MAGIC2)
		{
			if (keyIndex >= N_SPRX_KEYS_2)			
				DPRINTF("This key is not implemented yet: %lx:%x\n", magic, keyIndex);			
			else			
				keySet = &sprx_keys_set2[keyIndex];			
		}

		if (keySet)
		{
			if (total == 0)
			{
				uint8_t dif_keys[16];

				memset(dif_keys, 0, 16);

				if (dongle_decrypt)
				{
				}
				else				
					memcpy(keys, extHdr->keys_mod, 16);				

				for (int i = 0; i < 16; i++)				
					keys[i] ^= (keySet->keys[15-i] ^ dif_keys[15-i]);				

				nonce = keySet->nonce ^ extHdr->nonce_mod;
			}

			uint32_t num_blocks = chunk_size / 8;

			xtea_ctr(keys, nonce, enc_buf, num_blocks*8);
			nonce += num_blocks;

			if (last_chunk)
			{
				get_pseudo_random_number(keys, sizeof(keys));
				nonce = 0;
			}
		}

		memcpy(saved_buf, (void *)ptr[8/8], ptr32[4/4]);

		if (total == 0)		
			buf = (uint32_t *)saved_buf;		
		
		#ifdef	DEBUG
			if (last_chunk)
			{
				//DPRINTF("Total section size: %x\n", total+ptr32[4/4]);
			}
		#endif

		saved_buf += ptr32[4/4];
	}
	else
	{
		decrypt_func(arg1, arg2);
		buf = (uint32_t *)saved_buf;
	}

	total += ptr32[4/4];

	if (((ptr[0x10/8] << 24) >> 56) == 0xFF)
	{
		uint64_t hash = 0;

		for (int i = 0; i < 0x8; i++)  //0x20 bytes only		
			hash ^= buf[i+0xb0];  //unique location in all files+static hashes between firmware		
	
		total = (total & 0xfff000); //if size is less than 0x10000 then check for next 4 bits			
		
		hash = ((hash << 32) & 0xfffff00000000000) | (total);  //20 bits check, prevent diferent hash just because of minor changes

		total = 0;
		
		#ifdef	DEBUG
			//DPRINTF("hash = %lx\n", hash);
		#endif
		
		switch(hash)
		{
		    pad_data data;
			
			case VSH_CEX_HASH:
				vsh_check = hash;
			break;
		
			case VSH_DEX_HASH:
				vsh_check = hash;
			break;
			
			case EMULATOR_DRM_HASH:
				if (condition_psp_keys)
					buf[psp_drm_tag_overwrite/4] = LI(R5, psp_code);
			break;
			
			case EMULATOR_DRM_DATA_HASH:
				if (condition_psp_keys)
				{
					buf[psp_drm_key_overwrite/4] = psp_tag;
					memcpy(buf+((psp_drm_key_overwrite+8)/4), psp_keys, 16);
				}
			break;

			/*
			case BASIC_PLUGINS_HASH:
				if (condition_psp_change_emu)
				{
					memcpy(((char *)buf)+pspemu_path_offset, pspemu_path, sizeof(pspemu_path));
					memcpy(((char *)buf)+psptrans_path_offset, psptrans_path, sizeof(psptrans_path));
				}
			break;
			*/
			
			case PEMUCORELIB_HASH:
				if (condition_pemucorelib)
				{

					if (pad_get_data(&data) >= ((PAD_BTN_OFFSET_DIGITAL+1)*2)){

						if((data.button[PAD_BTN_OFFSET_DIGITAL] & (PAD_CTRL_CROSS|PAD_CTRL_R1)) == (PAD_CTRL_CROSS|PAD_CTRL_R1)){

							DPRINTF("Button Shortcut detected! Applying pemucorelib Extra Savedata Patch...\n");

							DPRINTF("Now patching %s %lx\n", hash_to_name(hash), hash);

							uint32_t data = LI(R31, 1);
							buf[psp_extra_savedata_patch/4] = data;			

							DPRINTF("Offset: 0x%08X | Data: 0x%08X\n", psp_extra_savedata_patch, data);
						}
					}
				}
			break;
			
			default:
				//Do nothing
			break;
		}

		for (int i = 0; i < N_PATCH_TABLE_ENTRIES; i++)
		{
			if (patch_table[i].hash == hash)
			{		
				#ifdef	DEBUG
				DPRINTF("Now patching  %s %lx\n", hash_to_name(hash), hash);
				#endif

				int j = 0;
				SprxPatch *patch = &patch_table[i].patch_table[j];

				while (patch->offset != 0)
				{
					if (*patch->condition)
					{
						buf[patch->offset/4] = patch->data;
						#ifdef	DEBUG
							DPRINTF("Offset: 0x%08X | Data: 0x%08X\n", (uint32_t)patch->offset, (uint32_t)patch->data);
							//DPRINTF("Offset: %lx\n", &buf[patch->offset/4]);
						#endif
					}

					j++;
					patch = &patch_table[i].patch_table[j];
				}

				break;
			}
		}
		
	}

	return 0;
}

#if defined (FIRMWARE_4_84)
LV2_HOOKED_FUNCTION_COND_POSTCALL_2(int, pre_modules_verification, (uint32_t *ret, uint32_t error))
{
/*
	// Patch original from psjailbreak. Needs some tweaks to fix some games
	#ifdef	DEBUG
	DPRINTF("err = %x\n", error);
	#endif
	if (error == 0x13)
	{
		//dump_stack_trace2(10);
		return DO_POSTCALL; // Fixes Mortal Kombat <- DON'T DO IT
	}
*/
	*ret = 0;
	return 0;
}
#elif defined (FIRMWARE_4_84DEX)
LV2_HOOKED_FUNCTION_COND_POSTCALL_2(int, pre_modules_verification, (uint32_t *ret, uint32_t error))
{
	return DO_POSTCALL;  //Fixes fucking DEX issue, no more crying bitches  //
}
#endif

void pre_map_process_memory(void *object, uint64_t process_addr, uint64_t size, uint64_t flags, void *unk, void *elf, uint64_t *out);


uint8_t cleared_stage0 = 0;

LV2_HOOKED_FUNCTION_POSTCALL_7(void, pre_map_process_memory, (void *object, uint64_t process_addr, uint64_t size, uint64_t flags, void *unk, void *elf, uint64_t *out))
{
	#ifdef	DEBUG
	//DPRINTF("Map %lx %lx %s\n", process_addr, size, get_current_process() ? get_process_name(get_current_process())+8 : "KERNEL");
	#endif
	
	// Not the call address, but the call to the caller (process load code for .self)
	if (get_call_address(1) == (void *)MKA(process_map_caller_call))
	{	
		if ((process_addr == 0x10000) && (size == dex_vsh_text_size) && (flags == 0x2008004) && (cleared_stage0 == 0))
		{
			#ifdef	DEBUG
				DPRINTF("Making Debug VSH text writable, Size: 0x%lx\n", size);   
			#endif

			// Change flags, RX -> RWX, make vsh text writable
			set_patched_func_param(4, 0x2004004);

			// We can clear stage0. 
			cleared_stage0 = 1; 
		}
		else if ((process_addr == 0x10000) && (size == cex_vsh_text_size) && (flags == 0x2008004) && (cleared_stage0 == 0))
		{
			#ifdef	DEBUG
				DPRINTF("Making Retail VSH text writable, Size: 0x%lx\n", size);   
			#endif

			// Change flags, RX -> RWX, make vsh text writable
			set_patched_func_param(4, 0x2004004);

			// We can clear stage0. 
			cleared_stage0 = 1; 
		}
		else if (flags == 0x2008004) 
			set_patched_func_param(4, 0x2004004); // Change flags, RX -> RWX
	}	
}

LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_8(int, load_process_hooked, (process_t process, int fd, char *path, int r6, uint64_t r7, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t sp_70))
{
	#ifdef	DEBUG
	DPRINTF("PROCESS %s (%08X) loaded\n", path, process->pid);
	#endif

	if (!vsh_process)
	{
		if (strcmp(path, "/dev_flash/vsh/module/vsh.self") == 0)		
			vsh_process = process;		
		else if (strcmp(path, "emer_init.self") == 0)
		{
			#ifdef	DEBUG
				DPRINTF("COBRA: Safe mode detected\n");
			#endif
			safe_mode = 1;
		}		
	}
	
	#ifndef  DEBUG
		if (vsh_process) 
			unhook_function_on_precall_success(load_process_symbol, load_process_hooked, 9); //Hook no more needed
	#endif
	
	return 0;
}

int bc_to_net(int param)
{	
	//returns:
	//0=disabled
	//1=enabled
	//-1=its not a bc/semi-bc console

	if(condition_ps2softemu)	
		return -1;
	
	if(param==1) //enable netemu
	{
		switch(vsh_check) 
		{
			case VSH_CEX_HASH:
				copy_to_process(vsh_process, &cex_vsh_patches[0].data, (void *)(uint64_t)(0x10000+cex_vsh_patches[0].offset), 4);
				copy_to_process(vsh_process, &cex_vsh_patches[1].data, (void *)(uint64_t)(0x10000+cex_vsh_patches[1].offset), 4);
				break;
			
			case VSH_DEX_HASH:
				copy_to_process(vsh_process, &dex_vsh_patches[0].data, (void *)(uint64_t)(0x10000+dex_vsh_patches[0].offset), 4);
				copy_to_process(vsh_process, &dex_vsh_patches[1].data, (void *)(uint64_t)(0x10000+dex_vsh_patches[1].offset), 4);
				break;
		}

		bc_to_net_status=1;
		return 1;
	}

	if(param==0) //restore
	{
		switch(vsh_check) 
		{
			case VSH_CEX_HASH:		
				copy_to_process(vsh_process, &cex_vsh_patches[2].data, (void *)(uint64_t)(0x10000+cex_vsh_patches[2].offset), 4);
				copy_to_process(vsh_process, &cex_vsh_patches[3].data, (void *)(uint64_t)(0x10000+cex_vsh_patches[3].offset), 4);
				break;
		
			case VSH_DEX_HASH:		
				copy_to_process(vsh_process, &dex_vsh_patches[2].data, (void *)(uint64_t)(0x10000+dex_vsh_patches[2].offset), 4);
				copy_to_process(vsh_process, &dex_vsh_patches[3].data, (void *)(uint64_t)(0x10000+dex_vsh_patches[3].offset), 4);
				break;
		}	
		
		bc_to_net_status=0;
		return 0;
	}

	if(param==2)	
		return bc_to_net_status;	
	
	return -2;
}

/*void do_spoof_patches(void)
{
	if (!vsh_process || get_current_process() != vsh_process)
		return;

	// Test
	config.spoof_version = 0x0469;
	config.spoof_revision = 69069;

	if (config.spoof_version && config.spoof_revision)
	{
		char rv[MAX_SPOOF_REVISION_CHARS+1];

		int n = snprintf(rv, sizeof(rv), "%05d", config.spoof_revision);
		if (n < sizeof(rv))
		{
			DPRINTF("Patching to revision %d\n", config.spoof_revision);
			copy_to_user(rv, (void *)(0x10000+revision_offset), n);
			copy_to_user(rv, (void *)(0x10000+revision_offset2), n);
		}
		else
		{
			//DPRINTF("n = %d\n", n);
		}

		uint32_t patch[4];

		patch[0] = MR(R4, R27);
		patch[1] = LI(R11, 8);
		patch[2] = LI(R3, SYSCALL8_OPCODE_VSH_SPOOF_VERSION);
		patch[3] = SC;

		copy_to_user(patch, (void *)(0x10000+spoof_version_patch), sizeof(patch));
	}
}*/

// Kernel version of prx_load_vsh_plugin
int prx_load_vsh_plugin(unsigned int slot, char *path, void *arg, uint32_t arg_size)
{
	void *kbuf, *vbuf;
	sys_prx_id_t prx;
	int ret;

	if (slot >= MAX_VSH_PLUGINS || (arg != NULL && arg_size > KB(64)))
		return EINVAL;

	if (vsh_plugins[slot] != 0)	
		return EKRESOURCE;	

	CellFsStat stat;
	if (cellFsStat(path, &stat) != 0 || stat.st_size < 0x230) return EINVAL; // prevent a semi-brick (black screen on start up) if the sprx is 0 bytes (due a bad ftp transfer).

	loading_vsh_plugin = 1;
	prx = prx_load_module(vsh_process, 0, 0, path);
	loading_vsh_plugin  = 0;

	if (prx < 0)
		return prx;

	if (arg && arg_size > 0)
	{
		page_allocate_auto(vsh_process, KB(64), 0x2F, &kbuf);
		page_export_to_proc(vsh_process, kbuf, 0x40000, &vbuf);
		memcpy(kbuf, arg, arg_size);
	}
	else	
		vbuf = NULL;	

	ret = prx_start_module_with_thread(prx, vsh_process, 0, (uint64_t)vbuf);

	if (vbuf)
	{
		page_unexport_from_proc(vsh_process, vbuf);
		page_free(vsh_process, kbuf, 0x2F);
	}

	if (ret == 0)	
		vsh_plugins[slot] = prx;	
	else
	{
		prx_stop_module_with_thread(prx, vsh_process, 0, 0);
		prx_unload_module(prx, vsh_process);
	}
	
	#ifndef  DEBUG
		DPRINTF("Vsh plugin load: %x\n", ret);
	#endif

	return ret;

}


// User version of prx_load_vsh_plugin
int sys_prx_load_vsh_plugin(unsigned int slot, char *path, void *arg, uint32_t arg_size)
{
	return prx_load_vsh_plugin(slot, get_secure_user_ptr(path), get_secure_user_ptr(arg), arg_size);
}

// Kernel version of prx_unload_vsh_plugin
int prx_unload_vsh_plugin(unsigned int slot)
{
	int ret;
	sys_prx_id_t prx;
	
	#ifndef  DEBUG
		DPRINTF("Trying to unload vsh plugin %x\n", slot);
	#endif

	if (slot >= MAX_VSH_PLUGINS)
		return EINVAL;

	prx = vsh_plugins[slot];
		
	#ifndef  DEBUG
		DPRINTF("Current plugin: %08X\n", prx);
	#endif

	if (prx == 0)
		return ENOENT;

	ret = prx_stop_module_with_thread(prx, vsh_process, 0, 0);
	if (ret == 0)	
		ret = prx_unload_module(prx, vsh_process);	
	#ifndef  DEBUG
		else	
			DPRINTF("Stop failed: %x!\n", ret);	
	#endif

	if (ret == 0)
	{
		vsh_plugins[slot] = 0;
		#ifndef  DEBUG
			DPRINTF("Vsh plugin unloaded succesfully!\n");
		#endif
	}
	#ifndef  DEBUG
		else	
			DPRINTF("Unload failed : %x!\n", ret);	
	#endif

	return ret;
}

// User version of prx_unload_vsh_plugin. Implementation is same.
int sys_prx_unload_vsh_plugin(unsigned int slot)
{
	return prx_unload_vsh_plugin(slot);

}

// static int was removed to support cfg implementation for homebrew blocker by KW & AV
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
	if(cellFsStat(path, &stat) == 0)
	{
		if(stat.st_size > 4)
		{
			if(cellFsOpen(path, CELL_FS_O_RDONLY, &file, 0, NULL, 0) == 0)
			{
				void *skprx=alloc(stat.st_size,0x27);
				if(skprx)
				{
					if(cellFsRead(file, skprx, stat.st_size, &read)==0)
					{	
						f_desc_t f;
						f.addr = skprx;
						f.toc = (void *)MKA(TOC);
						func = (void *)&f;
						func();
						uint64_t resident=(uint64_t)skprx;
						return resident;
					}
					else
					{
						dealloc(skprx, 0x27);
						return -2;
					}
				}
				else				
					return 0;				
			}
			else			
				return -1;			
		}
	}
	return -1;
}

void load_boot_plugins_kernel(void)
{
	int fd;
	int current_slot_kernel = 0;
	int num_loaded_kernel = 0;
	
	if (safe_mode)
	{
		cellFsUnlink(BOOT_PLUGINS_KERNEL_FILE);
		return;
	}
	
	if (!vsh_process)
		return;	  // lets wait till vsh so we dont brick the console perma!

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
					DPRINTF("Load boot plugin %s -> %x\n", path, current_slot_kernel);
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

// webMAN integration support
void load_boot_plugins(void)
{
	int fd;
	int current_slot = BOOT_PLUGINS_FIRST_SLOT;
	int num_loaded = 0;
	int webman_loaded = 0; // KW
	
	if (safe_mode)
	{
		cellFsUnlink(BOOT_PLUGINS_FILE);
		return;
	}
	
	if (!vsh_process)
		return;	

	// EVILNAT START
	// KW / Special thanks to KW for providing an awesome source
	// Improving initial KW's code	
	// Firstly will load plugin from '/dev_hdd0' instead '/dev_flash'
	// If it does not exist in '/dev_hdd0' will load it from '/dev_flash' 
	if (cellFsOpen(BOOT_PLUGINS_FILE, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == 0)
	{
		while (num_loaded < MAX_BOOT_PLUGINS)
		{
			char path[128];
			int eof;			
			
			if (read_text_line(fd, path, sizeof(path), &eof) > 0)
			{
				int ret = prx_load_vsh_plugin(current_slot, path, NULL, 0);
					
				if (ret >= 0)
				{
					if(strstr(path, "web"))
						webman_loaded = 1;

					DPRINTF("Load boot plugin %s -> %x\n", path, current_slot);
					current_slot++;
					num_loaded++;
				}			
			}
			
			if (eof)
				break;
		}

		cellFsClose(fd);
	}

	if(!webman_loaded)
	{
		CellFsStat stat;
		char path_prx[128];		

		// Default plugin's paths in some CFW
		char webman_paths[4][60] = 
		{
        	"/dev_flash/vsh/module/webftp_server.sprx",
        	"/dev_flash/ps3ita/webftp_server.sprx",
        	"/dev_flash/webman/webftp_server.sprx",
        	"/dev_flash/dragon/web.sprx",
    	};		

    	// Copy "/dev_flash/vsh/module/webftp_server.sprx" as default path
    	strcpy(path_prx, webman_paths[0]);

    	// Let's find the plugin
		for(int i = 0; i < 4; i++)
	    { 
	        if(cellFsStat(webman_paths[i], &stat) == 0)
	        {            
	            strcpy(path_prx, webman_paths[i]);
	            break;
	        }
	    }

		if (prx_load_vsh_plugin(current_slot, path_prx, NULL, 0) >= 0)	
			DPRINTF("Loading integrated webMAN plugin into slot %x\n", current_slot);		
	}
	// EVILNAT END
}

#ifdef DEBUG
LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_8(int, create_process_common_hooked, (process_t parent, uint32_t *pid, int fd, char *path, int r7, uint64_t r8,
									  uint64_t r9, void *argp, uint64_t args, void *argp_user, uint64_t sp_80,
									 void **sp_88, uint64_t *sp_90, process_t *process, uint64_t *sp_A0,
									  uint64_t *sp_A8))
{
	char *parent_name = get_process_name(parent);
	DPRINTF("PROCESS %s (%s) (%08X) created from parent process: %s\n", path, get_process_name(*process), *pid, ((int64_t)parent_name < 0) ? parent_name : "KERNEL");

	return 0;
}

LV2_HOOKED_FUNCTION_POSTCALL_8(void, create_process_common_hooked_pre, (process_t parent, uint32_t *pid, int fd, char *path, int r7, uint64_t r8,
									  uint64_t r9, void *argp, uint64_t args, void *argp_user, uint64_t sp_80,
									 void **sp_88, uint64_t *sp_90, process_t *process, uint64_t *sp_A0,
									  uint64_t *sp_A8))
{

	DPRINTF("Pre-process\n");
}

#endif

void modules_patch_init(void)
{
	hook_function_with_precall(lv1_call_99_wrapper_symbol, post_lv1_call_99_wrapper, 2);
	patch_call(patch_func2_offset, modules_patching);
	hook_function_with_cond_postcall(modules_verification_symbol, pre_modules_verification, 2);
	hook_function_with_postcall(map_process_memory_symbol, pre_map_process_memory, 7);
	hook_function_on_precall_success(load_process_symbol, load_process_hooked, 9);
#ifdef DEBUG
	hook_function_on_precall_success(create_process_common_symbol, create_process_common_hooked, 16);
	//hook_function_with_postcall(create_process_common_symbol, create_process_common_hooked_pre, 8);
#endif
}

void unhook_all_modules(void)
{
	suspend_intr();
	unhook_function_with_precall(lv1_call_99_wrapper_symbol, post_lv1_call_99_wrapper, 2);
	unhook_function_with_cond_postcall(modules_verification_symbol, pre_modules_verification, 2);
	unhook_function_with_postcall(map_process_memory_symbol, pre_map_process_memory, 7);		
#ifdef DEBUG
	unhook_function_on_precall_success(load_process_symbol, load_process_hooked, 9); //Auto unload if not def DEBUG 
	unhook_function_on_precall_success(create_process_common_symbol, create_process_common_hooked, 16);
	//unhook_function_with_postcall(create_process_common_symbol, create_process_common_hooked_pre, 8);
#endif
	resume_intr();
}

int ps3mapi_unload_vsh_plugin(char *name)
{
    if (vsh_process <= 0) return ESRCH;
	for (unsigned int slot = 0; slot < MAX_VSH_PLUGINS; slot++)
	{
		if (vsh_plugins[slot] == 0) 
			continue;

		char *filename = alloc(256, 0x35);

		if (!filename) 
			return ENOMEM;

		sys_prx_segment_info_t *segments = alloc(sizeof(sys_prx_segment_info_t), 0x35);
		
		if (!segments) {dealloc(filename, 0x35); 
			return ENOMEM;}

		sys_prx_module_info_t modinfo;
		memset(&modinfo, 0, sizeof(sys_prx_module_info_t));
		modinfo.filename_size = 256;
		modinfo.segments_num = 1;
		int ret = prx_get_module_info(vsh_process, vsh_plugins[slot], &modinfo, filename, segments);
		
		if (ret == SUCCEEDED)
		{
			if (strcmp(modinfo.name, get_secure_user_ptr(name)) == 0) 
				{
					dealloc(filename, 0x35);
					dealloc(segments, 0x35);
					return prx_unload_vsh_plugin(slot);
				}				
		}

		dealloc(filename, 0x35);
		dealloc(segments, 0x35);
	}
	return ESRCH;
}

int ps3mapi_get_vsh_plugin_info(unsigned int slot, char *name, char *filename)
{
	if (vsh_process <= 0) 
		return ESRCH;

	if (vsh_plugins[slot] == 0) 
		return ENOENT;

	char *tmp_filename = alloc(256, 0x35);
	if (!tmp_filename) 
		return ENOMEM;

	sys_prx_segment_info_t *segments = alloc(sizeof(sys_prx_segment_info_t), 0x35);
	if (!segments) {dealloc(tmp_filename, 0x35); 
		return ENOMEM;}

	char tmp_filename2[256];
	char tmp_name[30];
	sys_prx_module_info_t modinfo;
	memset(&modinfo, 0, sizeof(sys_prx_module_info_t));
	modinfo.filename_size = 256;
	modinfo.segments_num = 1;
	int ret = prx_get_module_info(vsh_process, vsh_plugins[slot], &modinfo, tmp_filename, segments);
	
	if (ret == SUCCEEDED)
	{
			sprintf(tmp_name, "%s", modinfo.name);
			ret = copy_to_user(&tmp_name, get_secure_user_ptr(name), strlen(tmp_name));	
			sprintf(tmp_filename2, "%s", tmp_filename);
			ret = copy_to_user(&tmp_filename2, get_secure_user_ptr(filename), strlen(tmp_filename2));
	}
	
	dealloc(tmp_filename, 0x35);
	dealloc(segments, 0x35);
	return ret;
}

