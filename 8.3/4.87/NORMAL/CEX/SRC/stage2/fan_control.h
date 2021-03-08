#ifndef __FAN_CONTROL_H__
#define __FAN_CONTROL_H__

extern uint8_t fan_control_running;
extern uint8_t fan_speed;

int sm_get_fan_speed(void);
void do_fan_control();
void load_fan_control(void);

void fan_patches(void);
void unhook_all_fan_patches(void);

#endif /* __FAN_CONTROL_H__ */
