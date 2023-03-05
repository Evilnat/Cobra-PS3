#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lv2/lv2.h>
#include <lv2/libc.h>
#include <lv2/memory.h>
#include <lv2/patch.h>
#include <lv2/syscall.h>
#include <lv2/usb.h>
#include <lv2/storage.h>
#include <lv2/thread.h>
#include <lv2/synchronization.h>
#include <lv2/modules.h>
#include <lv2/ctrl.h>
#include <lv2/io.h>
#include <lv2/time.h>
#include <lv2/security.h>
#include <lv2/error.h>
#include <lv2/symbols.h>
#include <lv1/stor.h>
#include <lv1/patch.h>
#include "common.h"
#include "syscall8.h"
#include "modulespatch.h"
#include "mappath.h"
#include "storage_ext.h"
#include "region.h"
#include "psp.h"
#include "config.h"
#include "ps3mapi_core.h"
#include "fan_control.h"
#include "make_rif.h"
#include "homebrew_blocker.h"

// Format of version:
// byte 0, 7 MS bits -> reserved
// byte 0, 1 LS bit -> 1 = CFW version, 0 = OFW/exploit version
// byte 1 and 2 -> ps3 fw version in BCD e.g 3.55 = 03 55. For legacy reasons, 00 00 means 3.41
// byte 3 is cobra firmware version,
// 1 = version 1.0-1.2,
// 2 = 2.0,
// 3 = 3.0
// 4 = 3.1
// 5 = 3.2
// 6 = 3.3
// 7 = 4.0
// 8 = 4.1
// 9 = 4.2
// A = 4.3
// B = 4.4
// C = 5.0
// D = 5.1
// E = 6.0
// F = 7.0

#define CB_LOCATION 			"/dev_blind/sys/stage2.bin"

#define COBRA_VERSION			0x0F
#define COBRA_VERSION_BCD		0x0840

#if defined(FIRMWARE_DEX)
	#define FIRMWARE_VERSION	0x0490
#endif

#if defined(CFW)
	#define IS_CFW			1
#else
	#define IS_CFW			0
#endif

#define MAKE_VERSION(cobra, fw, type) ((cobra & 0xFF) | ((fw & 0xffff) << 8) | ((type & 0x1) << 24))
#define N_KERNEL_PATCHES	(sizeof(kernel_patches) / sizeof(Patch))

typedef struct
{
	uint32_t address;
	uint32_t data;
} Patch;

static Patch kernel_patches[] =
{
	{ patch_data1_offset, 0x01000000 },
	{ patch_func8_offset1, LI(R3, 0) }, // force lv2open return 0

	// disable calls in lv2open to lv1_send_event_locally which makes the system crash
	{ patch_func8_offset2, NOP },
	{ patch_func9_offset, NOP }, // 4.30 - watch: additional call after

	// psjailbreak, PL3, etc destroy this function to copy their code there.
	// We don't need that, but let's dummy the function just in case that patch is really necessary
	{ mem_base2, LI(R3, 1) },
	{ mem_base2 + 4, BLR },

	// sys_sm_shutdown, for ps2 let's pass to copy_from_user a fourth parameter
	{ shutdown_patch_offset, MR(R6, R31) },
	{ module_sdk_version_patch_offset, NOP },

	// User thread prio hack (needed for netiso)
	{ user_thread_prio_patch, NOP },
	{ user_thread_prio_patch2, NOP },

	// ODE Protection removal (needed for CFW 4.60+) 
	{ lic_patch, LI(R3, 1) },
	{ ode_patch, LI(R3, 0) },
	{ ode_patch + 4, STD(R3, 0, R9) },

	// Fan patches
	{ sm_get_temperature_patch, LI(R3, 0) },
	{ sm_get_fan_policy_patch, LI(R3, 1) },
	{ sm_set_fan_policy_patch, LI(R3, 1) },
};

