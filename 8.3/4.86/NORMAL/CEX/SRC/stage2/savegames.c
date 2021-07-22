#include <stdint.h>
#include <stdio.h>
#include <lv2/io.h>
#include <lv2/error.h>
#include <lv1/inttypes.h>
#include <lv1/patch.h>
#include <lv2/libc.h>
#include <lv2/memory.h>
#include <lv2/security.h>
#include <lv2/ctrl.h>
#include "common.h"
#include "modulespatch.h"
#include "ps3mapi_core.h"
#include "savegames.h"

#define SAVEGAME_HASH_OFFSET 			(prot_table + 0x90)
#define CONSOLE_ID_HASH_OFFSET 			(prot_table + 0x90 + 0x14)
#define DISC_HASH_OFFSET 				(prot_table + 0x90 + 0x28)
#define AUTHENTICATION_ID_HASH_OFFSET 	(prot_table + 0x90 + 0x3C)

typedef struct
{
    size_t size;
    char file_path[120];
    u8 savegame_param_sfo_hash[0x14];    
    u8 console_id_hash[0x14];  
    u8 disc_hash_hash[0x14];  
    u8 authentication_id_hash[0x14];  
} sfo_data_t;

typedef struct
{
    size_t size;
    char file_path[120];
    u8 vector[0x10];
    u8 real_hashkey[0x14];
    u8 default_hash[0x14];
} pfd_data_t;

static int prot_table = 0;
//uint8_t account_id[0x10];
//uint32_t userID = 0;

static u8 console_id_key[0x10];

static u8 savegame_param_sfo_key[0x14] =
{
	0x0C, 0x08, 0x00, 0x0E, 0x09, 0x05, 0x04, 0x04, 0x0D, 0x01, 
	0x0F, 0x00, 0x04, 0x06, 0x02, 0x02, 0x09, 0x06, 0x0D, 0x03
};

static u8 authentication_id[0x08] = 
{
    0x10, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03
};

static u8 disc_hash_key[0x10] = 
{
	0xD1, 0xC1, 0xE1, 0x0B, 0x9C, 0x54, 0x7E, 0x68, 
	0x9B, 0x80, 0x5D, 0xCD, 0x97, 0x10, 0xCE, 0x8D
};

static u8 keygen_key[0x14] =
{
	0x6B, 0x1A, 0xCE, 0xA2, 0x46, 0xB7, 0x45, 0xFD, 0x8F, 0x93, 
	0x76, 0x3B, 0x92, 0x05, 0x94, 0xCD, 0x53, 0x48, 0x3B, 0x82
};

static u8 syscon_manager_key[0x14] = 
{
	0xD4, 0x13, 0xB8, 0x96, 0x63, 0xE1, 0xFE, 0x9F, 
	0x75, 0x14, 0x3D, 0x3B, 0xB4, 0x56, 0x52, 0x74
};

static uint8_t empty[0x10] = 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t fake_accountid[0x10] = 
{
    0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30
};

static void generate_entry_hash(uint8_t real_key[0x14], uint8_t *buffer)
{
	sha1_context sha1;

	uint64_t entry;
	memcpy(&entry, buffer + 0xB8, 8);

	memset(&sha1, 0, sizeof(sha1_context));
	sha1_hmac_starts(&sha1, real_key, 0x14);

	if(entry != 0)
	{
		sha1_hmac_update(&sha1, buffer + prot_table + 0x110 * entry + 8, 0x41);
		sha1_hmac_update(&sha1, buffer + prot_table + 0x110 * entry + 0x50, 0xC0);
	}

	sha1_hmac_update(&sha1, buffer + prot_table + 8, 0x41);
	sha1_hmac_update(&sha1, buffer + prot_table + 0x50, 0xC0);
	sha1_hmac_finish(&sha1, buffer + Y_TABLE_OFFSET + 0x14 * 8);
}

static int readfile(const char *file, uint8_t *buffer, size_t size)
{
	int fd;
	uint64_t read;

	if(cellFsOpen(file, CELL_FS_O_RDWR, &fd, 0666, NULL, 0) != SUCCEEDED)
		return -1;

	if(cellFsRead(fd, buffer, size, &read) != SUCCEEDED)
		return -1;

	cellFsClose(fd);

	return SUCCEEDED;
}

static int savefile(const char *path, void *data, size_t size)
{
	int file;
	uint64_t write;

	if(cellFsOpen(path, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &file, 0666, NULL, 0) != SUCCEEDED)
		return -1;

	cellFsWrite(file, data, size, &write);
	cellFsClose(file);

	return SUCCEEDED;	
}

