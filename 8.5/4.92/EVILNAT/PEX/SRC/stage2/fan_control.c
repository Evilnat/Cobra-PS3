#include <lv2/symbols.h>
#include <lv2/syscall.h>
#include <lv2/thread.h>
#include <lv2/time.h>
#include <lv2/ctrl.h>
#include "common.h"
#include "fan_control.h"

#define SYS_SHUTDOWN			0x0100
#define SYS_SHUTDOWN2 			0x1100

thread_t fan_control_id = 0, control_id = 0;
uint8_t fan_control_running = 0;
uint8_t fan_speed;
uint8_t ps2_speed;

int sm_get_fan_speed(void)
{
	uint8_t st, mode, _fan_speed, unk;
	sm_get_fan_policy(0, &st, &mode, &_fan_speed, &unk);
	return _fan_speed;
}

// Optimized by aldostools and Evilnat
// Keeps if possible that the temperature of the CPU and RSX do not exceed the temperature chosen between 60°C, 65°C, 70°C or 75°C
static void fan_control(uint64_t arg0)
{
	//DPRINTF("Cobra's Dynamic Control FAN: Started.\n");	

	int speed = 0x80, max_temp = 65, warning = 0;
	uint32_t t_cpu, t_rsx;
	fan_control_running = 1;

	if(arg0 == 2)
		max_temp = 60;
	else if(arg0 == 3)
		max_temp = 65;
	else if(arg0 == 4)
		max_temp = 70;
	else if(arg0 == 5)
		max_temp = 75;

	timer_usleep(SECONDS(2));

	while(fan_control_running)
	{
		timer_usleep(SECONDS(3));

		// Avoids loading previous mode (By Evilnat)
		if (!fan_control_running) 
			break;

		t_cpu = t_rsx = 0;
		
		sm_get_temperature(0, &t_cpu);
		sm_get_temperature(1, &t_rsx);

		if(t_rsx > t_cpu) 
			t_cpu = t_rsx;

		if(t_cpu > max_temp && t_cpu < 80) // Temp under max_temp (60°C/65°C/70°C/75°C) and 80°C
		{		
			warning = 0;
			speed = speed + 5; // +2%

			// Maximum fan speed
			if(speed > 0xFF)
				speed = 0xFF;
		}
		else if(t_cpu <= max_temp) // Temp lower or equal to max_temp (60°C/65°C/70°C/75°C)
		{
			if(t_cpu >= (max_temp - 2)) 
				continue;

			warning = 0;
			speed = speed - 4; // -1.5%

			// Minimium fan speed, probably a lower speed can create problems
			if(speed < 0x55)
                speed = 0x55; // 33%
		}
		else if(t_cpu >= 80) // Temp higher or equal to 80°C (Overheating!!)
		{
			// Overheating
			// Let's warn the user with triple beep and we set fan speed to 0xF8	
			if(!warning)
				sm_ring_buzzer(TRIPLE_BEEP);

			warning = 1;
			speed = 0xF8; // 97%
		}

		sm_set_fan_policy(0, 2, speed);	
	}

	//DPRINTF("Cobra's Dynamic Control FAN: Finished.\n");

	ppu_thread_exit(0);
}

static void start_control(uint64_t arg0)
{
	// Waiting until the old thread is finished
	timer_usleep(SECONDS(5));

	if(!fan_control_running)	
		ppu_thread_create(&fan_control_id, fan_control, (uint64_t)arg0, -0x1D8, 0x4000, 1, "start_control");	

	ppu_thread_exit(0);
}

void do_fan_control(int mode)
{	
	fan_control_running = 0;

	if(!fan_control_running)	
		ppu_thread_create(&control_id, start_control, (uint64_t)mode, -0x1D8, 0x4000, 1, "fan_control");	
}

void load_fan_control(int mode)
{
	if(fan_speed == 0) // Disabled
		return;
	else if(fan_speed >= 0x33 && fan_speed <= 0xFE) // Manual mode
		sm_set_fan_policy(0, 2, fan_speed); 
	else if(fan_speed == 1) // SYSCON mode
		sm_set_fan_policy(0, 1, 0);
	else
		do_fan_control(mode); // Dynamic fan control
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_3(int, sm_set_fan_policy_sc, (uint8_t unk, uint8_t _fan_mode, uint8_t _fan_speed))
{
	fan_control_running = 0; // Disable internal fan control if fan speed is set manually via syscall 389
	return DO_POSTCALL;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_3(int, sys_shutdown, (uint16_t op, const void *buf, uint64_t size))
{
	// Avoids max FAN speed after a shutdown (By Evilnat)
	// This may doesn't work if the PS3 has lv2 panic
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