static int disable_cobra_stage()
{
	cellFsUtilMount_h("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0, 0, 0, 0, 0);
	cellFsRename(CB_LOCATION, CB_LOCATION ".bak");
	uint64_t size = 0x5343450000000000;

	int dst;
	cellFsOpen("/dev_hdd0/tmp/loadoptical", CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &dst, 0666, NULL, 0);
	cellFsWrite(dst, &size, 4, &size);
	cellFsClose(dst);
	return SUCCEEDED;
}

static int inst_and_run_kernel(uint8_t *payload, int size)
{
	if((!size) || (size > 0x10000))
		return -1;
	
	if(!payload)
		return -2;
	
	memcpy((void *)0x80000000007F0000, get_secure_user_ptr(payload), size);

	f_desc_t f;
	int (* func)(void);
	f.addr = (void *)0x80000000007F0000;
	f.toc = (void *)MKA(TOC);
	func = (void *)&f;
	func();

	return SUCCEEDED;
}

static int inst_and_run_kernel_dynamic(uint8_t *payload, int size, uint64_t *residence)
{
	if(!size)
		return -1;
	
	if(!payload)
		return -2;
	
	void *skprx = malloc(size);

	if(skprx)
	{
		memcpy(skprx, get_secure_user_ptr(payload), size);

		f_desc_t f;
		int (* func)(void);
		f.addr = skprx;
		f.toc = (void *)MKA(TOC);
		func = (void *)&f;
		func();
		uint64_t resident = (uint64_t)skprx;
		copy_to_user(&resident, get_secure_user_ptr(residence), 8);

		return 1;
	}
	
	return SUCCEEDED;
}

static int unload_plugin_kernel(uint64_t residence)
{
	free((void *)residence);
	return SUCCEEDED;
}

f_desc_t extended_syscall8;

int64_t syscall8(uint64_t function, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5, uint64_t param6, uint64_t param7);

LV2_SYSCALL2(uint64_t, sys_cfw_peek, (uint64_t *addr))
{
	uint64_t ret = *addr;

	// Fix compatibilty issue with prx loader. It searches for a string... that is also in this payload, and then lv2_peek((vsh_str + 0x70)) crashes the system.
	if (ret == 0x5F6D61696E5F7673)
	{
		extern uint64_t _start;
		extern uint64_t __self_end;

		if ((uint64_t)addr >= (uint64_t)&_start && (uint64_t)addr < (uint64_t)&__self_end)
		{
			//DPRINTF("peek to addr %p blocked for compatibility.\n", addr);
			return SUCCEEDED;
		}
	}

	return ret;
}

static void *current_813;

void _sys_cfw_poke(uint64_t *addr, uint64_t value);

LV2_HOOKED_FUNCTION(void, sys_cfw_new_poke, (uint64_t *addr, uint64_t value))
{
	//DPRINTF("New poke called\n");

	_sys_cfw_poke(addr, value);
	asm volatile("icbi 0,%0; isync" :: "r"(addr));
}

LV2_HOOKED_FUNCTION(void *, sys_cfw_memcpy, (void *dst, void *src, uint64_t len))
{
	//DPRINTF("sys_cfw_memcpy: %p %p 0x%lx\n", dst, src, len);

	if (len == 8)
	{
		_sys_cfw_poke(dst, *(uint64_t *)src);
		return dst;
	}

	return memcpy(dst, src, len);
}

LV2_SYSCALL2(void, sys_cfw_poke, (uint64_t *ptr, uint64_t value))
{
	uint64_t addr = (uint64_t)ptr;

	DPRINTF("LV2 poke %p %016lx\n", (void*)addr, value);

	if (addr >= MKA(syscall_table_symbol))
	{
		uint64_t syscall_num = (addr - MKA(syscall_table_symbol)) / 8;

		if ((syscall_num >= 6 && syscall_num <= 10) || syscall_num == 35)
		{
			uint64_t sc_null = *(uint64_t *)MKA(syscall_table_symbol);
			uint64_t syscall_not_impl = *(uint64_t *)sc_null;

			if (syscall_num == 8 && (value & 0xFFFFFFFF00000000ULL) == MKA(0))
			{
				// Probably iris manager or similar
				// Lets extend our syscall 8 so that it can call this other syscall 8
				// First check if it is trying to restore our syscall8
				if (*(uint64_t *)syscall8 == value)
				{
					//DPRINTF("Removing syscall 8 extension\n");

					extended_syscall8.addr = 0;
					return;
				}

				extended_syscall8.addr = (void *) *(uint64_t *)value;
				extended_syscall8.toc = (void *) *(uint64_t *)(value + 8);

				//DPRINTF("Adding syscall 8 extension %p %p\n", extended_syscall8.addr, extended_syscall8.toc);
				return;
			}
			else if (((value == sc_null) ||(value == syscall_not_impl)) && (syscall_num != 8)) //Allow removing protected syscall 6 7 9 10 35 NOT 8			
			{
				//DPRINTF("HB remove syscall %ld\n", syscall_num);			
			}
			else //Prevent syscall 6 7 9 10 and 35 from being re-written
			{
				//DPRINTF("HB has been blocked from rewritting syscall %ld\n", syscall_num);
				return;
			}
		}
	}
	else if (addr == MKA(open_path_symbol))
	{
		//DPRINTF("open_path poke: %016lx\n", value);

		if (value == 0xf821ff617c0802a6ULL)
		{
			//DPRINTF("Restoring map_path patches\n");

			*ptr = value;
			clear_icache(ptr, 8);
			map_path_patches(0);

			return;
		}
	}
	else
	{
		uint64_t sc813 = get_syscall_address(SYS_FS_RMDIR);

		if (addr == sc813)
		{
			if (value == 0xF88300007C001FACULL)
			{
				f_desc_t f;

				// Assume app is trying to write the so called "new poke"
				//DPRINTF("Making sys_cfw_new_poke\n");

				if (current_813)				
					unhook_function(sc813, current_813);				

				hook_function(sc813, sys_cfw_new_poke, &f);
				current_813 = sys_cfw_new_poke;
				return;
			}
			else if (value == 0x4800000428250000ULL)
			{
				f_desc_t f;

				// Assume app is trying to write a memcpy
				//DPRINTF("Making sys_cfw_memcpy\n");

				if (current_813)				
					unhook_function(sc813, current_813);				

				hook_function(sc813, sys_cfw_memcpy, &f);
				current_813 = sys_cfw_memcpy;
				return;
			}
			else if (value == 0xF821FF017C0802A6ULL)
			{
				// Assume app is trying to restore sc 813
				if (current_813)
				{
					//DPRINTF("Restoring syscall 813\n");

					unhook_function(sc813, current_813);
					current_813 = NULL;
					return;
				}
			}
			else
			{
				//DPRINTF("Warning: syscall 813 being overwritten with unknown value (%016lx). *blocking it*\n", value);
				return;
			}
		}
		else if (addr > sc813 && addr < (sc813 + 0x20))		
			return;		
	}

	*ptr = value;
}

LV2_SYSCALL2(void, sys_cfw_lv1_poke, (uint64_t lv1_addr, uint64_t lv1_value))
{
	DPRINTF("LV1 poke %p %016lx\n", (void*)lv1_addr, lv1_value);	
	lv1_poked(lv1_addr, lv1_value);
}

LV2_SYSCALL2(void, sys_cfw_lv1_call, (uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7, uint64_t num))
{
	/* DO NOT modify */
	asm("mflr 0\n");
	asm("std 0, 16(1)\n");
	asm("mr 11, 10\n");
	asm("sc 1\n");
	asm("ld 0, 16(1)\n");
	asm("mtlr 0\n");
	asm("blr\n");
}

LV2_SYSCALL2(void, sys_cfw_lv2_func, (uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7, uint64_t num))
{
	/* DO NOT modify */
	asm("mflr 0\n");
	asm("std 0, 16(1)\n");
	asm("mr 0, 10\n");
	asm("mtctr 0\n");
	asm("bctrl\n");
	asm("ld 0, 16(1)\n");
	asm("mtlr 0\n");
	asm("blr\n");
}

static INLINE int sys_get_version(uint32_t *version)
{
	uint32_t pv = MAKE_VERSION(COBRA_VERSION, FIRMWARE_VERSION, IS_CFW);
	return copy_to_user(&pv, get_secure_user_ptr(version), sizeof(uint32_t));
}

static INLINE int sys_get_version2(uint16_t *version)
{
	uint16_t cb = COBRA_VERSION_BCD;
	return copy_to_user(&cb, get_secure_user_ptr(version), sizeof(uint16_t));
}

LV2_SYSCALL2(uint64_t, sys_cfw_lv1_peek, (uint64_t lv1_addr))
{
	DPRINTF("lv1_peek %p\n", (void*)lv1_addr);
	
    uint64_t ret;
    ret = lv1_peekd(lv1_addr);
    return ret;
}

void create_syscalls(void)
{
	create_syscall2(8, syscall8);
	create_syscall2(6, sys_cfw_peek);
	create_syscall2(7, sys_cfw_poke);
	create_syscall2(9, sys_cfw_lv1_poke);
	create_syscall2(10, sys_cfw_lv1_call);
	create_syscall2(11, sys_cfw_lv1_peek);
	create_syscall2(15, sys_cfw_lv2_func);
	create_syscall2(SYS_MAP_PATH, sys_map_path);
}

static inline void ps3mapi_unhook_all(void)
{
	unhook_all_modules();
	unhook_all_region();
	unhook_all_map_path();
    unhook_all_storage_ext();
}

static uint64_t ps3mapi_key = 0;
static uint8_t ps3mapi_access_tries = 0;
static uint8_t ps3mapi_access_granted = 1;

static int ps3mapi_partial_disable_syscall8 = 0;
static uint8_t disable_cobra = 0;

LV2_SYSCALL2(int64_t, syscall8, (uint64_t function, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5, uint64_t param6, uint64_t param7))
{
	extend_kstack(0);

	//DPRINTF("Syscall 8 -> %lx\n", function);
	
	if(function >= 0x8000000000000000ULL)	
	{
		DPRINTF("LV1 peek %lx %llux\n", function, (long long unsigned int)(lv1_peekd(function)));
	}
	
	// -- AV: temporary disable cobra syscall (allow dumpers peek 0x1000 to 0x9800)
	static uint8_t tmp_lv1peek = 0;

	if(ps3mapi_partial_disable_syscall8 == 0 && extended_syscall8.addr == 0 && ps3mapi_access_granted)
	{
		if((function >= 0x9800) || (function & 3)) 
			tmp_lv1peek = 0; 
		else if(function <= 0x1000) 
		{
			tmp_lv1peek = 1; 

			if(function <= SYSCALL8_OPCODE_ENABLE_COBRA) 
			{
				if(param1 >= SYSCALL8_DISABLE_COBRA_CAPABILITY) 
					return (param1 == SYSCALL8_DISABLE_COBRA_CAPABILITY) ? SYSCALL8_DISABLE_COBRA_OK : disable_cobra; 
				else 
					disable_cobra = (function == SYSCALL8_OPCODE_DISABLE_COBRA && param1 == 1);
			}
		}

		if(tmp_lv1peek) 		
			return lv1_peekd(function);		
	}
	else 
		tmp_lv1peek = 0;

	if ((function == SYSCALL8_OPCODE_PS3MAPI) && ((int)param1 == PS3MAPI_OPCODE_REQUEST_ACCESS) && (param2 == ps3mapi_key) && (ps3mapi_access_tries < 3)) 
	{
		ps3mapi_access_tries = 0; 
		ps3mapi_access_granted = 1;
	}

	if((!ps3mapi_access_granted) && (ps3mapi_key != 0))
	{
		ps3mapi_access_tries += 1;

		if(ps3mapi_access_tries > 3) 
			ps3mapi_access_tries = 99;

		return ENOSYS;
	}

	if (3 <= ps3mapi_partial_disable_syscall8)
	{
		if (function == SYSCALL8_OPCODE_PS3MAPI && ps3mapi_access_granted)
		{
			if ((int)param1 == PS3MAPI_OPCODE_PDISABLE_SYSCALL8)
			{
				ps3mapi_partial_disable_syscall8 = (int)param2;
				return SUCCEEDED;
			}
			else if ((int)param1 == PS3MAPI_OPCODE_PCHECK_SYSCALL8)			
				return ps3mapi_partial_disable_syscall8;			
			else 
				return ENOSYS;
		}
		else 
			return ENOSYS;
	}

	if ((function != SYSCALL8_OPCODE_PS3MAPI) && (2 <= ps3mapi_partial_disable_syscall8)) 
		return ENOSYS;

	// -- AV: disable cobra without reboot (use lv1 peek)
	if(disable_cobra) 
		return lv1_peekd(function);

	switch (function)
	{
		case SYSCALL8_OPCODE_PS3MAPI:
			switch ((int)param1)
			{
				//DPRINTF("syscall8: PS3M_API function 0x%x\n", (int)param1);

				//----------
				//CORE
				//----------
				case PS3MAPI_OPCODE_GET_CORE_VERSION:
					return PS3MAPI_CORE_VERSION;
				break;

				case PS3MAPI_OPCODE_GET_CORE_MINVERSION:
					return PS3MAPI_CORE_MINVERSION;
				break;

				case PS3MAPI_OPCODE_GET_FW_VERSION:
					return PS3MAPI_FW_VERSION;
				break;

				case PS3MAPI_OPCODE_GET_FW_TYPE:
					return ps3mapi_get_fw_type((char *)param2);
				break;

				//----------------
				//PEEK & POKE (av)
				//----------------
				case PS3MAPI_OPCODE_SUPPORT_SC8_PEEK_POKE:
					return PS3MAPI_OPCODE_SUPPORT_SC8_PEEK_POKE_OK;
				break;

				case PS3MAPI_OPCODE_LV1_PEEK:
					return lv1_peekd(param2);
				break;

				case PS3MAPI_OPCODE_LV1_POKE:
					lv1_poked(param2, param3);
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_LV2_PEEK:
					return lv1_peekd(param2 + 0x8000000ULL);
				break;

				case PS3MAPI_OPCODE_LV2_POKE:
					lv1_poked(param2 + 0x8000000ULL, param3);
					return SUCCEEDED;
				break;

				//----------
				//SECURITY
				//----------
				case PS3MAPI_OPCODE_SET_ACCESS_KEY:
					ps3mapi_key = param2;
					ps3mapi_access_granted = 0;
					ps3mapi_access_tries = 0;
					return SUCCEEDED;
				break;

				//----------
				//PROCESS
				//----------
				case PS3MAPI_OPCODE_GET_ALL_PROC_PID:
					return ps3mapi_get_all_processes_pid((process_id_t *)param2);
				break;

				case PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID:
					return ps3mapi_get_process_name_by_pid((process_id_t)param2, (char *)param3);
				break;

				case PS3MAPI_OPCODE_GET_PROC_BY_PID:
					return ps3mapi_get_process_by_pid((process_id_t)param2, (process_t)param3);
				break;

				case PS3MAPI_OPCODE_GET_CURRENT_PROC:
					return ps3mapi_get_current_process((process_t)param2);
				break;

				case PS3MAPI_OPCODE_GET_CURRENT_PROC_CRIT:
					return ps3mapi_get_current_process_critical((process_t)param2);
				break;

				//----------
				//MEMORY
				//----------
				case PS3MAPI_OPCODE_GET_PROC_MEM:
					return ps3mapi_get_process_mem((process_id_t)param2, param3, (char *)param4, (int)param5);
				break;

				case PS3MAPI_OPCODE_SET_PROC_MEM:
					return ps3mapi_set_process_mem((process_id_t)param2, param3, (char *)param4, (int)param5);
				break;

				case PS3MAPI_OPCODE_PROC_PAGE_ALLOCATE:
					return ps3mapi_process_page_allocate((process_id_t)param2, param3, param4, param5, param6, (uint64_t *)param7);
				break;

				case PS3MAPI_OPCODE_PROC_PAGE_FREE:
					return ps3mapi_process_page_free((process_id_t)param2, param3, (u64 *)param4);
				break;
					
				//----------
				//MODULE
				//----------
				case PS3MAPI_OPCODE_GET_ALL_PROC_MODULE_PID:
					return ps3mapi_get_all_process_modules_prx_id((process_id_t)param2, (sys_prx_id_t *)param3);
				break;

				case PS3MAPI_OPCODE_GET_PROC_MODULE_NAME:
					return ps3mapi_get_process_module_name_by_prx_id((process_id_t)param2, (sys_prx_id_t)param3, (char *)param4);
				break;

				case PS3MAPI_OPCODE_GET_PROC_MODULE_FILENAME:
					return ps3mapi_get_process_module_filename_by_prx_id((process_id_t)param2, (sys_prx_id_t)param3, (char *)param4);
				break;

				case PS3MAPI_OPCODE_LOAD_PROC_MODULE:
					return ps3mapi_load_process_modules((process_id_t)param2, (char *)param3, (void *)param4, (uint32_t)param5);
				break;

				case PS3MAPI_OPCODE_UNLOAD_PROC_MODULE:
					return ps3mapi_unload_process_modules((process_id_t)param2, (sys_prx_id_t)param3);
				break;

				case PS3MAPI_OPCODE_GET_PROC_MODULE_INFO:
					return ps3mapi_get_process_module_info((process_t)param2, (sys_prx_id_t)param3, (char *)param4, (char *)param5);
				break;

				case PS3MAPI_OPCODE_GET_PROC_MODULE_SEGMENTS:
					return ps3mapi_get_process_module_segments((process_id_t)param2, (sys_prx_id_t)param3, (sys_prx_module_info_t *)param4); // TheRouletteBoi
				break;

				//----------
				//VSH PLUGINS
				//----------
				case PS3MAPI_OPCODE_UNLOAD_VSH_PLUGIN:
					return ps3mapi_unload_vsh_plugin((char *)param2);
				break;

				case PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO:
					return ps3mapi_get_vsh_plugin_info((unsigned int)param2, (char *)param3, (char *)param4);
				break;

				case PS3MAPI_OPCODE_GET_VSH_PLUGIN_BY_NAME:
					return ps3mapi_get_vsh_plugin_slot_by_name((char *)param2, (uint8_t)param3);
				break;

				//----------
				//SYSCALL
				//----------
				case PS3MAPI_OPCODE_DISABLE_SYSCALL:
					return ps3mapi_disable_syscall((int)param2);
				break;

				case PS3MAPI_OPCODE_CHECK_SYSCALL:
					return ps3mapi_check_syscall((int)param2);
				break;

				case PS3MAPI_OPCODE_PDISABLE_SYSCALL8:
					ps3mapi_partial_disable_syscall8 = (int)param2;
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_PCHECK_SYSCALL8:
					return ps3mapi_partial_disable_syscall8;
				break;

				case PS3MAPI_OPCODE_CREATE_CFW_SYSCALLS:
					create_syscalls();
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_ALLOW_RESTORE_SYSCALLS:
					allow_restore_sc = (uint8_t)param2; // 1 = allow, 0 = do not allow
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_GET_RESTORE_SYSCALLS:
					return allow_restore_sc;
				break;

				//----------
				//REMOVE HOOK
				//----------
				case PS3MAPI_OPCODE_REMOVE_HOOK:
					ps3mapi_unhook_all(); //REMOVE ALL "MAMBA/COBRA" HOOK
					return SUCCEEDED;
				break;

				//-----------
				//PSID/IDPS
				//-----------
				case PS3MAPI_OPCODE_GET_IDPS:
					return ps3mapi_get_idps((uint64_t *)param2);
				break;

				case PS3MAPI_OPCODE_SET_IDPS:
					return ps3mapi_set_idps(param2, param3);
				break;

				case PS3MAPI_OPCODE_GET_PSID:
					return ps3mapi_get_psid((uint64_t *)param2);
				break;

				case PS3MAPI_OPCODE_SET_PSID:
					return ps3mapi_set_psid(param2, param3);
				break;

				//----------
				//MISC
				//----------
				case PS3MAPI_OPCODE_PHOTO_GUI:
					photo_gui = (uint8_t)param2;
					return photo_gui;
				break;

				case PS3MAPI_OPCODE_SET_FAN_SPEED:
					if(param2 == 2 || param2 == 3 || param2 == 4 || param2 == 5)
						do_fan_control((uint8_t)param2);
					else
						fan_control_running = 0;

					if(param2 == 2 || param2 == 3 || param2 == 4 || param2 == 5)
						sm_set_fan_policy(0, 2, 0x68); // 40%, to avoid a lower initial speed
					else
						sm_set_fan_policy(0, (uint8_t)(param2 >= 0x33 ? 2 : 1), (uint8_t)(param2 >= 0x33 ? param2 : 0));
				break;

				// Now the value is setted externally with xai_plugin
				case PS3MAPI_OPCODE_SET_PS2_FAN_SPEED:
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_GET_FAN_SPEED:
					return sm_get_fan_speed();
				break;
				
				case PS3MAPI_OPCODE_RING_BUZZER:
					sm_ring_buzzer((uint16_t)param2);
				break;

				case PS3MAPI_OPCODE_SKIP_EXISTING_RIF:
					skip_existing_rif = (uint8_t)param2;
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_GET_SKIP_EXISTING_RIF:
					return skip_existing_rif;
				break;

				case PS3MAPI_OPCODE_CREATE_RIF:
					make_rif((char *)param2);
					return SUCCEEDED;
				break;

				case PS3MAPI_OPCODE_SWAP_PS2_ICON_COLOR:
					color_disc = (uint8_t)param2; // 1 = PS2 CD: blue icon - PS2 DVD: yellow icon | 0 = PS2 CD/DVD blue icon
					return SUCCEEDED;
				break;

				//----------
				//DEFAULT
				//----------
				default:
					DPRINTF("syscall8: Unsupported PS3M_API opcode: 0x%lx\n", function);
					return ENOSYS;
				break;
			}
		break;

		case SYSCALL8_OPCODE_STEALTH_TEST:  // KW PSNPatch stealth extension compatibility
			return SYSCALL8_STEALTH_OK;
		break;

		case SYSCALL8_OPCODE_STEALTH_ACTIVATE: // KW PSNPatch stealth extension compatibility
		{
			uint64_t syscall_not_impl = *(uint64_t *)MKA(syscall_table_symbol);
			//*(uint64_t *)MKA(syscall_table_symbol + 8 * 8) = syscall_not_impl;
			ps3mapi_partial_disable_syscall8 = 2; //NzV Edit: Keep PS3M_API Features only.
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 9) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 10) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 11) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 15) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 35) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 36) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 38) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 6) = syscall_not_impl;
			*(uint64_t *)MKA(syscall_table_symbol + 8 * 7) = syscall_not_impl;
			return SYSCALL8_STEALTH_OK;
		}
		break;

		//--------------------------
		// COBRA VERSION
		//--------------------------
		case SYSCALL8_OPCODE_GET_VERSION:
			return sys_get_version((uint32_t *)param1);
		break;

		case SYSCALL8_OPCODE_GET_VERSION2:
			return sys_get_version2((uint16_t *)param1);
		break;

		//--------------------------
		// STORAGE
		//--------------------------
		case SYSCALL8_OPCODE_GET_DISC_TYPE:
			return sys_storage_ext_get_disc_type((unsigned int *)param1, (unsigned int *)param2, (unsigned int *)param3);
		break;

		case SYSCALL8_OPCODE_READ_PS3_DISC:
			return sys_storage_ext_read_ps3_disc((void *)param1, param2, (uint32_t)param3);
		break;

		case SYSCALL8_OPCODE_FAKE_STORAGE_EVENT:
			return sys_storage_ext_fake_storage_event(param1, param2, param3);
		break;

		case SYSCALL8_OPCODE_GET_EMU_STATE:
			return sys_storage_ext_get_emu_state((sys_emu_state_t *)param1);
		break;

		case SYSCALL8_OPCODE_USE_PS2NETEMU:
			return bc_to_net((int)param1);
		break;

		//--------------------------
		// MOUNT
		//--------------------------
		case SYSCALL8_OPCODE_MOUNT_PS3_DISCFILE:
			return sys_storage_ext_mount_ps3_discfile(param1, (char **)param2);
		break;

		case SYSCALL8_OPCODE_MOUNT_DVD_DISCFILE:
			return sys_storage_ext_mount_dvd_discfile(param1, (char **)param2);
		break;

		case SYSCALL8_OPCODE_MOUNT_BD_DISCFILE:
			return sys_storage_ext_mount_bd_discfile(param1, (char **)param2);
		break;

		case SYSCALL8_OPCODE_MOUNT_PSX_DISCFILE:
			return sys_storage_ext_mount_psx_discfile((char *)param1, param2, (ScsiTrackDescriptor *)param3);
		break;

		case SYSCALL8_OPCODE_MOUNT_PS2_DISCFILE:
			return sys_storage_ext_mount_ps2_discfile(param1, (char **)param2, param3, (ScsiTrackDescriptor *)param4);
		break;

		case SYSCALL8_OPCODE_MOUNT_DISCFILE_PROXY:
			return sys_storage_ext_mount_discfile_proxy(param1, param2, param3, param4, param5, param6, (ScsiTrackDescriptor *)param7);
		break;

		case SYSCALL8_OPCODE_UMOUNT_DISCFILE:
			return sys_storage_ext_umount_discfile();
		break;

		case SYSCALL8_OPCODE_MOUNT_ENCRYPTED_IMAGE:
			return SUCCEEDED; // deprecated OPCODE for sys_storage_ext_mount_encrypted_image((char *)param1, (char *)param2, (char *)param3, param4);
		break;

		//--------------------------
		// KERNEL PAYLOAD
		//--------------------------
		case SYSCALL8_OPCODE_RUN_PAYLOAD:
			return inst_and_run_kernel((uint8_t *)param1, param2);
		break;

		case SYSCALL8_OPCODE_RUN_PAYLOAD_DYNAMIC:
			return inst_and_run_kernel_dynamic((uint8_t *)param1, param2, (uint64_t *)param3);
		break;

		case SYSCALL8_OPCODE_UNLOAD_PAYLOAD_DYNAMIC:
			return unload_plugin_kernel(param1);
		break;

		//--------------------------
		// PROCESS
		//--------------------------
		case SYSCALL8_OPCODE_PROC_CREATE_THREAD:
			return ps3mapi_create_process_thread((process_id_t)param1, (thread_t *)param2, (void *)param3, (uint64_t)param4, (int)param5, (size_t)param6, (char *)param7);
		break;

		//--------------------------
		// MISC
		//--------------------------
		case SYSCALL8_OPCODE_GET_ACCESS:
		case SYSCALL8_OPCODE_REMOVE_ACCESS:
		case SYSCALL8_OPCODE_COBRA_USB_COMMAND:
			return SUCCEEDED; // deprecated OPCODES
		break;
		
		case SYSCALL8_OPCODE_DISABLE_COBRA_STAGE:
			return disable_cobra_stage();
		break;		

		case SYSCALL8_OPCODE_DRM_GET_DATA:
		case SYSCALL8_OPCODE_VSH_SPOOF_VERSION:
			return ENOSYS; // deprecated opcodes
		break;

		//--------------------------
		// CONFIG
		//--------------------------
		case SYSCALL8_OPCODE_READ_COBRA_CONFIG:
			return sys_read_cobra_config((CobraConfig *)param1);
		break;

		case SYSCALL8_OPCODE_WRITE_COBRA_CONFIG:
			return sys_write_cobra_config((CobraConfig *)param1);
		break;

		//--------------------------
		// PSP
		//--------------------------
		case SYSCALL8_OPCODE_SET_PSP_UMDFILE:
			return sys_psp_set_umdfile((char *)param1, (char *)param2, param3);
		break;

		case SYSCALL8_OPCODE_SET_PSP_DECRYPT_OPTIONS:
			return sys_psp_set_decrypt_options(param1, param2, (uint8_t *)param3, param4, param5, (uint8_t *)param6, param7);
		break;

		case SYSCALL8_OPCODE_READ_PSP_HEADER:
			return sys_psp_read_header(param1, (char *)param2, param3, (uint64_t *)param4);
		break;

		case SYSCALL8_OPCODE_READ_PSP_UMD:
			return sys_psp_read_umd(param1, (void *)param2, param3, param4, param5);
		break;

		case SYSCALL8_OPCODE_PSP_PRX_PATCH:
			return sys_psp_prx_patch((uint32_t *)param1, (uint8_t *)param2, (void *)param3);
		break;

		case SYSCALL8_OPCODE_PSP_CHANGE_EMU:
			return SUCCEEDED; // deprecated OPCODE for sys_psp_set_emu_path((char *)param1);
		break;

		case SYSCALL8_OPCODE_PSP_POST_SAVEDATA_INITSTART:
			return sys_psp_post_savedata_initstart(param1, (void *)param2);
		break;

		case SYSCALL8_OPCODE_PSP_POST_SAVEDATA_SHUTDOWNSTART:
			return sys_psp_post_savedata_shutdownstart();
		break;

		//--------------------------
		// MAP PATHS
		//--------------------------
		case SYSCALL8_OPCODE_AIO_COPY_ROOT:
			return sys_aio_copy_root((char *)param1, (char *)param2);
		break;		

		case SYSCALL8_OPCODE_MAP_PATHS:
			return sys_map_paths((char **)param1, (char **)param2, param3);
		break;		

		//--------------------------
		// ADVANCED POKE (syscall8)
		//--------------------------
		case SYSCALL8_OPCODE_POKE_LV2:
			*(uint64_t *)param1 = param2;
			clear_icache((void *)param1, 8);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE32_LV2:
			*(uint32_t *)param1 = (uint32_t)param2;
			clear_icache((void *)param1, 4);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE16_LV2:
			*(uint16_t *)param1 = (uint16_t)param2;
			clear_icache((void *)param1, 2);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE8_LV2:
			*(uint8_t *)param1 = (uint8_t)param2;
			clear_icache((void *)param1, 1);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE_LV1:
			lv1_poked(param1, param2);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE32_LV1:
			lv1_pokew(param1, (uint32_t)param2);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE16_LV1:
			lv1_pokeh(param1, (uint16_t)param2);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_POKE8_LV1:
			lv1_pokeb(param1, (uint8_t)param2);
			return SUCCEEDED;
		break;		

		//--------------------------
		// VSH PLUGINS
		//--------------------------
		case SYSCALL8_OPCODE_LOAD_VSH_PLUGIN:
			return sys_prx_load_vsh_plugin(param1, (char *)param2, (void *)param3, param4);
		break;

		case SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN:
			return sys_prx_unload_vsh_plugin(param1);
		break;

