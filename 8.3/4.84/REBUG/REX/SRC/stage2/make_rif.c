#include <lv2/libc.h>
#include <lv2/io.h>
#include <lv2/error.h>
#include <lv2/security.h>
#include <lv2/ctrl.h>
#include "common.h"
#include "mappath.h"
#include "modulespatch.h"
#include "ps3mapi_core.h"
#include "make_rif.h"
#include "savegames.h"

uint8_t skip_existing_rif = 0;

unsigned char RAP_KEY[] =  { 0x86, 0x9F, 0x77, 0x45, 0xC1, 0x3F, 0xD8, 0x90, 0xCC, 0xF2, 0x91, 0x88, 0xE3, 0xCC, 0x3E, 0xDF };
unsigned char RAP_PBOX[] = { 0x0C, 0x03, 0x06, 0x04, 0x01, 0x0B, 0x0F, 0x08, 0x02, 0x07, 0x00, 0x05, 0x0A, 0x0E, 0x0D, 0x09 };
unsigned char RAP_E1[] =   { 0xA9, 0x3E, 0x1F, 0xD6, 0x7C, 0x55, 0xA3, 0x29, 0xB7, 0x5F, 0xDD, 0xA6, 0x2A, 0x95, 0xC7, 0xA5 };
unsigned char RAP_E2[] =   { 0x67, 0xD4, 0x5D, 0xA3, 0x29, 0x6D, 0x00, 0x6A, 0x4E, 0x7C, 0x53, 0x7B, 0xF5, 0x53, 0x8C, 0x74 };

static void get_rif_key(unsigned char* rap, unsigned char* rif)
{
	int i;
	int round;

	unsigned char key[0x10];
	unsigned char iv[0x10];
	memset(key, 0, 0x10);
	memset(iv, 0, 0x10);

	// Initial decrypt.
	aescbccfb_dec(key, rap, 0x10, RAP_KEY, 0x80, iv);
	memset(iv, 0, 0x10);

	// rap2rifkey round.
	for (round = 0; round < 5; ++round)
	{
		for (i = 0; i < 16; ++i)
		{
			int p = RAP_PBOX[i];
			key[p] ^= RAP_E1[p];
		}

		for (i = 15; i >= 1; --i)
		{
			int p = RAP_PBOX[i];
			int pp = RAP_PBOX[i - 1];
			key[p] ^= key[pp];
		}

		int o = 0;

		for (i = 0; i < 16; ++i)
		{
			int p = RAP_PBOX[i];
			unsigned char kc = key[p] - o;
			unsigned char ec2 = RAP_E2[p];
			if (o != 1 || kc != 0xFF)
			{
				o = kc < ec2 ? 1 : 0;
				key[p] = kc - ec2;
			}
			else if (kc == 0xFF)			
				key[p] = kc - ec2;			
			else			
				key[p] = kc;			
		}
	}

	memcpy(rif, key, 0x10);
}

