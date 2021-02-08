// Created and improved by Evilnat

#ifndef __LV2_FAN_H__
#define __LV2_FAN_H__

#include <stdint.h>
#include <sys/types.h>
#include <lv2/lv2.h>
#include <lv2/symbols.h>

#define SYSM_OBJ_OFFSET 		 0x8000000000347E40ULL

uint64_t *sysm_obj;

LV2_EXPORT int sm_get_temperature_internal(uint64_t *sysm_obj, int id, uint32_t *temp, int time_out);
LV2_EXPORT int sm_get_fan_policy_internal(uint64_t *sysm_obj, uint8_t id, uint8_t *st, uint8_t *mode, uint8_t *speed, uint8_t *unk, int time_out);
LV2_EXPORT int sm_set_fan_policy_internal(uint64_t *sysm_obj, uint8_t unk, uint8_t mode, uint8_t speed);


static INLINE void sm_get_temperature(int id, uint32_t *temp)
{
	memcpy(&sysm_obj, (void *)SYSM_OBJ_OFFSET, 8); 
	sm_get_temperature_internal(sysm_obj, id, temp, 1000000);
	*temp >>= 24; // return °C
}

static INLINE void sm_get_fan_policy(uint8_t id, uint8_t *st, uint8_t *mode, uint8_t *speed, uint8_t *unk)
{
	memcpy(&sysm_obj, (void *)SYSM_OBJ_OFFSET, 8); 
	sm_get_fan_policy_internal(sysm_obj, id, st, mode, speed, unk, 1000000);
}

static INLINE int sm_set_fan_policy(uint8_t unk, uint8_t mode, uint8_t speed)
{
	memcpy(&sysm_obj, (void *)SYSM_OBJ_OFFSET, 8); 
	sm_set_fan_policy_internal(sysm_obj, unk, mode, speed);

	return 0;
}

#endif /* __LV2_FAN_H__ */
