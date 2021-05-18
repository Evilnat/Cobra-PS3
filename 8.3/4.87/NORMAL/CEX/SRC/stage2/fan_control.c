#include <lv2/symbols.h>
#include <lv2/syscall.h>
#include <lv2/thread.h>
#include <lv2/time.h>
#include <lv2/ctrl.h>
#include "common.h"
#include "fan_control.h"

#define SYS_SHUTDOWN			0x0100
#define SYS_SHUTDOWN2 			0x1100

uint8_t fan_control_running = 0;
uint8_t fan_speed;
uint8_t ps2_speed;

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
	//DPRINTF("CONTROL FAN Payload: Started.\n");

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

			// 60°C=38%, 61°C=40%, 62°C=42%, 63°C=44%, 64°C=46%, 65°C=48%, 66°C=50%, 67°C=52%, 68°C=54%, 69°C=56%
			// 70°C=57%, 71°C=60%, 72°C=63%, 73°C=66%, 74°C=69%, 75°C=72%, 76°C=75%, 77°C=78%, 78°C=81%,+79°C=84%
			// >80°C=88%
			// >82°C=98% (Overheating)	

			if(t_cpu >= 82)
				sm_set_fan_policy(0, 2, 0xF9); // 98% (Overheating)	
			else if(t_cpu >= 80)
				sm_set_fan_policy(0, 2, 0xE0); // 88%
			else if(t_cpu >= 70)
				sm_set_fan_policy(0, 2, 0x90 + 0x8 * (t_cpu - 70)); // 57% + 3% per degree °C
			else if(t_cpu >= 60)
				sm_set_fan_policy(0, 2, 0x60 + 0x5 * (t_cpu - 60)); // 37% + 2% per degree °C
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
		//DPRINTF("Shutdown executed, resetting FAN policy\n");
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
