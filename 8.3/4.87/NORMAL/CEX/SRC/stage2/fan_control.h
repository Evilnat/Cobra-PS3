#ifndef __FAN_CONTROL_H__
#define __FAN_CONTROL_H__

#include <lv2/syscall.h>

extern uint8_t fan_control_running;
extern uint8_t fan_speed;

int sm_get_fan_policy_sc(uint8_t id, uint8_t *st, uint8_t *policy, uint8_t *mode, uint8_t *duty);
int sm_set_fan_policy_sc(uint8_t unk, uint8_t _fan_mode, uint8_t _fan_speed);

int sm_set_fan_policy(uint8_t unk, uint8_t _fan_mode, uint8_t fan_speed);
int sm_get_fan_speed(void);
void do_fan_control();
void load_fan_control(void);

#endif /* __FAN_CONTROL_H__ */