int set_fakeID(int userid, int overwrite)
{  
    char autoLogin[120], accountid[120];

    sprintf(autoLogin, SETTING_AUTOSIGN, userid);
    sprintf(accountid, SETTING_ACCOUNTID, userid);

    if(xreg_data(autoLogin, AUTOSIGN, WRITE, 0, 0) != SUCCEEDED)
        return 2;

    if(xreg_data(accountid, ACCOUNTID, WRITE, overwrite, 0) != SUCCEEDED)
        return 1;

    return 0;
}

int xreg_data(char *value, int type, int mode, int overwrite, int checkEmpty)
{
    int fd, result = -1; 
    uint16_t offset = 0;
    uint64_t dummy, read, seek;
    char *buffer = malloc(0x2A);    

    if(!buffer)
		return result;

    if(cellFsOpen(XREGISTRY_FILE, CELL_FS_O_RDWR, &fd, 0666, NULL, 0) != SUCCEEDED)
    {
		free(buffer);
		return result;
	}  

    // Get offset
    for(int i = 0; i < 0x10000; i++)
    {       
        cellFsLseek(fd, i, SEEK_SET, &seek);
        cellFsRead(fd, buffer, 0x31 + 1, &read);

        // Found offset
        if(strcmp(buffer, value) == 0) 
        {
            offset = i - 0x15;

            uint8_t *data = NULL;

            // Search value from value table
            for(int i = 0x10000; i < 0x15000; i++)
            {
            	data = (uint8_t *) malloc(0x17);

                cellFsLseek(fd, i, SEEK_SET, &seek);
                cellFsRead(fd, data, 0x17, &read);
                
                // Found value
                if (memcmp(data, &offset, 2) == 0 && data[4] == 0x00 && 
                    ((type) ? data[5] == 0x11 && data[6] == 0x02 : data[5] == 0x04 && data[6] == 0x01))
                {       
                    result = 0;      

                    if(type && mode) // Set/Overwrite with fake accountID
                    {                   
                        if(!overwrite && memcmp(data + 7, empty, 0x10) != SUCCEEDED)
                        {
                            free(data);
                            free(buffer);
                            cellFsClose(fd);
                            return 1;
                        }

                        cellFsLseek(fd, i + 7, SEEK_SET, &seek);
                        cellFsWrite(fd, fake_accountid, 0x10, &dummy);
                    }
                    else if(type && !mode) // Check if there is no accountID
                    {
                        memcpy(&account_id, data + 7, 0x10);

                        if(checkEmpty && memcmp(data + 7, empty, 0x10) != SUCCEEDED)                        
                            result = 1;                                                                    
                    } 
                    else if(!type && mode) // Disable auto sign in PSN with empty email/password
                    {
                        uint32_t disabled = 0;
                        cellFsLseek(fd, i + 7, SEEK_SET, &seek);
                        cellFsWrite(fd, &disabled, 4, &dummy);
                    }                                       
                    else                     
                        memcpy(&userID, data + 7, 4);

                    free(data);
					free(buffer);
					cellFsClose(fd);
					return result;
                }

                free(data);
            }
        }
    }

    free(buffer);
    cellFsClose(fd);    

    return result;
}