#ifdef DEBUG
		case SYSCALL8_OPCODE_DUMP_STACK_TRACE:
			dump_stack_trace3((void *)param1, 16);
			return SUCCEEDED;
		break;

		case SYSCALL8_OPCODE_PSP_SONY_BUG:
			return sys_psp_sony_bug((uint32_t *)param1, (void *)param2, param3);
		break;
#endif

		default:
			if (1 <= ps3mapi_partial_disable_syscall8)	
				return ENOSYS;

			if (extended_syscall8.addr)
			{
				// Lets handle a few hermes opcodes ourself, and let their payload handle the rest
				if (function == 2)				
					return (uint64_t)_sys_cfw_memcpy((void *)param1, (void *)param2, param3);				
				else if (function == 0xC)
				{
					//DPRINTF("Hermes copy inst: %lx %lx %lx\n", param1, param2, param3);
				}
				else if (function == 0xD)
				{
					//DPRINTF("Hermes poke inst: %lx %lx\n", param1, param2);
					_sys_cfw_new_poke((void *)param1, param2);
					return param1;
				}

				int64_t (* syscall8_hb)() = (void *)&extended_syscall8;

				//DPRINTF("Handling control to HB syscall 8 (opcode = 0x%lx)\n", function);
				return syscall8_hb(function, param1, param2, param3, param4, param5, param6, param7);
			}
			else // if (function >= 0x9800) // AV: allow peek all other addresses
			{
				// Partial support for lv1_peek here
				return lv1_peekd(function);
			}
	}

	DPRINTF("Unsupported syscall8 opcode: 0x%lx\n", function);
	return ENOSYS;
}

