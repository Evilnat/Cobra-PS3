#include <lv2/lv2.h>
#include <lv2/symbols.h>
#include <lv2/patch.h>
#include <lv2/syscall.h>
#include <lv2/memory.h>
#include <lv2/thread.h>
#include <lv2/time.h>
#include <lv2/ctrl.h>
#include "common.h"
#include "fan_control.h"

#define SYS_SHUTDOWN			0x0100
#define SYS_SHUTDOWN2 			0x1100

uint8_t fan_control_running = 0;
uint8_t fan_speed;

int sm_get_fan_speed(void)
{
	uint8_t st;
	uint8_t mode;
	uint8_t _fan_speed;
	uint8_t unk;

	sm_get_fan_policy(0, &st, &mode, &_fan_speed, &unk);

	return _fan_speed;
}

static void fan_control(uint64_t arg0)
{
	DPRINTF("CONTROL FAN Payload: Started.\n");

	uint32_t t_cpu, t_rsx, prev = 0; 
	fan_control_running = 1;

	while(fan_control_running)
	{
		timer_usleep(SECONDS(10));

		if(fan_control_running) // Avoids loading previous mode [Evilnat]
		{
			t_cpu = t_rsx = 0;
			
			sm_get_temperature(0, &t_cpu);
			sm_get_temperature(1, &t_rsx);

			if(t_rsx > t_cpu) 
				t_cpu = t_rsx;
			
			if(prev == t_cpu) 
				continue;

			// 60°C=31%, 61°C=33%, 62°C=35%, 63°C=37%, 64°C=39%, 65°C=41%, 66°C=43%, 67°C=45%, 68°C=47%, 69°C=49%
			// 70°C=50%, 71°C=53%, 72°C=56%, 73°C=59%, 74°C=62%, 75°C=65%, 76°C=68%, 77°C=71%, 78°C=74%,+79°C=80%

			if(t_cpu > 78)
				sm_set_fan_policy(0, 2, 0xCC); // 80%  // Backup 0xCC
			else if(t_cpu >= 70)
				sm_set_fan_policy(0, 2, 0x80 + 0x8*(t_cpu - 70)); // 50% + 3% per degree °C // Backup 0x80
			else if(t_cpu >= 60)
				sm_set_fan_policy(0, 2, 0x50 + 0x5*(t_cpu - 60)); // 30% + 2% per degree °C // Backup 0x50
			else
				sm_set_fan_policy(0, 1, 0); // SYSCON < 60°C

			prev = t_cpu;
		}
	}

	ppu_thread_exit(0);
}

void do_fan_control(void)
{
	thread_t fan_control_id;

	if(!fan_control_running)	
		ppu_thread_create(&fan_control_id, fan_control, 0, -0x1D8, 0x4000, 0, "fan_control");	
}

void load_fan_control(void)
{
	if(fan_speed == 0) // Disabled
		return;
	else if(fan_speed >= 0x33 && fan_speed <= 0xFE) // Manual mode
		sm_set_fan_policy(0, 2, fan_speed); 
	else if(fan_speed == 1)  // SYSCON mode
		sm_set_fan_policy(0, 1, 0);
	else
		do_fan_control();  // Dynamic fan control
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_3(int, sm_set_fan_policy_sc, (uint8_t unk, uint8_t _fan_mode, uint8_t _fan_speed))
{
	fan_control_running = 0; // disable internal fan control if fan speed is set manually via syscall 389

	return DO_POSTCALL;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_3(int, sys_shutdown, (uint16_t op, const void *buf, uint64_t size))
{
	// Avoids max FAN speed after a shutdown by Evilnat
	// This doesn't work if the PS3 has lv2 panic
	if(op == SYS_SHUTDOWN || op == SYS_SHUTDOWN2)
	{		
		DPRINTF("Shutdown executed, resetting FAN policy\n");
		sm_set_fan_policy(0, 1, 0);
	}

	return DO_POSTCALL;
}

void fan_patches(void)
{
	hook_function_with_cond_postcall(get_syscall_address(SYS_SM_SET_FAN_POLICY), sm_set_fan_policy_sc, 3);
	hook_function_with_cond_postcall(get_syscall_address(SYS_SM_SHUTDOWN), sys_shutdown, 3);
}

void unhook_all_fan_patches(void)
{
	suspend_intr();
	unhook_function_with_precall(get_syscall_address(SYS_SM_SET_FAN_POLICY), sm_set_fan_policy_sc, 3);
	unhook_function_with_precall(get_syscall_address(SYS_SM_SHUTDOWN), sys_shutdown, 3);
	resume_intr();
}
