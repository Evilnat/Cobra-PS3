#ifndef __MODULESPATCH_H__
#define __MODULESPATCH_H__

#include <lv2/process.h>
#include <lv2/thread.h>

#if defined(FIRMWARE_4_75) || defined(FIRMWARE_4_75DEX) 

#define VSH_DEX_HASH					0xb6b6d000002e2000 
#define VSH_CEX_HASH					0xb6b6d000002e1000
#define BDP_DISC_CHECK_PLUGIN_HASH		0x9940000000003000
#define EXPLORE_PLUGIN_HASH				0xc50d0000000ec000
#define EXPLORE_CATEGORY_GAME_HASH		0xde52c00000056000
#define PS1_EMU_HASH					0xcc2850000009a000
#define PS1_NETEMU_HASH 				0xcc28b000000bd000
#define GAME_EXT_PLUGIN_HASH			0xcc2680000001e000
#define PSP_EMULATOR_HASH				0xcc29b00000023000
#define PEMUCORELIB_HASH				0x40425000000c0000
#define EMULATOR_API_HASH				0x8409f0000001b000
#define EMULATOR_DRM_HASH				0xbbb8800000005000
#define EMULATOR_DRM_DATA_HASH			0x2f3ab0000001b000
#define LIBSYSUTIL_SAVEDATA_PSP_HASH	0x0dfdc00000003000
#define LIBFS_EXTERNAL_HASH				0x05fd000000006000

/* vsh */
#define dex_vsh_text_size				0x6C0000
#define dex_ps2tonet_patch				0xCA060
#define dex_ps2tonet_size_patch 		0xCA054
#define dex_psp_drm_patch1				0x249FA4
#define dex_psp_drm_patch2				0x24AA44
#define dex_psp_drm_patch3				0x24A680
#define dex_psp_drm_patch4				0x24AE8C
#define dex_psp_drm_patchA				0x24A0C0
#define dex_psp_drm_patchB				0x24A960
#define dex_psp_drm_patchC				0x249AF8
#define dex_psp_drm_patchD				0x24A0A8
#define dex_psp_drm_patchE				0x24A0AC
#define dex_psp_drm_patchF				0x24AA78
#define dex_vmode_patch_offset			0x44D7E0

#define cex_vsh_text_size				0x6B0000
#define cex_ps2tonet_patch				0xC4B14
#define cex_ps2tonet_size_patch 		0xC4B08
#define cex_psp_drm_patch1				0x24277C
#define cex_psp_drm_patch2				0x24321C
#define cex_psp_drm_patch3				0x242E58
#define cex_psp_drm_patch4				0x243664
#define cex_psp_drm_patchA				0x242898
#define cex_psp_drm_patchB				0x243138
#define cex_psp_drm_patchC				0x2422D0
#define cex_psp_drm_patchD				0x242880
#define cex_psp_drm_patchE				0x242884
#define cex_psp_drm_patchF				0x243250
#define cex_vmode_patch_offset			0x445BEC		

/* explore_plugin */
#define app_home_offset      			0x251740
#define ps2_nonbw_offset      			0xDDD8C

/* explore_category_game */
#define ps2_nonbw_offset2				0x68264

/* bdp_disccheck_plugin */
#define dvd_video_region_check_offset	0x152C

/* ps1_emu */
#define ps1_emu_get_region_offset		0x3E74

/* ps1_netemu */
#define ps1_netemu_get_region_offset	0xA440C

/* game_ext_plugin */
#define sfo_check_offset				0x240EC
#define ps2_nonbw_offset3				0x172F4
#define ps_region_error_offset			0x6880
#define game_exit_popup_patch       	0xE41C 

/* psp_emulator */ 
#define psp_set_psp_mode_offset 		0x1C18 

/* emulator_api */
#define psp_read						0x102D8
#define psp_read_header 				0x1125C
#define psp_drm_patch5					0x11080
#define psp_drm_patch6					0x110B0
#define psp_drm_patch7					0x110C8
#define psp_drm_patch8					0x110CC
#define psp_drm_patch9					0x1120C
#define psp_drm_patch11 				0x11210
#define psp_drm_patch12 				0x11220
#define psp_product_id_patch1			0x11320
#define psp_product_id_patch3			0x115F8
#define umd_mutex_offset				(0x64480+0x38C)

/* pemucorelib */
#define psp_eboot_dec_patch				0x5E6BC
#define psp_prx_patch					0x577D8
#define psp_savedata_bind_patch1		0x7A4BC
#define psp_savedata_bind_patch2		0x7A514
#define psp_savedata_bind_patch3		0x7A030
#define psp_extra_savedata_patch		0x87540
#define psp_prometheus_patch			0x12EA28
#define prx_patch_call_lr				0x5892C

/* emulator_drm */
#define psp_drm_tag_overwrite			0x4C68
#define psp_drm_key_overwrite			(0x27600-0xBE80)

/* libsysutil_savedata_psp */
#define psp_savedata_patch1				0x46CC
#define psp_savedata_patch2				0x46A4
#define psp_savedata_patch3				0x4504
#define psp_savedata_patch4				0x453C
#define psp_savedata_patch5				0x4550
#define psp_savedata_patch6				0x46B8

/* libfs */
#define aio_copy_root_offset			0xD658

#endif /* FIRMWARE */

typedef struct
{
	uint32_t offset;
	uint32_t data;
	uint8_t *condition;
} SprxPatch;

extern uint8_t condition_ps2softemu;
extern uint8_t condition_apphome;
extern uint8_t condition_psp_iso;
extern uint8_t condition_psp_dec;
extern uint8_t condition_psp_keys;
extern uint8_t condition_psp_change_emu;
extern uint8_t condition_psp_prometheus;
extern uint64_t vsh_check;

extern process_t vsh_process;
extern uint8_t safe_mode;

/* Functions for kernel */
void modules_patch_init(void);
void load_boot_plugins(void);
int prx_load_vsh_plugin(unsigned int slot, char *path, void *arg, uint32_t arg_size);
int prx_unload_vsh_plugin(unsigned int slot);

void load_boot_plugins_kernel(void);

int bc_to_net();

/* Syscalls */
int sys_prx_load_vsh_plugin(unsigned int slot, char *path, void *arg, uint32_t arg_size);
int sys_prx_unload_vsh_plugin(unsigned int slot);
int sys_thread_create_ex(sys_ppu_thread_t *thread, void *entry, uint64_t arg, int prio, uint64_t stacksize, uint64_t flags, const char *threadname);

// PS3Mapi v1.2.1
int ps3mapi_unload_vsh_plugin(char* name); 
void unhook_all_modules(void);
int ps3mapi_get_vsh_plugin_info(unsigned int slot, char *name, char *filename);

#endif /* __MODULESPATCH_H__ */
