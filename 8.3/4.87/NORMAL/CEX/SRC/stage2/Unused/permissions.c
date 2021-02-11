#include <lv2/lv2.h>
#include <lv2/process.h>
#include <lv2/symbols.h>
#include <lv2/libc.h>
#include <lv2/error.h>
#include <lv2/patch.h>
#include "modulespatch.h"
#include "common.h"

uint32_t access_pid;
//uint32_t vsh_pid;

/* static char *system_processes[] =
{
	"_main_sys_init_osd.",
	"_main_vsh.self",
	"_main_mcore.self",
	"_main_bdp_BDVD.self",
	"_main_bdp_BDMV.self",
	"_main_psp_emulator.",
	"_main_psp_translato",
	"_main_ps1_netemu.se",
	"_main_ps1_emu.self",
	//"_main_EBOOT.BIN"
}; */

//#define N_SYSTEM_PROCESSES (sizeof(system_processes) / sizeof(char *))

/* int is_system_process(process_t process)
{
	for (int i = 0; i < N_SYSTEM_PROCESSES; i++)
	{
		if (strcmp(get_process_name(process)+8, system_processes[i]) == 0)
			return 1;
	}
	
	return 0;
} */

/* LV2_HOOKED_FUNCTION_COND_POSTCALL_4(int, permissions_func_hook, (void *r3, void *r4, void *r5, void *r6))
{
	process_t process = get_current_process_critical();
	
	if (process && is_system_process(process))
		return DO_POSTCALL;
	
	// Uncomment to do tests with original permissions except on cobra usb manager
	//if (!process || process->pid != access_pid)
	//	return DO_POSTCALL;
		
	uint32_t call_addr = (uint32_t)((uint64_t)get_patched_func_call_address() & 0xFFFFFFFF);	
	
	//if (process)
	//	DPRINTF("Function called from process %s, at %x\n", get_process_name(process)+8, call_addr);

	if (call_addr == permissions_exception2)	
		return DO_POSTCALL;	
	
	if (call_addr == permissions_exception1 || call_addr == permissions_exception3)	
		return (process == NULL);		
		
	return 1;
} */

LV2_PATCHED_FUNCTION(uint32_t, get_pid_patched, (process_t process))
{
	if (process)
	{
		if (vsh_process && access_pid != 0 && process->pid == access_pid)		
			return vsh_process->pid;		
		
		return process->pid;	
	}
	
	return -1;
}

int sys_permissions_get_access(void)
{
	access_pid = get_current_process_critical()->pid;	
	return 0;
}

int sys_permissions_remove_access(void)
{
	if (access_pid == get_current_process_critical()->pid)
	{
		access_pid = 0;
		return 0;
	}
	
	return ENOENT;
}

void permissions_patches(void)
{

	//hook_function_with_cond_postcall(permissions_func_symbol, permissions_func_hook, 4);
	patch_call(ss_pid_call_1, get_pid_patched);
}

/* void unhook_all_permissions(void)
{

   suspend_intr();	
	//unhook_function_with_cond_postcall(permissions_func_symbol, permissions_func_hook, 4);
	resume_intr();
} */

