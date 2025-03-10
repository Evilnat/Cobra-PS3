#include <lv2/lv2.h>
#include <lv2/security.h>
#include <lv2/memory.h>

#include "common.h"
#include "drm.h"
#include "crypto.h"

uint8_t drm_data[24];

void drm_init(void)
{
	uint8_t open_psid[16];
	
	get_pseudo_random_number(drm_data, 8);

	#ifdef DEBUG
		//DPRINT_HEX(drm_data, 8);
	#endif

	ss_get_open_psid(open_psid, 0);
	
	for (int i = 0; i < 8; i++)
	{
		if (i & 3)		
			drm_data[i + 8] = open_psid[13 - i] ^ drm_data[i];		
		else		
			drm_data[i + 8] = open_psid[i] ^ open_psid[15 - i] ^ drm_data[i + 1];		
		
		if (i == 6)		
			drm_data[i + 8] += (open_psid[9] ^ 0x43);		
	}
	
	xtea_ctr(drm_data, drm_data[0]+drm_data[12], drm_data + 16, 8);

	#ifdef DEBUG	
		//DPRINT_HEX(drm_data, sizeof(drm_data));
	#endif
}

int sys_drm_get_data(void *data, uint64_t param)
{
	return copy_to_user(drm_data, get_secure_user_ptr(data), sizeof(drm_data));
}