int patch_savedata(const char *path)
{
	CellFsStat stat;

	sfo_data_t sfo_data;
	pfd_data_t pfd_data;

	char path_file[120], acc_char[60];

    uint32_t magicSFO = 0x00505346;
    uint64_t magicPFD = 0x0000000050464442;

	sprintf(path_file, "%s/PARAM.SFO", path);

	if(cellFsStat(path_file, &stat) == SUCCEEDED)
	{
        // Getting last logged userID
		if(xreg_data(LOGINUSERID, AUTOSIGN, READ, 0, 0))
            return 1;

        // Getting accountID from current logged user
        sprintf(acc_char, SETTING_ACCOUNTID, userID);
        if(xreg_data(acc_char, ACCOUNTID, READ, 0, 0))
            return 2;	
				
		// Read and retrieve PARAM.SFO data		
        uint8_t *temp_data_sfo = malloc(stat.st_size);
        if(!temp_data_sfo)
			return -1;

        memcpy(&sfo_data.size, (void *)&stat.st_size, 16);
        strcpy(sfo_data.file_path, path_file);          
		if(readfile(path_file, temp_data_sfo, stat.st_size))
		{
			free(temp_data_sfo);
			return 3;
		}

		// PARAM.SFO not valid
		if(memcmp(temp_data_sfo, &magicSFO, 4)) 
		{
			free(temp_data_sfo);
			return 4;
		}

		// Disable copy protection of savedata
		*(temp_data_sfo + COPY_PROTECTION_OFFSET) = 0;
		
		// Patching with current userID
		userID = SWAP32(userID);
		memcpy(temp_data_sfo + USER_ID_1_OFFSET, &userID, 4);
		memcpy(temp_data_sfo + USER_ID_2_OFFSET, &userID, 4);

		// Patching with current accountID
		memcpy(temp_data_sfo + 0x140, account_id, 0x10);
		memcpy(temp_data_sfo + 0x588, account_id, 0x10);		

		// Patching with PSID
		memcpy(temp_data_sfo + 0x574, (void *)PS3MAPI_PSID, 0x10);

		// Creating CID key
		memcpy(console_id_key, (void *)PS3MAPI_IDPS_1, 0x10);

		// Generate PARAM.SFO hashes
		sha1_hmac(savegame_param_sfo_key, 0x14, temp_data_sfo, stat.st_size, sfo_data.savegame_param_sfo_hash);
		sha1_hmac(console_id_key, 0x10, temp_data_sfo, stat.st_size, sfo_data.console_id_hash);
		sha1_hmac(disc_hash_key, 0x10, temp_data_sfo, stat.st_size, sfo_data.disc_hash_hash);
		sha1_hmac(authentication_id, 8, temp_data_sfo, stat.st_size, sfo_data.authentication_id_hash);

		sprintf(path_file, "%s/PARAM.PFD", path);		

		if(cellFsStat(path_file, &stat) == SUCCEEDED)
		{		
			// Read and retrieve PARAM.PFD data		
            uint8_t *temp_data_pfd = malloc(stat.st_size);
            if(!temp_data_pfd)
			{
				free(temp_data_sfo);
				return -1;
			}

            memcpy(&pfd_data.size, (void *)&stat.st_size, 16);
            strcpy(pfd_data.file_path, path_file);
			if(readfile(path_file, temp_data_pfd, stat.st_size))
			{
				free(temp_data_sfo);
				free(temp_data_pfd);
				return 5;
			}

			// PARAM.PFD not valid
			if(memcmp(temp_data_pfd, &magicPFD, 8))
			{
				free(temp_data_sfo);
				free(temp_data_pfd);
				return 6;
			}

			// Decrypt PARAM.PFD header
			memcpy(pfd_data.vector, temp_data_pfd + 0x10, 0x10);
			aescbccfb_dec(temp_data_pfd + 0x20, temp_data_pfd + 0x20, 0x40, syscon_manager_key, 128, pfd_data.vector);

			// Generate real hash key
			uint64_t param_version;		
			memcpy(&param_version, temp_data_pfd + 8, 8);

			if(param_version == 3)
				memcpy(pfd_data.real_hashkey, temp_data_pfd + 0x48, 0x14);
			else
				sha1_hmac(keygen_key, 0x14, temp_data_pfd + 0x48, 0x14, pfd_data.real_hashkey);		

            uint64_t reserved = 0;
            memcpy(&reserved, temp_data_pfd + HEADER_SIZE, 8);
            prot_table = HEADER_SIZE + 0x18 + reserved * 8;		

			// Copy PARAM.SFO hashes
			memcpy(temp_data_pfd + SAVEGAME_HASH_OFFSET, sfo_data.savegame_param_sfo_hash, 0x14);
			memcpy(temp_data_pfd + CONSOLE_ID_HASH_OFFSET, sfo_data.console_id_hash, 0x14);
			memcpy(temp_data_pfd + DISC_HASH_OFFSET, sfo_data.disc_hash_hash, 0x14);
			memcpy(temp_data_pfd + AUTHENTICATION_ID_HASH_OFFSET, sfo_data.authentication_id_hash, 0x14);			

			// Generate hash entry for PARAM.SFO            
			generate_entry_hash(pfd_data.real_hashkey, temp_data_pfd);

			// Generate default hash
			sha1_hmac(pfd_data.real_hashkey, 0x14, NULL, 0, pfd_data.default_hash);

			// Generate bottom hash
			sha1_hmac(pfd_data.real_hashkey, 0x14, temp_data_pfd + 0x7B60, 0x474, temp_data_pfd + 0x20);

			// Generate top hash
			sha1_hmac(pfd_data.real_hashkey, 0x14, temp_data_pfd + 0x60, 0x1E0, temp_data_pfd + 0x34);

			// Encrypt header
			aescbccfb_enc(temp_data_pfd + 0x20, temp_data_pfd + 0x20, 0x40, syscon_manager_key, 128, pfd_data.vector);

			// Write new files
			if(savefile(sfo_data.file_path, temp_data_sfo, sfo_data.size))
			{
				free(temp_data_sfo);	
				free(temp_data_pfd);
				return 7;
			}

			if(savefile(pfd_data.file_path, temp_data_pfd, pfd_data.size))
			{
				free(temp_data_sfo);	
				free(temp_data_pfd);
				return 7;
			}

			free(temp_data_sfo);	
			free(temp_data_pfd);	

			return SUCCEEDED;		
		}		

		free(temp_data_sfo);				
	}
	
	return -1;
} 
