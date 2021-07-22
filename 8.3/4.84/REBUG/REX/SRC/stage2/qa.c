//---------------------------------------------------
// Advanced QA Flag & Debug Settings Enabler by habib
//---------------------------------------------------

// Improved by Evilnat

#include <lv2/security.h>
#include <lv2/thread.h>
#include <lv2/patch.h>
#include <lv2/error.h>
#include <lv1/patch.h>
#include <lv1/mm.h>
#include "ps3mapi_core.h"
#include "common.h"
#include "storage_ext.h"
#include "savegames.h"
#include "qa.h"

static u8 erk[0x20] = 
{
	0x34, 0x18, 0x12, 0x37, 0x62, 0x91, 0x37, 0x1c,
	0x8b, 0xc7, 0x56, 0xff, 0xfc, 0x61, 0x15, 0x25,
	0x40, 0x3f, 0x95, 0xa8, 0xef, 0x9d, 0x0c, 0x99,
	0x64, 0x82, 0xee, 0xc2, 0x16, 0xb5, 0x62, 0xed
};

static u8 hmac[0x40] = 
{
	0xcc, 0x30, 0xc4, 0x22, 0x91, 0x13, 0xdb, 0x25,
	0x73, 0x35, 0x53, 0xaf, 0xd0, 0x6e, 0x87, 0x62,
	0xb3, 0x72, 0x9d, 0x9e, 0xfa, 0xa6, 0xd5, 0xf3,
	0x5a, 0x6f, 0x58, 0xbf, 0x38, 0xff, 0x8b, 0x5f,
	0x58, 0xa2, 0x5b, 0xd9, 0xc9, 0xb5, 0x0b, 0x01,
	0xd1, 0xab, 0x40, 0x28, 0x67, 0x69, 0x68, 0xea,
	0xc7, 0xf8, 0x88, 0x33, 0xb6, 0x62, 0x93, 0x5d,
	0x75, 0x06, 0xa6, 0xb5, 0xe0, 0xf9, 0xd9, 0x7a
};

static u8 iv_qa[0x10] = 
{
	0xe8, 0x66, 0x3a, 0x69, 0xcd, 0x1a, 0x5c, 0x45,
	0x4a, 0x76, 0x1e, 0x72, 0x8c, 0x7c, 0x25, 0x4e
};

static void lv1_poked2(u64 addr, u64 value)
{
	*(u64 *)(HV_BASE + addr) = value;
}

static void lv1_patches()
{
	lv1_pokew(UM_PATCH_OFFSET, LI(R0, 0)); 
    lv1_pokew(DM_PATCH1_OFFSET, NOP); 
    lv1_pokew(DM_PATCH2_OFFSET, LI(R3, 1)); 
    lv1_pokew(DM_PATCH3_OFFSET, LI(R31, 1)); 
    lv1_pokew(DM_PATCH4_OFFSET, LI(R3, 0)); 
}

static void restore_patches()
{
	lv1_pokew(UM_PATCH_OFFSET, UM_PATCH_ORI); 
    lv1_pokew(DM_PATCH1_OFFSET, DM_PATCH1_ORI); 
    lv1_pokew(DM_PATCH2_OFFSET, DM_PATCH2_ORI); 
    lv1_pokew(DM_PATCH3_OFFSET, DM_PATCH3_ORI);
    lv1_pokew(DM_PATCH4_OFFSET, DM_PATCH4_ORI); 
}

uint8_t read_qa_flag()
{
	uint8_t value = 0;
	update_mgr_read_eeprom(QA_FLAG_OFFSET, &value, LV2_AUTH_ID);
	return (value == 0x00);
}

int set_qa_flag(uint8_t value)
{
	uint8_t seed[TOKEN_SIZE];
	uint8_t token[TOKEN_SIZE];	

	uint64_t IDPS_1 = lv1_peekd(IDPS_LV1);
	uint64_t IDPS_2 = lv1_peekd(IDPS_LV1 + 8);

	memset(seed, 0, TOKEN_SIZE);
	memcpy(seed + 4, (void *)&IDPS_1, 8);
	memcpy(seed + 12, (void *)&IDPS_2, 8);

	if(seed[0x07] != 0x01)
	{
		//DPRINTF("QA Flag: IDPS is not valid!!\n");
		return 1;
	}

	seed[3] = 1;

	if(value)
	{
		seed[39] |= 0x1; // QA_FLAG_EXAM_API_ENABLE
		seed[39] |= 0x2; // QA_FLAG_QA_MODE_ENABLE

		seed[47] |= 0x2; // checked by lv2_kernel.self and sys_init_osd.self 
		seed[47] |= 0x4; // can run sys_init_osd.self from /app_home ?

		seed[51] |= 0x1; // QA_FLAG_ALLOW_NON_QA
		seed[51] |= 0x2; // QA_FLAG_FORCE_UPDATE
	}

	sha1_hmac(hmac, 0x40, seed, 60, seed + 60);
	aescbccfb_enc(token, seed, 0x50, erk, 0x100, iv_qa);

	//DPRINTF("QA Flag: %s...\n", (value) ? "Enabling" : "Disabling");

	lv1_patches();

	struct ps3dm_scm_write_eeprom write_eeprom;
	int len;
	u8 *p = (u8*)&write_eeprom;
	u64 laid, paid, vuart_lpar_addr, muid, nwritten;

	if(lv1_allocate_memory(4096, 0x0C, 0, 0, &vuart_lpar_addr, &muid) != 0)
	{
		restore_patches();
		return 2;
	}

	if(mm_map_lpar_memory_region(vuart_lpar_addr, HV_BASE, HV_SIZE2, HV_PAGE_SIZE, 0) != 0)
	{
		restore_patches();
		return 3;
	}

	laid = 0x1070000002000001ULL;
	paid = 0x1070000033000001ULL;

	memset(&write_eeprom, 0, sizeof(write_eeprom));

	ps3dm_init_header(&write_eeprom.dm_hdr, 1, PS3DM_FID_SCM,
		sizeof(write_eeprom)	-	sizeof(struct ps3dm_header),
		sizeof(write_eeprom)	-	sizeof(struct ps3dm_header));

	ps3dm_init_ss_header(&write_eeprom.ss_hdr, PS3DM_PID_SCM_WRITE_EEPROM, laid, paid);
	write_eeprom.offset = 0x48D3E;
	write_eeprom.nwrite = 0x50;
	write_eeprom.buf_size = 0x50;
	memset(write_eeprom.buf, 0, sizeof(write_eeprom.buf));
	memcpy(write_eeprom.buf, token, 0x50);
	len = sizeof(write_eeprom);

	for(u16 n = 0; n < len ; n += 8)
	{
		static u64 value;
		memcpy(&value, &p[n], 8);
		lv1_poked2((u64) n, value);
		__asm__("sync");
		memcpy(&value, (void *)0x8000000000000000ULL, 8);
	}

	if(lv1_write_virtual_uart(10, vuart_lpar_addr, len, &nwritten) != 0)
	{
		restore_patches();
		return 4;
	}

	update_mgr_write_eeprom(QA_FLAG_OFFSET, (value) ? 0x00 : 0xFF, LV2_AUTH_ID); 

	//DPRINTF("QA Flag: %s\n", (value) ? "Enabled (Value: 0x00)" : "Disabled (Value: 0xFF)");
	
	restore_patches();

	return SUCCEEDED;
}