static void read_act_dat_and_make_rif(uint8_t *rap, uint8_t *act_dat, const char *content_id, const char *rif_path)
{
	int fd;

	if(cellFsOpen(rif_path, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0) == SUCCEEDED)
	{
		uint8_t idps_const[0x10]    = { 0x5E, 0x06, 0xE0, 0x4F, 0xD9, 0x4A, 0x71, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
		uint8_t rif_key_const[0x10] = { 0xDA, 0x7D, 0x4B, 0x5E, 0x49, 0x9A, 0x4F, 0x53, 0xB1, 0xC1, 0xA1, 0x4A, 0x74, 0x84, 0x44, 0x3B };

		uint8_t *rif = ALLOC_RIF_BUFFER;
		uint8_t *key_index = rif + 0x40;
		uint8_t *rif_key = rif + 0x50;
		memset(rif, 0, 0x70);

		get_rif_key(rap, rif_key); //convert rap to rifkey (klicensee)

		uint8_t *iv = rif + 0x60;
		aescbccfb_enc(idps_const, idps_const, 0x10, (void*)PS3MAPI_IDPS_2, IDPS_KEYBITS, iv);

		uint8_t *act_dat_key = rap;
		memcpy(act_dat_key, act_dat + 0x10, 0x10);

		memset(iv, 0, 0x10);
		aescbccfb_dec(act_dat_key, act_dat_key, 0x10, idps_const, IDPS_KEYBITS, iv);

		memset(iv, 0, 0x10);
		aescbccfb_enc(rif_key, rif_key, 0x10, act_dat_key, ACT_DAT_KEYBITS, iv);

		memset(iv, 0, 0x10);
		aescbccfb_enc(key_index, key_index, 0x10, rif_key_const, RIF_KEYBITS, iv);

		const uint32_t version_number = 1;
		const uint32_t license_type = 0x00010002;
		const uint64_t timestamp = 0x000001619BF6DDCA;
		const uint64_t expiration_time = 0;

		memcpy(rif,        &version_number,  4); // 0x00 version_number
		memcpy(rif + 0x04, &license_type,    4); // 0x04 license_type
		memcpy(rif + 0x08, act_dat + 0x8,    8); // 0x08 account_id
		memcpy(rif + 0x10, content_id,    0x24); // 0x10 content_id
												 // 0x40 encrypted key index (Used for choosing act.dat key)
												 // 0x50 encrypted rif_key
		memcpy(rif + 0x60, &timestamp,       8); // 0x60 timestamp
		memcpy(rif + 0x68, &expiration_time, 8); // 0x68 expiration time

		uint64_t size;
		memset(rif + 0x70, 0x11, 0x28);			 // 0x70 ECDSA Signature
		cellFsWrite(fd, rif, 0x98, &size);
		cellFsClose(fd);
	}
}

int create_act_dat(const char *userid)
{
	int fd;
	uint64_t size;
	char full_path[120], exdata_dir[120];
	CellFsStat stat;

	//DPRINTF("Creating act.dat for userID %s...\n", userid);

	uint8_t timedata[0x10] = 
	{ 
		0x00, 0x00, 0x01, 0x2F, 0x3F, 0xFF, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	const uint64_t header = 0x0000000100000002;

	uint64_t accountID = (uint64_t)strtoull((const char*)account_id, NULL, 16);
	accountID = SWAP64(accountID);
	
	uint8_t *actdat = malloc(0x1038);	
	memset(actdat, 0x11, 0x1038);
	memcpy(actdat, &header, 8);
	memcpy(actdat + 8, &accountID, 8);
	memcpy(actdat + 0x870, timedata, 0x10);

	sprintf(exdata_dir, "/dev_hdd0/home/%s/exdata", userid);

	if(cellFsStat(exdata_dir, &stat) != SUCCEEDED)				
		cellFsMkdir(exdata_dir, 0777);		

	sprintf(full_path, "%s/act.dat", exdata_dir);

	cellFsOpen(full_path, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0);
	cellFsWrite(fd, actdat, 0x1038, &size);
	cellFsClose(fd);

	free(actdat);

	return SUCCEEDED;
}

void make_rif(const char *path)
{		
	char buffer[120];	
	int path_len = strlen(path);	
	
	if(!strncmp(path, "/dev_hdd0/home/", 15) && !strcmp(path + path_len - 4, ".rif"))
	{		
		int act_dat_found = 0;
		CellFsStat stat;		
		
		DPRINTF("open_path_hook: %s (looking for rap)\n", path);

		char *content_id = ALLOC_CONTENT_ID;
		memset(content_id, 0, 0x25);
		strncpy(content_id, strrchr(path, '/') + 1, 0x24);

		char *rap_path = ALLOC_PATH_BUFFER;

		uint8_t is_ps2_classic = !strncmp(content_id, "2P0001-PS2U10000_00-0000111122223333", 0x24);
		uint8_t is_psp_launcher = !strncmp(content_id, "UP0001-PSPC66820_00-0000111122223333", 0x24);

		if(!is_ps2_classic && !is_psp_launcher)
		{
			CellFsStat stat;
			const char *ext = "rap";

			// Support for rap and RAP extension (By aldostools)
			for(uint8_t i = 0; i < 2; i++)
			{
				sprintf(rap_path, "/dev_usb000/exdata/%36s.%s", content_id, ext);

				if(cellFsStat(rap_path, &stat) != SUCCEEDED) 
					rap_path[10] = '1'; //dev_usb001
				if(cellFsStat(rap_path, &stat) != SUCCEEDED) 
					sprintf(rap_path, "/dev_hdd0/exdata/%36s.%s", content_id, ext);

				if(cellFsStat(rap_path, &stat) != SUCCEEDED) 
					ext = "RAP"; 
				else 
					break;
			}
		}

		int fd;
		if(is_ps2_classic || is_psp_launcher || cellFsOpen(rap_path, CELL_FS_O_RDONLY, &fd, 0666, NULL, 0) == SUCCEEDED)
		{
			uint64_t nread = 0;
			uint8_t rap[0x10] = { 0xF5, 0xDE, 0xCA, 0xBB, 0x09, 0x88, 0x4F, 0xF4, 0x02, 0xD4, 0x12, 0x3C, 0x25, 0x01, 0x71, 0xD9 };

			if(!is_ps2_classic && !is_psp_launcher)
			{
				cellFsRead(fd, rap, 0x10, &nread);
				cellFsClose(fd);
			}

			//DPRINTF("rap_path: %s output: %s\n", rap_path, path);

			// Search act.dat in home dirs
			for(int i = 1; i < 100; i++)
			{
				sprintf(buffer, ACTDAT_PATH, i);
				
				if(cellFsStat(buffer, &stat) == SUCCEEDED) 
				{
					//DPRINTF("Found act.dat in %08d\n", i);
					act_dat_found = 1;
					break;
				}	
			}			

			char userid[8];
			strncpy(userid, path + 15, 8);
			userid[8] = '\0';

			sprintf(buffer, ACCOUNTID_VALUE, userid);
			
			if(!act_dat_found && xreg_data(buffer, ACCOUNTID, READ, 0, 1))
				create_act_dat(userid);	

			act_dat_found = 0;

			// Skip the creation of rif license if it already exists - By aldostool
			if(skip_existing_rif && cellFsStat(path, &stat) == SUCCEEDED)			
				return;			
			
			char *act_path = ALLOC_PATH_BUFFER;
			memset(act_path, 0, 0x50);
			strncpy(act_path, path, strrchr(path, '/') - path);
			strcpy(act_path + strlen(act_path), "/act.dat\0");

			//DPRINTF("act_path: %s content_id: %s\n", act_path, content_id);

			if(cellFsOpen(act_path, CELL_FS_O_RDONLY, &fd, 0666, NULL, 0) == SUCCEEDED)
			{
				uint8_t *act_dat = ALLOC_ACT_DAT;
				cellFsRead(fd, act_dat, 0x20, &nread); // size: 0x1038 but only first 0x20 are used to make rif
				cellFsClose(fd);

				if(nread == 0x20)
				{
					char *rif_path = ALLOC_PATH_BUFFER;
					sprintf(rif_path, "/%s", path);
					read_act_dat_and_make_rif(rap, act_dat, content_id, rif_path);
				}
			}			
		}
	}
}