static INLINE void apply_kernel_patches(void)
{
	for (int i = 0; i < N_KERNEL_PATCHES; i++)
	{
		uint32_t *addr = (uint32_t *)MKA(kernel_patches[i].address);
		*addr = kernel_patches[i].data;
		clear_icache(addr, 4);
	}

	create_syscalls();
}

static void loaded_cobra()
{
	// Cobra loaded successfully
	cellFsUtilMount_h("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0, 0, 0, 0, 0);
	cellFsRename(CB_LOCATION ".bak", CB_LOCATION);
	cellFsUtilUmount("/dev_blind", 0, 1);
}

int main(void)
{
#ifdef DEBUG
	debug_init();
	debug_install();
	extern uint64_t _start;
	extern uint64_t __self_end;
	DPRINTF("Stage 2 says hello (load base = %p, end = %p) (version = %08X)\n", &_start, &__self_end, MAKE_VERSION(COBRA_VERSION, FIRMWARE_VERSION, IS_CFW));
#endif

	storage_ext_init();
	modules_patch_init();

	apply_kernel_patches();
	map_path_patches(1);
	storage_ext_patches();
	region_patches();
	fan_patches();
	loaded_cobra();
	
	//map_path("/app_home", "/dev_usb000", 0); //Not needed

	return SUCCEEDED;
}
