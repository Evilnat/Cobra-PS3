#include <lv2/lv2.h>
#include <lv2/libc.h>
#include <lv2/ctrl.h>
#include <lv2/memory.h>
#include <lv2/storage.h>
#include <lv2/io.h>
#include <lv2/thread.h>
#include <lv2/synchronization.h>
#include <lv2/object.h>
#include <lv2/time.h>
#include <lv2/security.h>
#include <lv2/patch.h>
#include <lv2/syscall.h>
#include <lv2/error.h>
#include <lv2/symbols.h>
#include <lv1/lv1.h>
#include <lv1/lv1call.h>
#include <lv1/stor.h>
#include <lv1/patch.h>
#include "common.h"
#include "storage_ext.h"
#include "scsi.h"
#include "config.h"
#include "crypto.h"
#include "mappath.h"
#include "modulespatch.h"
#include "fan_control.h"
#include "homebrew_blocker.h"

//#define ps2emu_entry1_bc 0x165B44
//#define ps2emu_entry2_bc 0x165CC0

//#define ps2emu_entry1_semibc 0x165b40
//#define ps2emu_entry2_semibc 0x165CC0

#define _4KB_						0x1000
#define _64KB_						0x10000

#define LC_SECTORS					32
#define UNDEFINED   				-1

#define READ_BUF_SIZE				(256 * 1024)
#define READ_BUF_SIZE_SECTORS_PSX	(128)
#define CD_CACHE_SIZE				(64)

#ifdef DEBUG
	#define THREAD_NAME	"DiscemuDispatchThread"
#else
	#define THREAD_NAME	""
#endif

#define PS2EMU_STAGE2_FILE	"/dev_hdd0/vm/pm0"
#define PS2EMU_CONFIG_FILE	"/dev_hdd0/tmp/cfg.bin"
#define cfg_suffix 			"CONFIG"

#define MIN(a, b)	((a) <= (b) ? (a) : (b))
#define ABS(a)		(((a) < 0) ? -(a) : (a))

typedef struct _ReadIsoCmd
{
	uint64_t offset;
	uint64_t size;
	uint8_t *buf;
	process_t process;
} ReadIsoCmd;

typedef struct _ReadDiscCmd
{
	uint64_t start_sector;
	uint32_t sector_count;
	uint8_t *buf;
} ReadDiscCmd;

typedef struct _ReadCdIso2352Cmd
{
	uint32_t start_sector;
	uint32_t sector_count;
	uint8_t *buf;
	process_t process;
} ReadCdIso2352Cmd;

typedef struct _FakeStorageEventCmd
{
	uint64_t event;
	uint64_t param;
	uint64_t device;
} FakeStorageEventCmd;

typedef struct _DiscFile
{
	char **files;
	int  count;
	int activefile;
	uint64_t *sizes;
	uint64_t totalsize;
	uint64_t cached_offset;
	void *cached_sector;
} DiscFile;

typedef struct _DiscFileCD
{
	char *file;
	uint32_t num_sectors;
	ScsiTrackDescriptor *tracks;
	int numtracks;
	uint8_t *cache;
	uint32_t cached_sector;
} DiscFileCD;

typedef struct _DiscFileProxy
{
	uint64_t size;
	ScsiTrackDescriptor *tracks;
	int numtracks;
	uint32_t read_size;
	uint64_t cached_offset;
	void *cached_sector;
} DiscFileProxy;

/*
uint8_t encrypted_image_keys[16] =
{
	0x11, 0x0C, 0xE4, 0x15, 0xDD, 0x39, 0x76, 0x8C,
	0x90, 0xB6, 0x40, 0xF5, 0xCB, 0x33, 0xC6, 0xB6
};
*/

static mutex_t mutex;
static event_port_t command_port, result_port;
static event_queue_t command_queue, result_queue;

static event_port_t proxy_command_port;
static event_queue_t proxy_result_queue;

static uint8_t  lsd_header; // 0(lsd) / 4(sbi)
static uint8_t  lsd_struct; // 15(lsd) / 14(sbi)
static uint16_t lsd[LC_SECTORS];

static int subqfd = UNDEFINED;
static int discfd = UNDEFINED;

static int disc_emulation;
static int total_emulation;
static int skip_emu_check = 0;
static volatile int loop = 0;
static DiscFile *discfile;
static DiscFileCD *discfile_cd;
static DiscFileProxy *discfile_proxy;

static int emu_ps3_rec = 0; // Support for burned PS3 DVD/BD Discs by deank
static int disc_being_mounted = 0;
static int could_not_read_disc;
static int hdd0_mounted = 0;
static int usb000_mounted = 0;

// 3k3y/Redump ISOs decryption on-the-fly (By Evilnat)
uint8_t key_d1[0x10] = { 0x38, 0x0B, 0xCF, 0x0B, 0x53, 0x45, 0x5B, 0x3C, 0x78, 0x17, 0xAB, 0x4F, 0xA3, 0xBA, 0x90, 0xED };
uint8_t iv_d1[0x10] =  { 0x69, 0x47, 0x47, 0x72, 0xAF, 0x6F, 0xDA, 0xB3, 0x42, 0x74, 0x3A, 0xEF, 0xAA, 0x18, 0x62, 0x87 };
uint8_t disc_key[0x10] = { 0 };
char key_path[256];
char dkey_path[256];
int encrypted_iso = 0;
int keyfd;
static uint32_t sectors;
static unsigned char iv_[0x10];
DiscRegionInfo discRegionInfo[200];

int ps2emu_type;
//uint8_t color_disc = 0;

static int video_mode = -2;
static uint32_t base_offset = 0;
static uint32_t cd_sector_size = 2352; // -- AV: cd sector size
uint8_t forced_video_mode = 0; // 0 = Detect, 1 = NTSC, 2 = PAL

//static char *encrypted_image;
//static int encrypted_image_fd = -1;
//static uint64_t encrypted_image_nonce;

unsigned int real_disctype; 		/* Real disc in the drive */
unsigned int effective_disctype; 	/* The type of disc we want it to be, and the one faked in storage event. */
unsigned int fake_disctype; 		/* If no zero, get device type command will fake disc type to this. */

int emu_read_bdvd1(void *object, void *buf, uint64_t size, uint64_t offset);
int emu_storage_read(device_handle_t device_handle, uint64_t unk, uint64_t start_sector, uint32_t sector_count, void *buf, uint32_t *nread, uint64_t unk2);

LV2_EXPORT int storage_internal_get_device_object(void *object, device_handle_t handle, void **dev_object);

// Convert 4 bytes in big-endian format, to an unsigned integer (By red_meryl)
static unsigned int char_arr_BE_to_uint(unsigned char *arr)
{
	return arr[3] + 256 * (arr[2] + 256 * (arr[1] + (256 * arr[0])));
}

// Get key for ISO decryption on-the-fly (By Evilnat)
static int get_key(char *path) 
{
	char partial_path[256];
	uint64_t keynread;

	char *ps3iso_folder = strstr(path, "PS3ISO");
	if(ps3iso_folder == NULL)
		ps3iso_folder = strstr(path, "ps3iso");
	if(ps3iso_folder == NULL)
		return 1;		

	char *ext = strstr(path, ".ISO");	
	if(ext == NULL)
		ext = strstr(path, ".iso");
	if(ext == NULL)
		ext = strstr(path, ".ntfs["); // By aldostools

	if(ext)
	{
		strcpy(partial_path, path);
		int path_len = strlen(partial_path);
		int ext_len = strlen(ext);	
		partial_path[path_len - ext_len] = '\0';
		sprintf(key_path, "%s.key", partial_path);		
		sprintf(dkey_path, "%s.dkey", partial_path);	
	}	

	memset(ntfs_iso_path, 0, sizeof(ntfs_iso_path));

	if(!cellFsOpen(dkey_path, CELL_FS_O_RDONLY, &keyfd, 0666, NULL, 0))
	{
		//DPRINTF("[ISO] Found DKEY\n");
		char dkey[0x20];
		cellFsRead(keyfd, dkey, 0x20, &keynread);

		for (int i = 0; i < 0x10; i++)
		{
			char byte[3];
			strncpy(byte, &dkey[i * 2], 2);
			byte[2] = 0;
			disc_key[i] = strtoull(byte, NULL, 16);
		}

		return 0;
	}
	else if(!cellFsOpen(key_path, CELL_FS_O_RDONLY, &keyfd, 0666, NULL, 0))
	{			
		//DPRINTF("[ISO] Found DISCKEY\n");
		cellFsRead(keyfd, disc_key, 0x10, &keynread);		
		cellFsClose(keyfd);
		return 0;
	}
	else
		memset(disc_key, 0, 0x10);

	return 1;
}

// From https://www.psdevwiki.com/
static void getIV(unsigned char* iv, int sectorNumber)
{
	int num = sectorNumber;

	for (int j = 0; j < 16; j++)
	{
		iv[16 - j - 1] = (int)(num & 0xFF);
		num >>= 8;
	}
}

// Get sectors and region adresses (By Evilnat)
static int prepare_region_info(DiscRegionInfo *info, uint8_t *buffer)
{
	memcpy(&sectors, &buffer[0], 4);

	if(sectors)
	{
		sectors = (char_arr_BE_to_uint((unsigned char*)&sectors) * 2) - 1;

		for(int i = 0; i < sectors; i++)
		{
			info[i].isEncrypted = (i % 2 == 1);
			info[i].region_first_sector = (i == 0 ? 0 : info[i - 1].region_last_sector + 1);
			info[i].region_last_sector = char_arr_BE_to_uint(buffer + 0x0C + (i * 4));
		}

		return 0;
	}

	return 1;
}

// 3k3y/Redump ISOs decryption on-the-fly (By Evilnat)
static void decrypt_iso_data(DiscRegionInfo *info, uint32_t total_sectors, uint64_t offset, uint8_t *buffer, uint64_t size)
{
	int lba = 0;

	for(int i = 0; i < total_sectors; i++)
	{
		if(info[i].isEncrypted)						
		{				
			lba = 0;

			if((offset >= info[i].region_first_sector * 0x800) && (offset < info[i].region_last_sector * 0x800))
			{			
				if(size > 0x800)
				{
					for(int sector_pos = 0; sector_pos < size / 0x800; sector_pos++)
					{
						lba = (offset + sector_pos * 0x800 - (info[i].region_first_sector * 0x800)) / 0x800;
						getIV(iv_, info[i].region_first_sector + lba);
						//DPRINTF("[ISO] Decrypting offset: 0x%lX || Global sector: 0x%lX || lba: 0x%X\n", offset + sector_pos * 0x800, info[i].region_first_sector, lba);
						aescbccfb_dec(buffer + sector_pos * 0x800, buffer + sector_pos * 0x800, 0x800, disc_key, 128, iv_);
					}
				}
				else
				{
					lba = (offset - (info[i].region_first_sector * 0x800)) / 0x800;	
					getIV(iv_, info[i].region_first_sector + lba);
					//DPRINTF("[ISO] Decrypting offset: 0x%lX || Global sector: 0x%lX || lba: 0x%X\n", offset, info[i].region_first_sector, lba);
					aescbccfb_dec(buffer, buffer, 0x800, disc_key, 128, iv_);
				}
			}
		}
	}
}

static INLINE void get_next_read(int64_t discoffset, uint64_t bufsize, uint64_t *fileoffset, uint64_t *readsize, int *file)
{
	uint64_t base = 0;
	*file = UNDEFINED;
	*readsize = bufsize;
	*fileoffset = 0;

	for (int i = 0; i < discfile->count; i++)
	{
		uint64_t last = base + discfile->sizes[i];

		if (discoffset >= base && discoffset < last)
		{
			uint64_t maxfileread = last - discoffset;

			if (bufsize > maxfileread)
				*readsize = maxfileread;
			else
				*readsize = bufsize;

			*file = i;
			*fileoffset = discoffset-base;
			return;
		}

		base += discfile->sizes[i];
	}

	//DPRINTF("Offset or size out of range  %lx %lx!!!!!!!!\n", discoffset, bufsize);
}

static INLINE int process_read_iso_cmd(ReadIsoCmd *cmd)
{
	void *readbuf;
	uint8_t *ptr;
	uint64_t remaining, bufsize, offset;
	int ret, iskernel, activefile, doseek;
	int cache = 0;

	ret = SUCCEEDED;
	iskernel = (((uint64_t)cmd->buf) >> 63);
	offset = cmd->offset;
	remaining = cmd->size;

	//DPRINTF("Read %lx %lx\n", cmd->offset, cmd->size);

	if (disc_emulation == EMU_PS3 && remaining == 2048)
		cache = 1;

	if (cache)
	{
		if (discfile->cached_sector && discfile->cached_offset == offset)
		{
			if (iskernel)
				memcpy(cmd->buf, discfile->cached_sector, 2048);
			else
				copy_to_process(cmd->process, discfile->cached_sector, cmd->buf, 2048);

			return SUCCEEDED;
		}
	}

	if (discfile->cached_sector)
	{
		xfree(discfile->cached_sector);
		discfile->cached_sector = NULL;
	}

	if (iskernel)
	{
		readbuf = cmd->buf;
		bufsize = remaining;
	}
	else
	{
		bufsize = (remaining > READ_BUF_SIZE) ? READ_BUF_SIZE : remaining;
		ret = page_allocate_auto(NULL, bufsize, 0x2F, &readbuf);
		if (ret)
			return ret;
	}

	ptr = cmd->buf;
	activefile = discfile->activefile;
	doseek = 1;

	while (remaining > 0)
	{
		uint64_t maxreadsize, filepos, readsize, v;
		int file;

		maxreadsize = MIN(remaining, bufsize);
		get_next_read(offset, maxreadsize, &filepos, &readsize, &file);

		if (file != UNDEFINED)
		{
			if (discfd == UNDEFINED || file != activefile)
			{
				if (discfd != UNDEFINED)
					cellFsClose(discfd);

				//DPRINTF("Changed to part file %d\n", file);

				ret = cellFsOpen(discfile->files[file], CELL_FS_O_RDONLY, &discfd, 0, NULL, 0);
				if (ret)
				{
					discfd = UNDEFINED;
					break;
				}				

				// 3k3y/Redump ISOs decryption on-the-fly (By Evilnat)
				if(filepos == 0)
				{
					CellFsStat stat;
					if(cellFsStat(discfile->files[file], &stat) == CELL_FS_SUCCEEDED)
					{
						unsigned char buffer[0x1000];
						cellFsRead(discfd, buffer, 0x1000, &v);

						if(memcmp(&buffer[REDUMP_WATERMARK_OFFSET], "Decrypted", 9))
						{
							if(!memcmp(&buffer[REDUMP_WATERMARK_OFFSET], "Encrypted", 9) || disc_key[0] != 0x00)
							{
								//DPRINTF("[ISO] Encrypted ISO detected\n");	 

								if(disc_key[0] == 0x00)
								{
									memcpy(disc_key, &buffer[REDUMP_KEY_OFFSET], 0x10);
									if(disc_key[0] == 0x00)
									{
										//DPRINTF("[ISO] Key not found!\n");	
									}
									else 
									{
										aescbccfb_enc(disc_key, disc_key, 0x10, key_d1, 128, iv_d1);
										//DPRINTF("[ISO] Created DISCKEY\n");	
									}
								}		

								if(!prepare_region_info(discRegionInfo, buffer))
									encrypted_iso = 1;
								else
									encrypted_iso = 0;
							}
							else 
								encrypted_iso = 0;
						}
						else 
							encrypted_iso = 0;
					}
					else 
						encrypted_iso = 0;
				}

				activefile = file;
				doseek = 1;
			}

			if (doseek)
			{
				ret = cellFsLseek(discfd, base_offset + filepos, SEEK_SET, &v);
				if (ret)
					break;

				doseek = 0;
			}

			ret = cellFsRead(discfd, readbuf, readsize, &v);
			if (ret)
				break;
			
			// 3k3y/Redump ISOs decryption on-the-fly (By Evilnat)
			if(encrypted_iso && disc_key[0] != 0x00 && filepos != 0)
				decrypt_iso_data(discRegionInfo, sectors, filepos, (uint8_t *)readbuf, cmd->size);

			if (v != readsize)
			{
				ret = -1;
				break;
			}
		}
		else
		{
			// don't know why, but in some video blu ray iso i've seen a read request over the size reported. Let's just dummy data.
			memset(readbuf, 0, readsize);
			ret = 0;
		}

		if (!iskernel)
		{
			ret = copy_to_process(cmd->process, readbuf, ptr, readsize);
			if (ret)
				break;
		}

		ptr += readsize;
		offset += readsize;
		remaining -= readsize;

		if (iskernel)
			readbuf = ptr;
	}

	if (ret == 0)
	{
		if (cache)
		{
			discfile->cached_sector = xalloc(2048);

			if (iskernel)
				memcpy(discfile->cached_sector, cmd->buf, 2048);
			else
				copy_from_process(cmd->process, cmd->buf, discfile->cached_sector, 2048);

			discfile->cached_offset = cmd->offset;
		}
	}

	//DPRINTF("WARNING: Error %x\n", ret);

	discfile->activefile = activefile;

	if (!iskernel)
		page_free(NULL, readbuf, 0x2F);

	return ret;
}

static INLINE int process_read_cd_iso2048_cmd(ReadIsoCmd *cmd)
{
	uint8_t *readbuf, *ptr;
	uint64_t sector, v;
	uint32_t remaining, bufsize;
	int iskernel, ret;

	sector = cmd->offset / 2048;
	remaining = cmd->size / 2048;
	iskernel = (((uint64_t)cmd->buf) >> 63);

	if (discfd == UNDEFINED)
	{
		ret = cellFsOpen(discfile_cd->file, CELL_FS_O_RDONLY, &discfd, 0, NULL, 0);
		if (ret)
			return ret;
	}

	bufsize = MIN(remaining, READ_BUF_SIZE_SECTORS_PSX);
	ret = page_allocate_auto(NULL, bufsize * cd_sector_size, 0x2F, (void **)&readbuf);
	if (ret)
		return ret;

	ptr = cmd->buf;
	ret = cellFsLseek(discfd, base_offset + (sector * cd_sector_size), SEEK_SET, &v);

	if (ret)
		remaining = 0;

	while (remaining > 0)
	{
		uint32_t readsize = MIN(remaining, bufsize);

		if (sector >= discfile_cd->num_sectors)
			v = 0;
		else
		{
			ret = cellFsRead(discfd, readbuf, readsize * cd_sector_size, &v);
			if (ret)
				break;
		}

		if (v < (readsize * cd_sector_size))
			memset(readbuf + v, 0, (readsize * cd_sector_size) - v);

		for (int i = 0; i < readsize; i++)
		{
			uint8_t *s = readbuf + (i * cd_sector_size) + 24;

			if (iskernel)
				memcpy(ptr, s, 2048);
			else
				copy_to_process(cmd->process, s, ptr, 2048);

			ptr += 2048;
		}

		remaining -= readsize;
		sector += readsize;
	}

	page_free(NULL, readbuf, 0x2F);
	return ret;
}

static INLINE int process_read_cd_iso2352_cmd(ReadCdIso2352Cmd *cmd)
{
	void *readbuf;
	uint8_t *buf;
	uint8_t *ptr;
	uint64_t sector, v;
	uint32_t remaining, bufsize;
	int iskernel, ret, cache;

	ret = 0;
	sector = cmd->start_sector;
	remaining = cmd->sector_count;
	buf = cmd->buf;
	iskernel = (((uint64_t)buf) >> 63);

	if (discfd == UNDEFINED)
	{
		ret = cellFsOpen(discfile_cd->file, CELL_FS_O_RDONLY, &discfd, 0, NULL, 0);
		if (ret)
			return ret;
	}

	if (remaining <= CD_CACHE_SIZE)
	{
		int dif = (int)discfile_cd->cached_sector-sector;

		if (ABS(dif) < CD_CACHE_SIZE)
		{
			uint8_t *copy_ptr = NULL;
			uint32_t copy_offset = 0;
			uint32_t copy_size = 0;

			if (dif > 0)
			{
				if (dif < remaining)
				{
					copy_ptr = discfile_cd->cache;
					copy_offset = dif;
					copy_size = remaining-dif;
				}
			}
			else
			{
				copy_ptr = discfile_cd->cache + ((-dif) * cd_sector_size);
				copy_size = MIN(remaining, CD_CACHE_SIZE + dif);
			}

			if (copy_ptr)
			{
				if (iskernel)
					memcpy(buf + (copy_offset * cd_sector_size), copy_ptr, copy_size * cd_sector_size);
				else
					copy_to_process(cmd->process, copy_ptr, buf + (copy_offset * cd_sector_size), copy_size * cd_sector_size);

				if (remaining == copy_size)
					return SUCCEEDED;

				remaining -= copy_size;

				if (dif <= 0)
				{
					uint32_t newsector = discfile_cd->cached_sector + CD_CACHE_SIZE;
					buf += ((newsector - sector) * cd_sector_size);
					sector = newsector;
				}
			}
		}

		cache = 1;
	}

	if (cache)
		readbuf = discfile_cd->cache;
	else
	{
		if (iskernel)
		{
			bufsize = remaining;
			readbuf = buf;
		}
		else
		{
			bufsize = MIN(remaining, READ_BUF_SIZE_SECTORS_PSX);
			ret = page_allocate_auto(NULL, bufsize * cd_sector_size, 0x2F, (void **)&readbuf);
			if (ret)
				return ret;
		}
	}

	ptr = buf;
	ret = cellFsLseek(discfd, base_offset + (sector * cd_sector_size), SEEK_SET, &v);
	if (ret) 
		remaining = 0;

	while (remaining > 0)
	{
		uint32_t readsize;

		if (cache)
			readsize = CD_CACHE_SIZE;
		else
			readsize = MIN(remaining, bufsize);

		if (sector >= discfile_cd->num_sectors)
			v = 0;
		else
		{
			ret = cellFsRead(discfd, readbuf, readsize * cd_sector_size, &v);
			if (ret)
				break;
		}

		if (v < (readsize * cd_sector_size))
			memset(readbuf + v, 0, (readsize * cd_sector_size) - v);

		if (!cache)
		{
			if (iskernel)
			{
				ptr += readsize * cd_sector_size;
				readbuf = ptr;
			}
			else
			{
				copy_to_process(cmd->process, readbuf, ptr, readsize * cd_sector_size);
				ptr += readsize * cd_sector_size;
			}
		}
		else
		{
			if (iskernel)
				memcpy(ptr, readbuf, remaining * cd_sector_size);
			else
				copy_to_process(cmd->process, readbuf, ptr, remaining * cd_sector_size);

			discfile_cd->cached_sector = sector;
			return SUCCEEDED;
		}

		remaining -= readsize;
		sector += readsize;
	}

	if (!iskernel)
		page_free(NULL, readbuf, 0x2F);

	return ret;
}

int process_read_disc_cmd(ReadDiscCmd *cmd)
{
	lv1_stor_wrapper_var var;
	u64 dma_lpar;
	void *dma;
	int ret;

	// reasons to use lv1 calls here over lv2 storage functions
	// 1: this function may be called when lv2 storage functions haven't yet received the bdvd ready event, and thus, they don't work.
	// 2: this will read the real disc even with iso mounted, it may be useful in the future.

	ret = page_allocate_auto(NULL, 4096, 0x2F, &dma);
	memset(dma, 0x5B, 4096);

	if (ret == 0)
	{
		ret = kernel_ea_to_lpar_addr(dma, &dma_lpar);
		if (ret == 0)
		{
			suspend_intr();
			uint64_t state = spin_lock_irqsave();

			ret =  lv1_stor_wrapper_open(LV1_BDVD_DEV_ID, dma, dma_lpar, 12, &var);
			if (ret == 0)
			{
				ret = lv1_stor_wrapper_read(&var, 0, cmd->start_sector, cmd->sector_count, 0x2, cmd->buf);
				lv1_stor_wrapper_close(&var);
			}

			spin_unlock_irqrestore(state);
			resume_intr();
		}

		page_free(NULL, dma, 0x2F);
	}

	return ret;
}

int process_proxy_cmd(uint64_t command, process_t process, uint8_t *buf, uint64_t offset, uint32_t size)
{
	uint32_t remaining;
	int iskernel, do_copy;
	int ret;
	event_t event;

	iskernel = (((uint64_t)buf) >> 63);
	remaining = size;

	if(!vsh_process)
		return -666;

	do_copy = (iskernel || process != vsh_process);

	if (!do_copy)
	{
		//DPRINTF("Native VSH read\n");

		ret = event_port_send(proxy_command_port, command, offset, (((uint64_t)buf) << 32ULL) | remaining);
		if (ret)
		{
			//DPRINTF("event_port send failed: %x\n", ret);
			return ret;
		}

		ret = event_queue_receive(proxy_result_queue, &event, 0);
		if (ret)
		{
			//DPRINTF("event_queue_receive failed: %x\n", ret);
			return ret;
		}

		ret = (int)event.data1;
	}
	else
	{
		uint64_t read_size;
		void *kbuf, *vbuf;
		uint8_t *obuf;
		int cache = 0;

		obuf = buf;

		if (disc_emulation == EMU_PS3 && remaining == 2048)
			cache = 1;

		if (cache)
		{
			if (discfile_proxy->cached_sector && discfile_proxy->cached_offset == offset)
			{
				if (iskernel)
					memcpy(buf, discfile_proxy->cached_sector, 2048);
				else
					copy_to_process(process, discfile_proxy->cached_sector, buf, 2048);

				return SUCCEEDED;
			}
		}

		if (discfile_proxy->cached_sector)
		{
			xfree(discfile_proxy->cached_sector);
			discfile_proxy->cached_sector = NULL;
		}

		read_size = (remaining <= discfile_proxy->read_size) ? remaining : discfile_proxy->read_size;

		ret = page_allocate_auto(vsh_process, read_size, 0x2F, &kbuf);
		if (ret)
		{
			//DPRINTF("page_allocate failed: %x\n", ret);
			return ret;
		}

		ret = page_export_to_proc(vsh_process, kbuf, 0x40000, &vbuf);
		if (ret)
		{
			//DPRINTF("page_export_to_proc failed: %x\n", ret);

			page_free(vsh_process, kbuf, 0x2F);
			return ret;
		}

		while (remaining > 0)
		{
			uint64_t this_read_size;

			this_read_size = (remaining <= read_size) ? remaining : read_size;
			ret = event_port_send(proxy_command_port, command, offset, (((uint64_t)vbuf) << 32ULL) | this_read_size);
			if (ret)
				break;

			ret = event_queue_receive(proxy_result_queue, &event, 0);
			if (ret)
				break;

			ret = (int)event.data1;
			if (ret)
				break;

			if (iskernel)
				memcpy(buf, kbuf, this_read_size);
			else
				copy_to_process(process, kbuf, buf, this_read_size);

			// 3k3y/Redump ISOs decryption on-the-fly (By Evilnat)
			if(offset == 0)
			{
				if(!get_key(ntfs_iso_path))
					prepare_region_info(discRegionInfo, buf);
				else
					memset(disc_key, 0, 0x10);
			}

			if(offset != 0 && disc_key[0] != 0x00)
				decrypt_iso_data(discRegionInfo, sectors, offset, (uint8_t *)buf, size);

			buf += this_read_size;
			offset += this_read_size;
			remaining -= this_read_size;
		}

		page_unexport_from_proc(vsh_process, vbuf);
		page_free(vsh_process, kbuf, 0x2F);

		if (cache)
		{
			discfile_proxy->cached_sector = xalloc(2048);

			if (iskernel)
				memcpy(discfile_proxy->cached_sector, obuf, 2048);
			else
				copy_from_process(process, obuf, discfile_proxy->cached_sector, 2048);

			discfile_proxy->cached_offset = offset - 2048;
		}
	}

	if (ret)
	{
		DPRINTF("proxy read failed: %x\n", ret);
	}

	return ret;
}

static INLINE int process_read_iso_cmd_proxy(ReadIsoCmd *cmd)
{
	return process_proxy_cmd(CMD_READ_ISO, cmd->process, cmd->buf, cmd->offset, cmd->size);
}

static INLINE int process_read_cd_iso2352_cmd_proxy(ReadCdIso2352Cmd *cmd)
{
	return process_proxy_cmd(CMD_READ_CD_ISO_2352, cmd->process, cmd->buf, cmd->start_sector * cd_sector_size, cmd->sector_count * cd_sector_size);
}

#ifdef ENCRYPT_FUNCTIONS
	// Base function without wrappers is "__"
	#define device_event_func	__device_event
#else
	// Base function without wrappers is "_"
	#define device_event_func	_device_event
#endif

int device_event_func(event_port_t port, uint64_t event, uint64_t param, uint64_t device);

int process_fake_storage_event_cmd(FakeStorageEventCmd *cmd)
{
	uint64_t *ptr = (uint64_t *)(*(uint64_t *)MKA(TOC + device_event_rtoc_entry_1));
	ptr = (uint64_t *)ptr[0];

	event_port_t port = (event_port_t)ptr[0x40 / 8];

	loop = 1;
	int ret = device_event_func(port, cmd->event, cmd->param, cmd->device);
	loop = 0;

	return ret;
}

////////////// READ LSD SECTORS ////////////////////
static void read_libcrypt_sectors(const char *file)
{
	int ret = cellFsOpen(file, CELL_FS_O_RDONLY, &subqfd, 0, NULL, 0);

	if(ret)
		return;

	if(strstr(file, ".sbi"))
		lsd_struct = 14, lsd_header = 4;
	else
		lsd_struct = 15, lsd_header = 0;

	// Read list of LibCrypt sectors in MSF
	size_t r; MSF msf;

	for(u8 n = 0; n < LC_SECTORS; n++)
	{
		cellFsLseek(subqfd, lsd_header + (n * lsd_struct), SEEK_SET, &r);
		ret = cellFsRead(subqfd, &msf, sizeof(MSF), &r);

		if(ret)
			break;

		lsd[n] = msf_bcd_to_lba(msf);
	}

	if(ret)
	{
		// Close LSD file
		cellFsClose(subqfd);
		subqfd = UNDEFINED;
	}

	if(lsd_header)
		lsd_header = 8;
	else
		lsd_header = 3;
}

////////////// PROCESS PSX VIDEO MODE //////////////
static void get_cd_sector_size(unsigned int trackscount)
{
	// -- AV: cd sector size
	cd_sector_size = (trackscount & 0xffff00) >> 4; // <- Use: trackscount = num_of_tracks | (cd_sector_size<<4);

	if(cd_sector_size > 2448)
		cd_sector_size = (trackscount & 0xffff00) >> 8;

	if((cd_sector_size != 2352) && 
	   (cd_sector_size != 2048) && 
	   (cd_sector_size != 2328) && 
	   (cd_sector_size != 2336) && 
	   (cd_sector_size != 2340) && 
	   (cd_sector_size != 2368) && 
	   (cd_sector_size != 2448))
			cd_sector_size = 2352;
}

int read_psx_sector(void *dma, void *buf, uint64_t sector)
{
	if (disc_emulation == EMU_OFF)
	{
		device_handle_t handle;
		int ret;

		ret = storage_open(BDVD_DRIVE, 0, &handle, 0);
		if (ret == 0)
		{
			ret = storage_map_io_memory(BDVD_DRIVE, dma, 4096);
			if (ret == 0)
			{
				for (int i = 0; i < 3; i++)
				{
					uint32_t nread;

					skip_emu_check = 1;
					ret = call_hooked_function_7(emu_storage_read, (uint64_t)handle, 0, sector, 1, (uint64_t)dma, (uint64_t)&nread, 0);
					skip_emu_check = 0;

					if (ret == 0)
					{
						memcpy(buf, dma, 2048);
						break;
					}
				}

				storage_unmap_io_memory(BDVD_DRIVE, dma);
			}
			else
			{
				//DPRINTF("retm %x\n", ret);
			}

			storage_close(handle);

		}

		return ret;
	}
	else if (discfd >= 0)
	{
		uint64_t x;

		cellFsLseek(discfd, base_offset + (sector * cd_sector_size) + 0x18, SEEK_SET, &x);
		return cellFsRead(discfd, buf, 2048, &x);
	}
	else if (discfile_proxy)
		return process_proxy_cmd(CMD_READ_ISO, NULL, buf, sector * 2048, 2048);

	return -1;
}

uint32_t find_file_sector(uint8_t *buf, char *file)
{
	uint8_t *p =  (uint8_t *)buf;
	int len = strlen(file);

	while (((p + p[0]) < (buf + 2048)) && (p[0] != 0))
	{
		if (p[0x20] == len && strncasecmp((char *)p + 0x21, file, len) == 0)
			return *(uint32_t *)&p[6];

		p += p[0];
	}

	DPRINTF("%s not found\n", file);

	return SUCCEEDED;
}

int process_get_psx_video_mode(void)
{
	int ret = UNDEFINED;

	if (effective_disctype == DEVICE_TYPE_PSX_CD)
	{
		char *bbuf, *dma;

		bbuf = malloc(_4KB_);

		if(!bbuf)
			return ret;

		page_allocate_auto(NULL, _4KB_, 0x2F, (void **)&dma);

		if(!dma)
		{
			free(bbuf);
			return ret;
		}

		if ((read_psx_sector(dma, bbuf, 0x10) == 0) && (read_psx_sector(dma, bbuf + 2048, *(uint32_t *)&bbuf[0x9C + 6]) == SUCCEEDED))
		{
			uint32_t sector = find_file_sector((uint8_t *)bbuf + 2048, "SYSTEM.CNF;1");

			char *buf = malloc(_4KB_);

			if (!buf) ;
			else if ((sector != 0) && (read_psx_sector(dma, buf, sector) == SUCCEEDED))
			{
				char *p = strstr(buf, "cdrom");

				if (!p)
					p = strstr(buf, "CDROM");

				char *exe_path = malloc(140);

				if(!exe_path)
					p = 0;

				if(p)
				{
					p += 5;

					while ((*p != 0) && !isalpha(*p))
						p++;

					if(*p)
					{
						memset(exe_path, 0, 140);

						for(int i = 0; (i < 117) && (*p >= ' ') && (*p != ';'); p++)
						{
							if(*p=='\\' || *p=='/')
							{
								memset(exe_path, 0, i + 1);
								i = 0;
							}
							else
							{
								exe_path[i] = *p;
								i++;
							}
						}

						//DPRINTF("PSX EXE: %s\n", exe_path);

						// detect PAL by title_id
						if ((exe_path[4] == '_') &&
							(exe_path[0] == 'S' || exe_path[0] == 'P') &&
							(exe_path[1] == 'L' || exe_path[1] == 'C' ||	// SLUS, SCUS, SLPM, SLPS, SCPS, SLUD, SCUD
							 exe_path[1] == 'A' || exe_path[1] == 'B' ||	// PAPX, PBPX, PCPX
							 exe_path[1] == 'I'))							// SIPS
								ret = (exe_path[2] == 'E');					// SLES, SCES, SCED, SLED

						if(subqfd == UNDEFINED)
						{
							sprintf(buf, "/dev_hdd0/tmp/lsd/%s.lsd", exe_path);
							read_libcrypt_sectors(buf);
						}

						if(subqfd == UNDEFINED)
						{
							sprintf(buf, "/dev_hdd0/tmp/sbi/%s.sbi", exe_path);
							read_libcrypt_sectors(buf);
						}

						// detect PAL by PSX EXE
						if(ret == UNDEFINED)
						{
							strcat(exe_path, ";1");

							sector = find_file_sector((uint8_t *)(bbuf + 2048), exe_path);

							if ((sector != 0) && (read_psx_sector(dma, buf, sector) == SUCCEEDED))
							{
								/*if (strncmp(buf+0x71, "North America", 13) == 0 || strncmp(buf+0x71, "Japan", 5) == 0)
								{
									ret = 0;
								}
								else*/

								if (strncmp(buf + 0x71, "Europe", 6) == SUCCEEDED)								
									ret = 1; // PAL								
								else
									ret = 0; // NTSC
							}
						}
					}
				}

				if(exe_path)
					free(exe_path);
			}

			if(buf)
				free(buf);
		}

		DPRINTF((!ret ? "NTSC\n" : "PAL\n"));

		free(bbuf);
		page_free(NULL, dma, 0x2F);
	}

	forced_video_mode = (ret + 1);

	return ret;
}

void dispatch_thread_entry(uint64_t arg)
{
	int ret;

	while (1)
	{
		event_t event;
		int64_t cmd_result = 0;

		ret = event_queue_receive(command_queue, &event, 0);
		if (ret)
			break;

		switch (event.data1)
		{
			case CMD_READ_ISO:
				if (discfile_proxy)
					cmd_result = process_read_iso_cmd_proxy((ReadIsoCmd *)event.data2);
				else if (discfile_cd)
					cmd_result = process_read_cd_iso2048_cmd((ReadIsoCmd *)event.data2);
				else
					cmd_result = process_read_iso_cmd((ReadIsoCmd *)event.data2);
			break;

			case CMD_READ_DISC:
				cmd_result = process_read_disc_cmd((ReadDiscCmd *)event.data2);
			break;

			case CMD_READ_CD_ISO_2352:
				if (discfile_proxy)
					cmd_result = process_read_cd_iso2352_cmd_proxy((ReadCdIso2352Cmd *)event.data2);
				else
					cmd_result = process_read_cd_iso2352_cmd((ReadCdIso2352Cmd *)event.data2);
			break;

			case CMD_FAKE_STORAGE_EVENT:
				cmd_result = process_fake_storage_event_cmd((FakeStorageEventCmd *)event.data2);
			break;

			case CMD_GET_PSX_VIDEO_MODE:
				cmd_result = process_get_psx_video_mode();
			break;
		}

		event_port_send(result_port, cmd_result, 0, 0);
	}

	//DPRINTF("Exiting dispatch thread %d\n", ret);

	ppu_thread_exit(0);
}

static int read_real_disc_sector (void *buf, uint64_t lba, uint32_t size, int retries)
{
	ReadDiscCmd cmd;
	int ret = -1;

	cmd.buf = buf;
	cmd.start_sector = lba;
	cmd.sector_count = size;

	//DPRINTF("Read sector %lx\n", lba);

	for (int i = 0; i < retries && ret != 0; i++)
	{
		if (0/*!loop*/)
		{
			event_t event;

			event_port_send(command_port, CMD_READ_DISC, (uint64_t)&cmd, 0);
			if (event_queue_receive(result_queue, &event, 0) == 0)
				ret = (int)(int64_t)event.data1;
		}
		else
			ret = process_read_disc_cmd(&cmd);

		if (ret == 0)
		{
			// Even when we cannot really read the disc, we are reported success, do a lame check here:
			if (*(uint32_t *)buf == 0x5B5B5B5B)
				ret = -1;
		}
	}

	return ret;
}

int is_psx(int check_ps2)
{
	uint8_t *buf;
	int result;
	int ret = 0;

	if (page_allocate_auto(NULL, 2048, 0x2F, (void **)&buf) == 0)
	{
		result = read_real_disc_sector(buf, 0x10, 1, 3);

		if (result == 0)
		{
			// Check if it is a burned PS3 disk (deank)
			if(check_ps2 == 3)
			{
				ret = (memcmp(buf + 1, "CD001", 5) == 0 && memcmp(buf + 0x28, "PS3VOLUME", 9) == 0);
				if(!ret)
				{
					result = read_real_disc_sector(buf, 0x01, 1, 3);
					ret = (memcmp(buf, "PlayStation3", 12) == 0);
					if(!ret)
					{
						// AV: check for PS3_GAME folder if disc was burned with a generic tool
						result = read_real_disc_sector(buf, 0x14, 1, 3); // 0xA000
						for(uint8_t i = 0x12; i < 0x40; i++)
						{
							ret = (memcmp(buf + i, "PS3_GAME", 8) == SUCCEEDED);
							if(ret)
								break;
						}
					}
				}

				page_free(NULL, buf, 0x2F);
				return ret;
			}

			// Probably not the best way to say if a disc is psx...
			ret = (memcmp(buf + 1, "CD001", 5) == 0 && memcmp(buf+8, "PLAYSTATION ", 12) == 0);
			if (ret && check_ps2)
			{
				// Check for ps2, we need to read SYSTEM.CNF
				if (read_real_disc_sector(buf, *(uint32_t *)&buf[0x9C + 6], 1, 2) == 0)
				{
					uint8_t *p = buf;

					while (((p + p[0]) < (buf+2048)) && (p[0] != 0))
					{
						if (p[0x20] == 12 && memcmp(p + 0x21, "SYSTEM.CNF;1", 12) == 0)
						{
							if (read_real_disc_sector(buf, *(uint32_t *)&p[6], 1, 2) == 0)
							{
								if (memcmp(buf, "BOOT2", 5) == 0)
								{
									// It is ps2
									ret = 2;
								}
							}

							break;
						}

						p += p[0];
					}
				}
			}
		}
		else
			could_not_read_disc = 1;

		page_free(NULL, buf, 0x2F);
	}

	return ret;
}

void process_disc_insert(uint32_t disctype)
{
	could_not_read_disc = 0;
	real_disctype = disctype;
	effective_disctype = real_disctype;
	fake_disctype = 0;
	emu_ps3_rec = 0;

	DPRINTF("real disc type = %x\n", real_disctype);

	switch (disc_emulation)
	{
		case EMU_PS3:
			if (real_disctype != DEVICE_TYPE_PS3_BD)
				fake_disctype = effective_disctype = DEVICE_TYPE_PS3_BD;
		break;

		case EMU_DVD:
			if (real_disctype != DEVICE_TYPE_DVD)
				fake_disctype = effective_disctype = DEVICE_TYPE_DVD;
		break;

		case EMU_BD:
			// We must fake to BD-R/BD-RE and not to BD-ROM, otherwise the player will/may fail.
			// (maybe beause it attemps to do some AACS shit?)
			if (real_disctype < DEVICE_TYPE_BDMR_SR || real_disctype > DEVICE_TYPE_BDMRE)
				fake_disctype = effective_disctype = DEVICE_TYPE_BDMR_SR;
		break;

		case EMU_PSX:
			if (real_disctype != DEVICE_TYPE_PSX_CD)
				fake_disctype = effective_disctype = DEVICE_TYPE_PSX_CD;
		break;

		case EMU_PS2_CD:
			if (real_disctype != DEVICE_TYPE_PS2_CD)
				fake_disctype = effective_disctype = DEVICE_TYPE_PS2_CD;
		break;

		case EMU_PS2_DVD:
			/*if(color_disc)
				fake_disctype = effective_disctype = DEVICE_TYPE_PS2_CD;
			else*/
				if (real_disctype != DEVICE_TYPE_PS2_DVD)
					fake_disctype = effective_disctype = DEVICE_TYPE_PS2_DVD;	
		break;

		case EMU_OFF:
			if (real_disctype == DEVICE_TYPE_CD)
			{
				int psx_type = is_psx(1);

				if (psx_type == 1)
				{
					// PSX CD-R support
					fake_disctype = effective_disctype = DEVICE_TYPE_PSX_CD;
				}
				else if (psx_type == 2)
				{
					// PS2 CD-R support
					fake_disctype = effective_disctype = DEVICE_TYPE_PS2_CD;
				}
			}
			else if (real_disctype == DEVICE_TYPE_DVD)
			{
				if (is_psx(0))
				{
					/*if(color_disc)
						fake_disctype = effective_disctype = DEVICE_TYPE_PS2_CD;						
					else*/
						fake_disctype = effective_disctype = DEVICE_TYPE_PS2_DVD;
				}
			}
			/*else if (real_disctype == DEVICE_TYPE_PS2_DVD)			
				if(color_disc)
					fake_disctype = effective_disctype = DEVICE_TYPE_PS2_CD;*/

			// PS3 DVD-R/BD-R support
			if(real_disctype && real_disctype != DEVICE_TYPE_PS3_BD && fake_disctype == 0 && is_psx(3))
			{
				fake_disctype = effective_disctype = DEVICE_TYPE_PS3_BD;
				emu_ps3_rec = 1;
			}

		break;
	}

	DPRINTF("effective disc type = %x, fake disc type = %x\n", effective_disctype, fake_disctype);
}

LV2_PATCHED_FUNCTION(int, device_event, (event_port_t event_port, uint64_t event, uint64_t param, uint64_t device))
{
	int lock = !loop;

	DPRINTF("Storage event: %lx  %lx  %lx\n", event, param, device);

	if (device == BDVD_DRIVE)
	{
		disc_being_mounted = (event == 7);

		if (event == 3)
		{
			//DPRINTF("Disc Insert\n");

			if (lock)
				mutex_lock(mutex, 0);

			process_disc_insert(param >> 32);
			param = (uint64_t)(effective_disctype) << 32;

			if (lock)
				mutex_unlock(mutex);
		}
		else if (event == 4)
		{
			if (lock)
				mutex_lock(mutex, 0);

			//DPRINTF("Disc removed.\n");

			if (effective_disctype == DEVICE_TYPE_PSX_CD)
				video_mode = -1;

			real_disctype = 0;
			effective_disctype = 0;
			fake_disctype = 0;

			if (lock)
				mutex_unlock(mutex);
		}
	}

	return event_port_send(event_port, event, param, device);
}

int do_read_iso(void *buf, uint64_t offset, uint64_t size)
{
	ReadIsoCmd cmd;
	event_t event;
	int ret;

	cmd.offset = offset;
	cmd.size = size;
	cmd.buf = buf;
	cmd.process = get_current_process_critical();

	event_port_send(command_port, CMD_READ_ISO, (uint64_t)&cmd, 0);
	ret = event_queue_receive(result_queue, &event, 0);

	if (ret == 0)
		ret = (int)(int64_t)event.data1;

	if (ret)
	{
		DPRINTF("Read failed: %x\n", ret);
	}

	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_8(int, emu_read_bdvd0, (void *object, uint64_t offset, void *buf, uint64_t size, int r7, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t st0, uint64_t st1))
{
	int ret = DO_POSTCALL;

	mutex_lock(mutex, 0);

	if (disc_emulation != EMU_OFF)
	{
		//DPRINTF("Warning: emu_read_bdvd0 called.\n");
		dump_stack_trace2(16);

		if (r7 != 1 || r8 != 0 || r9 != 0 || r10 != 0 || st0 != 0 || st1 != 1)
		{
			//DPRINTF("emu_read_bdvd called with unknown params\n");
			//dump_stack_trace2(16);
			fatal("aborting.\n");
		}

		ret = do_read_iso(buf, offset, size);

	}

	mutex_unlock(mutex);
	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_4(int, emu_read_bdvd1, (void *object, void *buf, uint64_t size, uint64_t offset))
{
	int ret = DO_POSTCALL;

	mutex_lock(mutex, 0);

	if (disc_emulation != EMU_OFF)
		ret = do_read_iso(buf, offset, size);

	mutex_unlock(mutex);

	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_3(int, emu_read_bdvd2, (uint64_t *object, void *buf, int64_t size))
{
	int ret = DO_POSTCALL;

	mutex_lock(mutex, 0);

	if (disc_emulation != EMU_OFF)
	{
		if  (do_read_iso(buf, object[0x98 / 8], size) == 0)
			ret = size;
		else
			ret = -1;
	}

	mutex_unlock(mutex);
	return ret;
}

LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_2(int, post_storage_get_device_info, (uint64_t device_id, device_info_t *device_info))
{
	if (device_id == BDVD_DRIVE)
	{
		mutex_lock(mutex, 0);

		if (effective_disctype && disc_emulation)
		{
			if (discfile_cd)
				device_info->sector_count = discfile_cd->num_sectors;
			else if (discfile_proxy)
				device_info->sector_count = (discfile_proxy->tracks) ? discfile_proxy->size/cd_sector_size : discfile_proxy->size/2048;
			else
				device_info->sector_count = discfile->totalsize / device_info->sector_size;

			//DPRINTF("Faked size to %lx\n", device_info->sector_count);
		}

		mutex_unlock(mutex);
	}

	return SUCCEEDED;
}

static int get_handle_device(int handle, uint64_t *device)
{
	uint64_t *object;
	uint64_t *ptr = (uint64_t *)(*(uint64_t *)MKA(TOC + storage_rtoc_entry_1));
	ptr = (uint64_t *)(ptr[0] + 0x40);

	int ret = storage_internal_get_device_object(ptr, handle, (void **)&object);

	if (ret == 0)
		*device = object[0xA0/8];

	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_7(int, emu_storage_read, (device_handle_t device_handle, uint64_t unk, uint64_t start_sector, uint32_t sector_count, void *buf, uint32_t *nread, uint64_t unk2))
{
	uint64_t device;
	int ret = DO_POSTCALL;

	if (skip_emu_check)
		return ret;

	if (get_handle_device(device_handle, &device) == 0)
	{
		if (device == BDVD_DRIVE)
		{
			mutex_lock(mutex, 0);

			if (disc_emulation != EMU_OFF)
			{
				if (do_read_iso(buf, start_sector * 2048, sector_count * 2048) == 0)
				{
					ret = 0;
					*nread = sector_count;
				}
				else
					ret = -1;
			}

			mutex_unlock(mutex);
		}
	}

	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_7(int, emu_sys_storage_async_read, (sys_device_handle_t device_handle, uint32_t unk, uint64_t start_sector, uint32_t sector_count, void *buf, uint64_t param, uint64_t unk2))
{
	object_handle_t obj_handle;
	uint64_t *sys_storage_object;
	void *table;
	int ret = DO_POSTCALL;

	table = get_current_process_critical()->object_table;

	if (open_shared_kernel_object(table, device_handle, (void **)&sys_storage_object, &obj_handle, SYS_STORAGE_HANDLE_OBJECT, 1) == 0)
	{
		uint64_t device = sys_storage_object[8 / 8];

		if (device == BDVD_DRIVE)
		{
			mutex_lock(mutex, 0);

			if (disc_emulation != EMU_OFF)
			{
				mutex_t storage_mutex;
				event_port_t async_port;

				storage_mutex = (mutex_t)sys_storage_object[0x98 / 8];

				if (unk2)
				{
					DPRINTF("WARNING: unk2 not 0: %lx\n", unk2);
				}

				mutex_lock(storage_mutex, 0);

				async_port = (event_port_t) ((uint64_t *)sys_storage_object[(0x28 + 8) / 8])[0x30 / 8];

				if (do_read_iso(get_secure_user_ptr(buf), start_sector * 2048, sector_count * 2048) == 0)
					event_port_send(async_port, param, 0, 0);
				else
				{
					// Umm oh oh, what to send to port on error?
					// Let's try...
					event_port_send(async_port, param, -1, 0);
				}

				mutex_unlock(storage_mutex);
				ret = 0;
			}

			mutex_unlock(mutex);
		}

		close_kernel_object_handle(table, obj_handle);
	}

	return ret;
}

int process_generic_iso_scsi_cmd(uint8_t *indata, uint64_t inlen, uint8_t *outdata, uint64_t outlen)
{
	memset(outdata, 0, outlen);

	switch (indata[0])
	{
		case SCSI_CMD_GET_EVENT_STATUS_NOTIFICATION:
		{
			ScsiCmdGetEventStatusNotification *cmd = (ScsiCmdGetEventStatusNotification *)indata;

			if (cmd->notification_class_request == 0x10)
			{
				ScsiMediaEventResponse *resp;
				int alloc_size = sizeof(ScsiMediaEventResponse);

				resp = malloc(alloc_size);
				memset(resp, 0, alloc_size);

				resp->event_header.event_length = sizeof(ScsiMediaEventResponse) - sizeof(ScsiEventHeader);
				resp->event_header.nea_rv_nc = 4;
				resp->event_header.supported_event_class = 0xF;
				resp->media_status = 2;

				memcpy(outdata, resp, (outlen <= alloc_size) ? outlen : alloc_size);
				free(resp);
			}
			else
			{
				//DPRINTF("Event status: %02X\n", cmd->notification_class_request);
			}
		}
		break;

		case SCSI_CMD_READ_DISC_INFORMATION:
		{
			ScsiCmdReadDiscInformation *cmd = (ScsiCmdReadDiscInformation *)indata;

			int alloc_size = sizeof(ScsiReadDiscInformationResponse);
			ScsiReadDiscInformationResponse *resp = malloc(alloc_size);

			memset(resp, 0, sizeof(ScsiReadDiscInformationResponse));
			resp->length = sizeof(ScsiReadDiscInformationResponse) - sizeof(resp->length);

			resp->misc = 0x0E;
			resp->first_track = 1;
			resp->num_sessions_lb = 1;
			resp->first_track_lastsession_lb = 1;
			resp->last_track_lastsession_lb = 1;
			resp->misc2 = 0x20;
			resp->last_session_leadin = 0xFFFFFFFF;
			resp->last_session_leadout = 0xFFFFFFFF;
			memcpy(outdata, resp, (outlen <= cmd->alloc_length) ? outlen : cmd->alloc_length);
			free(resp);
		}
		break;

		/*
			default:
			#ifdef DEBUG
				DPRINTF("Command %s outlen=%ld\n", get_scsi_cmd_name(indata[0]), outlen);
			#endif
		*/
	}

	return 1;
}

#define GET_MSF(x) ((x)->rv_msf&2)
#define GET_FORMAT(x) ((x)->rv_format&0xF)
#define GET_EXPECTED_SECTOR_TYPE(x) (((x)->rv_est_raddr >> 2)&3)
#define GET_READ_SIZE(x) (((x)->length[0] << 16) | ((x)->length[1] << 8) | ((x)->length[2]))

static INLINE ScsiTrackDescriptor *find_track_by_lba(uint32_t lba)
{
	ScsiTrackDescriptor *tracks;
	uint32_t num_sectors;
	int n;

	if (discfile_proxy)
	{
		tracks = discfile_proxy->tracks;
		num_sectors = discfile_proxy->size/cd_sector_size;
		n = discfile_proxy->numtracks;
	}
	else
	{
		tracks = discfile_cd->tracks;
		num_sectors = discfile_cd->num_sectors;
		n = discfile_cd->numtracks;
	}

	for (int i = 0; i < n; i++)
	{
		uint32_t track_start = tracks[i].track_start_addr;
		uint32_t track_end;

		if (i == (n - 1))
			track_end = num_sectors;
		else
			track_end = tracks[i + 1].track_start_addr;


		if (lba >= track_start && lba < track_end)
			return &tracks[i];
	}

	return NULL;
}

/*static uint16_t q_crc_lut[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108,
	0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
	0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B,
	0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
	0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE,
	0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
	0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D,
	0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5,
	0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
	0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
	0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
	0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13,
	0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
	0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E,
	0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1,
	0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
	0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0,
	0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
	0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657,
	0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
	0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882,
	0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E,
	0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
	0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D,
	0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
	0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

static INLINE uint16_t calculate_subq_crc(uint8_t *data)
{
	uint16_t crc = 0;
	int i;

	for (i = 0; i < 10; i++)
		crc = q_crc_lut[(crc >> 8) ^ data[i]] ^ (crc << 8);

	return ~crc;
}*/

int process_cd_iso_scsi_cmd(uint8_t *indata, uint64_t inlen, uint8_t *outdata, uint64_t outlen, int is2048)
{
	if (inlen < 1)
		return SUCCEEDED;

	switch (indata[0])
	{
		case SCSI_CMD_READ_TOC_PMA_ATIP:
		{
			ScsiCmdReadTocPmaAtip *cmd = (ScsiCmdReadTocPmaAtip *)indata;
			int numtracks;

			// TODO: this part needs change when adding proxy to ps2
			if (is2048)
				numtracks = 1;
			else
				numtracks = (discfile_proxy) ? discfile_proxy->numtracks : discfile_cd->numtracks;

			if (inlen < sizeof(ScsiCmdReadTocPmaAtip))
				return -1;

			if (GET_FORMAT(cmd) != FORMAT_TOC)
			{
				//DPRINTF("Requesting something other than TOC: %d!!\nPassing command to real function.", GET_FORMAT(cmd));
				return SUCCEEDED;
			}

			if (GET_MSF(cmd))
			{
				//DPRINTF("Warning: requesting tracks in MSF format. Not implemented.\n");
				return -1;
			}

			// Specific case of CDDA player
			if (outlen == 12)
			{
				ScsiTocResponse *resp = (ScsiTocResponse *)outdata;
				ScsiTrackDescriptor *track = (ScsiTrackDescriptor *)(resp + 1);
				int i;

				memset(resp, 0, outlen);

				resp->toc_length = sizeof(ScsiTocResponse) - sizeof(resp->toc_length) + (sizeof(ScsiTrackDescriptor) * (numtracks + 1));
				resp->first_track = 1;
				resp->last_track = numtracks;

				if (cmd->track_session_num > numtracks)
				{
					DPRINTF("Track out of range %d\n", cmd->track_session_num);
				}

				if (cmd->track_session_num > 1)
					resp->toc_length = resp->toc_length - ((cmd->track_session_num - 1) * sizeof(ScsiTrackDescriptor));

				if (cmd->track_session_num == 0)
					i = 0;
				else
					i = cmd->track_session_num - 1;

				if (is2048)
				{
					memset(track, 0, sizeof(ScsiTrackDescriptor));
					track->adr_control = 0x14;
					track->track_number = 1;
					track->track_start_addr = 0;
				}
				else
					memcpy(track, (discfile_proxy) ? &discfile_proxy->tracks[i] : &discfile_cd->tracks[i], sizeof(ScsiTrackDescriptor));

				return 1;

			}

			int alloc_size = sizeof(ScsiTocResponse);

			if (cmd->alloc_length > sizeof(ScsiTocResponse))
				alloc_size += (sizeof(ScsiTrackDescriptor) * (numtracks + 1));

			ScsiTocResponse *resp = malloc(alloc_size);
			resp->toc_length = sizeof(ScsiTocResponse) - sizeof(resp->toc_length) + (sizeof(ScsiTrackDescriptor) * (numtracks + 1));
			resp->first_track = 1;
			resp->last_track = numtracks;

			if (alloc_size > sizeof(ScsiTocResponse))
			{
				// TODO: this part needs change when adding proxy to PS2
				if (is2048)
				{
					ScsiTrackDescriptor *track = (ScsiTrackDescriptor *)(resp + 1);

					memset(track, 0, sizeof(ScsiTrackDescriptor));
					track->adr_control = 0x14;
					track->track_number = 1;
					track->track_start_addr = 0;
				}
				else
					memcpy(resp + 1, (discfile_proxy) ? discfile_proxy->tracks : discfile_cd->tracks, numtracks * sizeof(ScsiTrackDescriptor));

				ScsiTrackDescriptor *leadout = &((ScsiTrackDescriptor *)(resp + 1))[numtracks];

				memset(leadout, 0, sizeof(ScsiTrackDescriptor));
				leadout->adr_control = 0x10;
				leadout->track_number = 0xAA;

				// TODO: this part needs change when adding proxy to ps2
				if (is2048)
					leadout->track_start_addr = discfile->totalsize / 2048;
				else
					leadout->track_start_addr = (discfile_proxy) ? (discfile_proxy->size / cd_sector_size) : discfile_cd->num_sectors;
			}

			memcpy(outdata, resp, (outlen <= cmd->alloc_length) ? outlen : cmd->alloc_length);
			free(resp);
			return 1;
		}
		break;

		case SCSI_CMD_READ_TRACK_INFORMATION:
		{
			ScsiCmdReadTrackInformation *cmd = (ScsiCmdReadTrackInformation *)indata;
			ScsiTrackDescriptor *track;
			int numtracks;
			uint32_t track_size;

			if (inlen < sizeof(ScsiCmdReadTrackInformation))
				return -1;

			if (is2048)
			{
				//DPRINTF("READ TRACK INFORMATION not implemented for 2048 cd iso!\n");
				return -1;
			}

			if (cmd->rv_o_type != 1)
			{
				//DPRINTF("rv_o_type = %x, not implemented\n", cmd->rv_o_type);
				return -1;
			}

			numtracks = (discfile_proxy) ? discfile_proxy->numtracks : discfile_cd->numtracks;

			if (cmd->lba_tsn == 0 || cmd->lba_tsn > numtracks)
			{
				//DPRINTF("Invalid track %d\n", cmd->lba_tsn);
				return -1;
			}

			if (discfile_proxy)
			{
				track = &discfile_proxy->tracks[cmd->lba_tsn - 1];
				if (cmd->lba_tsn == numtracks)
					track_size = discfile_proxy->size/cd_sector_size - track->track_start_addr;
				else
					track_size = discfile_proxy->tracks[cmd->lba_tsn].track_start_addr - track->track_start_addr;
			}
			else
			{
				track = &discfile_cd->tracks[cmd->lba_tsn - 1];
				if (cmd->lba_tsn == numtracks)
					track_size = discfile_cd->num_sectors - track->track_start_addr;
				else
					track_size = discfile_cd->tracks[cmd->lba_tsn].track_start_addr - track->track_start_addr;
			}

			int alloc_size = sizeof(ScsiReadTrackInformationResponse);
			ScsiReadTrackInformationResponse *resp = malloc(alloc_size);

			memset(resp, 0, sizeof(ScsiReadTrackInformationResponse));
			resp->length = sizeof(ScsiReadTrackInformationResponse) - sizeof(resp->length);
			resp->ltn = cmd->lba_tsn & 0xFF;
			resp->sn = 1;
			resp->misc2 = (cmd->lba_tsn == 1) ? 2 : 0xF;
			resp->lt_start_addr = track->track_start_addr;
			resp->lt_size = track_size;

			memset(outdata, 0, outlen);
			memcpy(outdata, resp, (outlen <= resp->length) ? outlen : resp->length);
			free(resp);
			return 1;

		}
		break;

		case SCSI_CMD_READ_DISC_INFORMATION:
		{
			ScsiCmdReadDiscInformation *cmd = (ScsiCmdReadDiscInformation *)indata;

			if (inlen < sizeof(ScsiCmdReadDiscInformation))
				return -1;

			int alloc_size = sizeof(ScsiReadDiscInformationResponse);
			ScsiReadDiscInformationResponse *resp = malloc(alloc_size);

			memset(resp, 0, sizeof(ScsiReadDiscInformationResponse));
			resp->length = sizeof(ScsiReadDiscInformationResponse) - sizeof(resp->length);
			resp->misc = 0x0E;
			resp->first_track = 1;
			resp->num_sessions_lb = 1;
			resp->first_track_lastsession_lb = 1;

			// TODO: this part needs change when adding proxy support to PS2
			if (is2048)
				resp->last_track_lastsession_lb = 1;
			else
				resp->last_track_lastsession_lb = (discfile_proxy) ? discfile_proxy->numtracks : discfile_cd->numtracks;

			resp->misc2 = 0x20;
			resp->disctype = 0x20;
			resp->last_session_leadin = 0xFFFFFFFF;
			resp->last_session_leadout = 0xFFFFFFFF;

			memcpy(outdata, resp, (outlen <= cmd->alloc_length) ? outlen : cmd->alloc_length);
			free(resp);
			return 1;
		}
		break;

		case SCSI_CMD_READ_CD:
		{
			ScsiCmdReadCd *cmd = (ScsiCmdReadCd *)indata;
			ReadCdIso2352Cmd read_cmd;
			event_t event;
			uint64_t outsize;
			uint8_t *buf;
			int ret;
			int user_data = 1;

			if (cmd->misc == 0)
				user_data = 0;

			else if (cmd->misc != 0xF8 && cmd->misc != 0x10)
			{
				//DPRINTF("Unexpected value for misc: %02X\n", cmd->misc);
				return -1;
			}

			if (cmd->rv_scsb != 0 && cmd->rv_scsb != 2)
			{
				//DPRINTF("Unexpected value for subchannel: %02X\n", cmd->rv_scsb);
				return -1;
			}

			if (GET_EXPECTED_SECTOR_TYPE(cmd))
			{
				//DPRINTF("Unexpected value for expected sector type: %d\n", GET_EXPECTED_SECTOR_TYPE(cmd));
				return -1;
			}

			uint32_t length = GET_READ_SIZE(cmd);
			uint32_t lba = cmd->lba;
			process_t process = get_current_process_critical();

			if (is2048)
			{
				DPRINTF("Read CD on 2048 iso (lba=0x%x, length=0x%x)!!! Not implemented.\n", lba, length);
				return SUCCEEDED; // Fallback to real disc, let's see what happens :)
			}

			if (user_data)
				outsize = length * cd_sector_size;
			else
				outsize = 0;

			if (cmd->rv_scsb == 2)
				outsize += (length * sizeof(SubChannelQ));

			if (outsize == 0)
			{
				//DPRINTF("Warning: outsize is zero\n");
			}

			if (outsize > outlen)
			{
				ret = page_allocate_auto(process, outsize, 0x2F, (void **)&buf);
				if (ret)
					return -1;
			}
			else
				buf = outdata;

			if (cmd->rv_scsb == 0)
			{
				read_cmd.start_sector = lba;
				read_cmd.sector_count = length;
				read_cmd.buf = buf;
				read_cmd.process = process;

				event_port_send(command_port, CMD_READ_CD_ISO_2352, (uint64_t)&read_cmd, 0);
				ret = event_queue_receive(result_queue, &event, 0);
				if (ret == 0)
					ret = (int)(int64_t)event.data1;

				if (ret)
					return -1;
			}
			else
			{
				uint8_t *p = buf;
				extend_kstack(0);

				for (int i = 0; i < length; i++)
				{
					if (user_data)
					{
						read_cmd.start_sector = lba;
						read_cmd.sector_count = 1;
						read_cmd.buf = p;
						read_cmd.process = process;

						event_port_send(command_port, CMD_READ_CD_ISO_2352, (uint64_t)&read_cmd, 0);
						ret = event_queue_receive(result_queue, &event, 0);

						if (ret == 0)
							ret = (int)(int64_t)event.data1;

						if (ret)
							return -1;

						p += cd_sector_size;
					}

					SubChannelQ *subq = (SubChannelQ *)p;
					memset(subq, 0, sizeof(SubChannelQ));

					// custom subchannel
					ret = UNDEFINED;
					u32 lba2 = lba + 150;

					if(subqfd != UNDEFINED)
					{
						u8 n = LC_SECTORS, max = LC_SECTORS;
						if(lba2 <= lsd[15] && lba2 >= lsd[00]) // min 3
							n = 0, max = 16;
						else if(lba2 <= lsd[31] && lba2 >= lsd[16]) // min 9
							n = 16;

						for(; n < max; n++)
						{
							if(lsd[n] >  lba2)
								break;

							if(lsd[n] == lba2)
							{
								size_t r;
								cellFsLseek(subqfd, lsd_header + (n * lsd_struct), SEEK_SET, &r);
								ret = cellFsRead(subqfd, (void *)subq, 10, &r);

								if(subq->control_adr <= 0 || r != 10)
									ret = UNDEFINED;

								break;
							}
						}
					}

					// use default subchannel data
					if(ret)
					{
						ScsiTrackDescriptor *track = find_track_by_lba(lba);
						subq->control_adr = ((track->adr_control << 4) & 0xF0) | (track->adr_control >> 4);
						subq->track_number = track->track_number;
						subq->index_number = 1;

						if (user_data)
							lba_to_msf_bcd(lba, &subq->min, &subq->sec, &subq->frame);

						lba_to_msf_bcd(lba2, &subq->amin, &subq->asec, &subq->aframe);
						//subq->crc = calculate_subq_crc((u8 *)subq); // this crc is ignored & recalculated by ps1 emulator
					}

					p += sizeof(SubChannelQ);
					lba++;
				}
			}

			if (outsize > outlen)
			{
				memcpy(outdata, buf, outlen);
				page_free(process, buf, 0x2F);
			}

			return 1;

			//DPRINTF("READ CD, sector %x size %x, expected sector type: %d\n", cmd->lba, s, GET_EXPECTED_SECTOR_TYPE(cmd));
			//DPRINTF("Misc: %02X, rv_scsb: %02X, outlen = %lu\n", cmd->misc, cmd->rv_scsb, outlen);
		}
		break;

		default:
			if (total_emulation)
				return process_generic_iso_scsi_cmd(indata, inlen, outdata, outlen);
	}

	return SUCCEEDED;
}

static INLINE int get_psx_video_mode(void)
{
	int ret = -1;
	event_t event;

	event_port_send(command_port, CMD_GET_PSX_VIDEO_MODE, 0, 0);
	if (event_queue_receive(result_queue, &event, 0) == 0)
		ret = (int)(int64_t)event.data1;

	return ret;
}

static INLINE void do_video_mode_patch(void)
{
	process_t p = get_current_process_critical();

	if (p == vsh_process)
	{
		uint32_t patch = 0;

		if (effective_disctype == DEVICE_TYPE_PSX_CD)
		{
			if (video_mode != 2)
			{
				int ret = forced_video_mode ? (forced_video_mode - 1) : get_psx_video_mode();
				if (ret >= 0) //{
					video_mode = ret;/*condition_game_ext_psx=0;}
				 if(ret == -1 || ps2emu_type!=PS2EMU_SW) {condition_game_ext_psx=1;}*/
			}
		}
		else
		{
			if (video_mode >= 0)
				video_mode = -1;
		}

		if (video_mode >= 0)
		{
			if (video_mode < 2)
			{
				patch = LI(R0, video_mode);
				video_mode = 2;
			}
		}
		else if (video_mode == -1)
		{
			patch = LWZ(R0, 0x74, SP);
			video_mode = -2;
		}

		if (patch) //&& !condition_game_ext_psx)
		{
			switch(vsh_check)
			{
				case VSH_HASH:
					DPRINTF("Patching Video mode in Retail VSH..\n");

					if(vmode_patch_offset)
						copy_to_user(&patch, (void *)(vmode_patch_offset + _64KB_), 4);

					//DPRINTF("Offset: 0x%08X | Data: 0x%08X\n", (uint32_t)vmode_patch_offset, (uint32_t)patch);
				break;

				default:
					DPRINTF("Unknown VSH HASH, Video mode was not patched!\n");
				break;
			}
		}
	}
}

int process_cmd(unsigned int command, void *indata, uint64_t inlen, void *outdata, uint64_t outlen)
{
	int ret = 0;

	switch (command)
	{
		case STORAGE_COMMAND_GET_DEVICE_SIZE:

			do_video_mode_patch();

			if (disc_emulation != EMU_OFF)
			{
				uint64_t ret;

				if (discfile_cd)
					ret = discfile_cd->num_sectors;
				else if (discfile_proxy)
					ret = (discfile_proxy->tracks) ? discfile_proxy->size/cd_sector_size : discfile_proxy->size/2048;

				else
					ret = discfile->totalsize / 2048;

				ret = (ret << 32) | 2048;
				memset(outdata, 0, outlen);
				memcpy(outdata, &ret, (sizeof(ret) > outlen) ? sizeof(ret) : outlen);

				//DPRINTF("FAKING to %16lx\n", ret);

				return 1;
			}
		break;

		case STORAGE_COMMAND_GET_DEVICE_TYPE:
			if (fake_disctype)
			{
				memset(outdata, 0, outlen);
				memcpy(outdata, &fake_disctype, (sizeof(fake_disctype) > outlen) ? sizeof(fake_disctype) : outlen);
				return 1;
			}
		break;

		case STORAGE_COMMAND_NATIVE:
		{
			uint8_t cmd = *(uint8_t *)indata;

			if ((effective_disctype == DEVICE_TYPE_PSX_CD || effective_disctype == DEVICE_TYPE_PS2_CD
				|| effective_disctype == DEVICE_TYPE_PS2_DVD) && cmd == SCSI_CMD_GET_CONFIGURATION)
			{
				// Region bypass on original psx/ps2 disc
				memset(outdata, 0, outlen);
				return 1;
			}

			if (disc_emulation != EMU_OFF)
			{
				if (discfile_cd || (discfile_proxy && discfile_proxy->tracks))
					return process_cd_iso_scsi_cmd(indata, inlen, outdata, outlen, 0);
				else if (disc_emulation == EMU_PS2_CD)
					return process_cd_iso_scsi_cmd(indata, inlen, outdata, outlen, 1);
				else
				{
					if (total_emulation)
						return process_generic_iso_scsi_cmd(indata, inlen, outdata, outlen);
				}
			}
		}
		break;
	}

	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_7(int, emu_storage_send_device_command, (device_handle_t device_handle, unsigned int command, void *indata, uint64_t inlen, void *outdata, uint64_t outlen, uint64_t *unkret))
{
	uint64_t device;
	int ret = DO_POSTCALL;

	if (get_handle_device(device_handle, &device) == 0)
	{
		if (device == BDVD_DRIVE)
		{
			mutex_lock(mutex, 0);

			int tri = process_cmd(command, indata, inlen, outdata, outlen);

			if (tri > 0)
				ret = 0;
			else if (tri < 0)
				ret = tri;

			mutex_unlock(mutex);
		}
	}
	/*int64_t debug_print(const char* buffer, size_t size);
	void debug_print_hex(void *buf, uint64_t size);
	void debug_print_hex_c(void *buf, uint64_t size);

	DPRINTF("sys_storage_send_device_command\n");*/

	return ret;
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_7(int, emu_sys_storage_async_send_device_command, (sys_device_handle_t device_handle, unsigned int command, void *indata, uint64_t inlen, void *outdata, uint64_t outlen, uint64_t param))
{
	object_handle_t obj_handle;
	uint64_t *sys_storage_object;
	void *table;
	int ret = DO_POSTCALL;

	table = get_current_process_critical()->object_table;

	if (open_shared_kernel_object(table, device_handle, (void **)&sys_storage_object, &obj_handle, SYS_STORAGE_HANDLE_OBJECT, 1) == 0)
	{
		uint64_t device = sys_storage_object[8 / 8];

		if (device == BDVD_DRIVE)
		{
			mutex_t storage_mutex;
			event_port_t async_port;

			storage_mutex = (mutex_t)sys_storage_object[0x98/8];
			mutex_lock(storage_mutex, 0);

			async_port = (event_port_t) ((uint64_t *)sys_storage_object[(0x28 + 8) / 8])[0x30 / 8];
			mutex_lock(mutex, 0);

			int tri = process_cmd(command, get_secure_user_ptr(indata), inlen, get_secure_user_ptr(outdata), outlen);

			if (tri > 0)
			{
				ret = 0;
				event_port_send(async_port, param, 0, 0);
			}
			else if (tri < 0)
			{
				ret = 0;
				event_port_send(async_port, param, -1, 0);
			}

			mutex_unlock(mutex);
			mutex_unlock(storage_mutex);
		}

		close_kernel_object_handle(table, obj_handle);
	}

	return ret;
}

static INLINE unsigned int get_disc_type(/*int hooked*/)
{
	device_handle_t handle;
	unsigned int ret = 0;

	if (storage_open(BDVD_DRIVE, 0, &handle, 0) == 0)
	{
		uint32_t indata = 0x01010000;
		unsigned int disctype;
		int res;

		/*if (!hooked)
		{
			res = storage_send_device_command(handle, STORAGE_COMMAND_GET_DEVICE_TYPE, &indata,
							  sizeof(indata), &disctype, sizeof(disctype), NULL);
		}
		else*/
		{
			res = (int)call_hooked_function_7(emu_storage_send_device_command, (uint64_t)handle, STORAGE_COMMAND_GET_DEVICE_TYPE, (uint64_t)&indata,
							  sizeof(indata), (uint64_t)&disctype, sizeof(disctype), (uint64_t)NULL);
		}

		if (res == 0 && disctype != 0)
			ret = disctype;

		storage_close(handle);
	}

	return ret;
}

static void fake_reinsert(unsigned int disctype)
{
	FakeStorageEventCmd cmd;

	cmd.param = (uint64_t)(disctype) << 32;
	cmd.device = BDVD_DRIVE;

	cmd.event = 4;
	process_fake_storage_event_cmd(&cmd);
	cmd.event = 8;
	process_fake_storage_event_cmd(&cmd);
	cmd.event = 7;
	process_fake_storage_event_cmd(&cmd);
	cmd.event = 3;
	process_fake_storage_event_cmd(&cmd);
}

void restore_BD()
{
	mutex_lock(mutex, 0);

	unsigned int disctype = get_disc_type();
	fake_reinsert(disctype);

	mutex_unlock(mutex);
}

LV2_HOOKED_FUNCTION_COND_POSTCALL_2(int, emu_disc_auth, (uint64_t func, uint64_t param))
{
	DPRINTF("Disc auth: %lx %lx (process: %s)\n", func, param, get_process_name(get_current_process_critical()));

	if (func == 0x5004)
	{
		uint32_t param5004 = param;

		if (param5004 == 1) /* Auth psx disc */
		{
			if (vsh_process && get_current_process_critical() == vsh_process && effective_disctype == DEVICE_TYPE_PSX_CD)
			{
				// Just bypass auth and leave current 0x29 profile
				return SUCCEEDED;
			}
		}
		else if (param5004 == 0x29)
		{
			if (vsh_process && get_current_process_critical() == vsh_process)
			{
				if (could_not_read_disc)
				{
					could_not_read_disc = 0;
					mutex_lock(mutex, 0);

					int ret = call_hooked_function_2(emu_disc_auth, func, param); // Recursive!
					if (ret == 0)
						fake_reinsert(get_disc_type());

					mutex_unlock(mutex);
					return ret;
				}
			}
		}
	}
	else if (func == 0x5007)
	{
		if (param == 0x43)
			return SUCCEEDED;

		if (( (emu_ps3_rec && disc_emulation == EMU_OFF) || disc_emulation == EMU_PS3) && real_disctype != DEVICE_TYPE_PS3_BD)
		{
			static int inloop = 0;

			if (!inloop)
			{
				inloop = 1;
				call_hooked_function_2(emu_disc_auth, func, param); // Recursive!
				return SUCCEEDED; /* return 0 regardless of result */
			}
			else
				inloop = 0;
		}
	}

	return DO_POSTCALL;
}

LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_8(int, post_cellFsUtilMount, (const char *block_dev, const char *filesystem, const char *mount_point, int unk, int read_only, int unk2, char *argv[], int argc))
{
	DPRINTF("cellFsUtilMount: %s\n", mount_point);

	if (!hdd0_mounted && strcmp(mount_point, "/dev_hdd0") == 0 && strcmp(filesystem, "CELL_FS_UFS") == 0)
	{
		hdd0_mounted = 1;
		copy_ps2emu_stage2(ps2emu_type);
		cellFsUnlink("/dev_hdd0/tmp/loadoptical");
		read_cobra_config();
		load_fan_control(fan_speed);
		disable_cfw_syscalls(syscalls_mode);
		patch_coldboot();
		//patch_epilepsy_warning();
		// do_spoof_patches();
		load_boot_plugins();
		load_boot_plugins_kernel();
		patch_overclock();	
		// update_hashes();

		mutex_lock(mutex, 0);

		if (real_disctype == 0)
		{
			unsigned int disctype = get_disc_type();

			if (disctype == DEVICE_TYPE_CD || disctype == DEVICE_TYPE_DVD)
				fake_reinsert(disctype);
			else if (disctype)
				process_disc_insert(disctype);
		}

		mutex_unlock(mutex);

		/*#ifndef DEBUG
			unhook_function_on_precall_success(cellFsUtilMount_symbol, post_cellFsUtilMount, 8); // Hook no more needed
		#endif*/
	}

	// Do overclock on boot (By Evilnat)
	if (!usb000_mounted && strcmp(mount_point, "/dev_usb000") == 0 && strcmp(filesystem, "CELL_FS_FAT") == 0)
	{
		usb000_mounted = 1;
		patch_overclock();
	}

	// CFW2OFW Fix by Evilnat
	// Fixes black screen while a CFW2OFW game is loaded with a mounted JB folder game
	if(CFW2OFW_game && !strcmp(mount_point, "/dev_bdvd/PS3_GAME"))
	{
		//DPRINTF("Detected CFW2OFW game\n");

		sys_storage_ext_umount_discfile();

		map_path("/dev_bdvd/PS3_GAME", NULL, 0);
		map_path("/app_home/PS3_GAME", NULL, 0);

		map_path("/dev_bdvd", NULL, 0);
		map_path("//dev_bdvd", NULL, 0);

		map_path("/app_home", NULL, 0);
		map_path("//app_home", NULL, 0);
	}

	return SUCCEEDED;
}

static INLINE int get_ps2emu_type(void)
{
	uint8_t config[8];
	u64 v2;

	lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("sys", 0), FIELD("hw", 0), FIELD("config", 0), 0, (u64 *)config, &v2);

	if (config[6] & 1) // has emotion engine
		return PS2EMU_HW;
	else if (config[0] & 0x20) // has graphics synthesizer
		return PS2EMU_GX;

	return PS2EMU_SW;
}

char *ps2emu_stage2[] =
{
	"ps2hwemu_stage2.bin",
	"ps2gxemu_stage2.bin",
};

void copy_ps2emu_stage2(int emu_type)
{
	char name[64];
	int src, dst;
	uint8_t *buf;

	if (emu_type < 0 || emu_type > PS2EMU_GX)
		return;

	page_allocate_auto(NULL, _64KB_, 0x2F, (void **)&buf);

	sprintf(name, "/dev_flash/ps2emu/%s", ps2emu_stage2[emu_type]);

	if (cellFsOpen(name, CELL_FS_O_RDONLY, &src, 0, NULL, 0) == 0)
	{
		uint64_t size;

		cellFsRead(src, buf, _64KB_, &size);
		cellFsClose(src);

		if (cellFsOpen(PS2EMU_STAGE2_FILE, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &dst, 0666, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			cellFsWrite(dst, buf, size, &size);
			cellFsClose(dst);
		}
	}
	else
	{
		DPRINTF("Failed to open ps2 stage2: %s\n", name);
	}

	page_free(NULL, buf, 0x2F);
}

static void build_netemu_params(uint8_t *ps2_soft, uint8_t *ps2_net)
{
	int fd;
	uint64_t written;

	// First take care of sm arguments
	memset(ps2_net, 0, 0x40);
	memcpy(ps2_net, ps2_soft, 8);
	ps2_net[8] = 3;
	strcpy((char *)ps2_net + 9, "--COBRA--");
	memcpy(ps2_net + 0x2A, ps2_soft + 0x118, 6);

	// Now ps2bootparam
	if (cellFsOpen("/dev_hdd0/tmp/game/ps2bootparam.dat", CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0) != CELL_FS_SUCCEEDED)
	{
		DPRINTF("Cannot open ps2bootparam.dat\n");
		return;
	}

	uint64_t static_one = { 0x054c026840000000 };
	uint64_t static_two = { 0x3600043504082225 };

	// netemu ps2bootparam.dat has a format very similar to softemu sm arguments
	ps2_soft[11] = 3;
	ps2_soft[0x4d0] = 8;
	ps2_soft[0x4d7] = 6;
	strcpy((char *)ps2_soft + 12, "--COBRA--");
	memset(ps2_soft + 0x4f0, 0, 0x2204);
	int controller_count = 0;

	// patch controllers
	uint64_t controller, offset;
	for(uint32_t i = 0; i < 11; i++)
	{
		memcpy(&controller, ps2_soft + 0x98 + (8 * i), 8);
		if(controller)
		{
			offset = (0x218 * i);
			memcpy(ps2_soft + offset + 0x4f4, &controller, 8);
			memcpy(ps2_soft + offset + 0x4f4 + 0x8, &static_one, 8);
			ps2_soft[offset + 0x515] = 9;
			memcpy(ps2_soft + offset + 0x516, &static_two, 8);
			controller_count++;
		}
	}

	ps2_soft[0x4f3] = controller_count;

	cellFsWrite(fd, ps2_soft, 0x26f4, &written);

	// netemu has a 0x4F0-0x773 section where custom memory card is, but we dont need it,
	// Not writing it + a patch in netemu forces the emu to use the internal ones like BC consoles
	// NPDRM games will still use their memcards as the section is written
	cellFsClose(fd);
}

LV2_HOOKED_FUNCTION(int, shutdown_copy_params_patched, (uint8_t *argp_user, uint8_t *argp, uint64_t args, uint64_t param))
{
	int prepare_ps2emu = 0;

	copy_from_user(argp_user, argp, args);
	extend_kstack(0);

	if (param == 0x8202) /* Reboot into PS2 LPAR (ps2_emu, ps2_gxemu or ps2_softemu) */
		prepare_ps2emu = 1;
	else if (param == 0x8204) /* Reboot into ps2_netemu LPAR */
	{
		DPRINTF("Reboot into ps2_netemu LPAR (0x8204), ps2emu_type = %i\n", ps2emu_type);

		// Delete ps2emu config file when ps2_netemu is loaded on BC/Semi-BC Consoles to fix the issue with ISO redirection
		// Credits @Habib and @aldostools
		if(ps2emu_type == PS2EMU_HW || ps2emu_type == PS2EMU_GX)
		{
			//DPRINTF("Deleting %s\n", PS2EMU_CONFIG_FILE);
			cellFsUnlink(PS2EMU_CONFIG_FILE);
		}

		// We need to check first if this a NPDRM or a plain iso launched from disc icon
		// Discard first the case of BC consoles, since the ps2tonet patch is not done there
		if (argp[12] == 0) /* if vsh prepared the arguments for ps2_softemu */
		{
			if (disc_emulation == EMU_OFF)
			{
				// If optical disc, let's just panic
				if (effective_disctype == DEVICE_TYPE_PS2_CD || effective_disctype == DEVICE_TYPE_PS2_DVD)
				{
					fatal("Sorry, no optical disc support in ps2_netemu\n");
				}
				else
				{
					// We should be never here "naturally" (a hb could still force this situation)
					// Well, maybe if the user quckly removed a ps2 disc...
				}
			}
			else if (disc_emulation == EMU_PS2_CD || disc_emulation == EMU_PS2_DVD)
			{
				prepare_ps2emu = 1;
				build_netemu_params(get_secure_user_ptr(argp_user), argp);
			}
			else
			{
				// We should be never here "naturally" (a hb could still force this situation)
			}
		}
		else
		{
			cellFsUnlink(PS2EMU_CONFIG_FILE);

			if (disc_emulation == EMU_PS2_CD || disc_emulation == EMU_PS2_DVD)
				prepare_ps2emu = 1;
			else
			{
				//DPRINTF("NPDRM game, skipping ps2emu preparation\n");

				// Set FAN Speed while launching a PS2 Classic game
				if(ps2_speed)
					sm_set_fan_policy(0, ((ps2_speed == 1) ? 1 : 2), ((ps2_speed == 1) ? 0 : ps2_speed));
			}
		}
	}

	if (prepare_ps2emu)
	{
		int fd;

		// Set FAN Speed while you are in a PS2 game
		if(ps2_speed)
			sm_set_fan_policy(0, (ps2_speed == 1) ? 1 : 2, (ps2_speed == 1) ? 0 : ps2_speed);

		if (cellFsOpen(PS2EMU_CONFIG_FILE, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			if (disc_emulation == EMU_PS2_DVD || disc_emulation == EMU_PS2_CD)
			{
				uint64_t nwritten;
				uint8_t *buf;

				page_allocate_auto(NULL, _4KB_, 0x2F, (void **)&buf);

				memset(buf, 0, _4KB_);
				// bit 0-> is cd
				// bit 1 -> total emulation
				buf[0] = (disc_emulation == EMU_PS2_CD) | ((real_disctype == 0) << 1);
				strncpy((char *)buf + 1, (discfile_cd) ? discfile_cd->file : discfile->files[0], 0x7FE);

				// TODO: this will need change when adding proxy to PS2
				if (discfile_cd)
				{
					buf[0x800] = discfile_cd->numtracks;
					memcpy(buf+0x801, discfile_cd->tracks, discfile_cd->numtracks*sizeof(ScsiTrackDescriptor));
				}

				buf[0x702] = 0x6d;
				buf[0x703] = 0x6f;
				buf[0x704] = 0x75;
				buf[0x705] = 0x6e;
				buf[0x706] = 0x74;

				cellFsWrite(fd, buf, _4KB_, &nwritten);
				cellFsClose(fd);

				page_free(NULL, buf, 0x2F);
			}
			else
			{
				cellFsClose(fd);

				// Delete file only on original disc, otherwise the file will be empty
				if (real_disctype == DEVICE_TYPE_PS2_DVD || real_disctype == DEVICE_TYPE_PS2_CD)
					cellFsUnlink(PS2EMU_CONFIG_FILE);
			}
		}

		copy_ps2emu_stage2(ps2emu_type);
	}

	return SUCCEEDED;
}

static INLINE void do_umount_discfile(void)
{
	if (discfd != UNDEFINED)
	{
		cellFsClose(discfd);
		discfd = UNDEFINED;
	}

	if (subqfd != UNDEFINED)
	{
		cellFsClose(subqfd);
		subqfd = UNDEFINED;
	}

	if (discfile)
	{
		if (discfile->cached_sector)
			xfree(discfile->cached_sector);

		free(discfile);
		discfile = NULL;
	}

	if (discfile_cd)
	{
		if (discfile_cd->cache)
			page_free(NULL, discfile_cd->cache, 0x2F);

		free(discfile_cd);
		discfile_cd = NULL;
	}

	if (discfile_proxy)
	{
		if (discfile_proxy->cached_sector)
			xfree(discfile_proxy->cached_sector);

		free(discfile_proxy);
		discfile_proxy = NULL;

		if (proxy_command_port)
		{
			event_port_disconnect(proxy_command_port);
			event_port_destroy(proxy_command_port);
			event_queue_destroy(proxy_result_queue);
			proxy_command_port = NULL;
		}
	}

	forced_video_mode = 0;
	disc_emulation = EMU_OFF;
	total_emulation = 0;
	emu_ps3_rec = 0;
	base_offset = 0;
}

static INLINE int check_files_and_allocate(unsigned int filescount, char *files[])
{
	base_offset = 0;

	if (filescount == 0 || filescount > 32)
		return EINVAL;

	int allocsize = sizeof(DiscFile) + (sizeof(char *) * filescount) + (sizeof(uint64_t) * filescount);

	for (int i = 0; i < filescount; i++)
	{
		int len = strlen(files[i]);
		if ((len >= MAX_PATH) || (len < 4))
			return EINVAL;

		if(strcmp(files[i] + (len - 4), ".PNG") == 0) 
			base_offset = _64KB_; // EXT

		allocsize += (len + 1);
	}

	discfile = malloc(allocsize);
	if (!discfile)
		return ENOMEM;

	discfile->count = filescount;
	discfile->activefile = 0;
	discfile->totalsize = 0;
	discfile->files = (char **)(discfile + 1);
	discfile->sizes = (uint64_t *)(discfile->files+filescount);
	char *p = (char *)(discfile->sizes+filescount);

	for (int i = 0; i < filescount; i++)
	{
		CellFsStat stat;

		int ret = cellFsStat(files[i], &stat);
		if (ret)
		{
			free(discfile);
			discfile = NULL;
			return ret;
		}

		//DPRINTF("%s, filesize: %lx\n", files[i], stat.st_size);

		// 3k3y/Redump ISOs decryption on-the-fly (By Evilnat)
		get_key(files[i]);

		if (stat.st_size < (4096 + base_offset))
		{
			free(discfile);
			discfile = NULL;
			return EINVAL;
		}

		discfile->sizes[i] = (stat.st_size - base_offset);
		discfile->totalsize += discfile->sizes[i];
		discfile->files[i] = p;
		strcpy(p, files[i]);
		p += (strlen(p) + 1);
	}

	return SUCCEEDED;
}

int mount_common(unsigned int filescount, char *files[])
{
	if (disc_emulation != EMU_OFF)
		return EBUSY;

	int ret = check_files_and_allocate(filescount, files);
	if (ret)
		return ret;

	discfile->cached_sector = NULL;
	discfile->cached_offset = 0;

	return SUCCEEDED;
}

int mount_ps3_discfile(unsigned int filescount, char *files[])
{
	int ret;
	mutex_lock(mutex, 0);

	ret = mount_common(filescount, files);
	if (ret == 0)
	{
		disc_emulation = EMU_PS3;
		total_emulation = (!disc_being_mounted && real_disctype == 0);
	}

	mutex_unlock(mutex);
	return ret;
}

int mount_dvd_discfile(unsigned int filescount, char *files[])
{
	int ret;
	mutex_lock(mutex, 0);

	ret = mount_common(filescount, files);
	if (ret == 0)
	{
		disc_emulation = EMU_DVD;
		total_emulation = (!disc_being_mounted && real_disctype == 0);
	}

	mutex_unlock(mutex);
	return ret;
}

int mount_bd_discfile(unsigned int filescount, char *files[])
{
	int ret;
	mutex_lock(mutex, 0);

	ret = mount_common(filescount, files);
	if (ret == 0)
	{
		disc_emulation = EMU_BD;
		total_emulation = (!disc_being_mounted && real_disctype == 0);
	}

	mutex_unlock(mutex);
	return ret;
}

int mount_ps_cd(char *file, unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	int ret;
	int len;

	if (disc_emulation != EMU_OFF)
		return EBUSY;

	// -- AV: cd sector size
	get_cd_sector_size(trackscount);
	trackscount &= 0xff;

	len = strlen(file);

	if (len >= MAX_PATH || trackscount >= 100)
		ret = EINVAL;
	else
	{
		CellFsStat stat;

		ret = cellFsStat(file, &stat);

		size_t cd_size = stat.st_size;
		if(cd_size < 0x9930)
			ret = EINVAL;

		if (ret == 0)
		{
			// -- AV: cd sector size
			char *ext = file + (len - 4);

			if(strcmp(ext, ".PNG") == 0)
				base_offset = _64KB_; // EXT

			if(cd_sector_size == 2352)
			{
				///////// Open LSD (LibCrypt Subchannel Data) for protected PAL games /////// AV:2022-06-02
				char file_ext[5];
				strcpy(file_ext, ext);

				strcpy(ext, ".lsd");
				ret = cellFsStat(file, &stat);

				if(ret)
				{
					strcpy(ext, ".sbi");
					ret = cellFsStat(file, &stat);
				}
				
				if(ret == 0)
					read_libcrypt_sectors(file);
				
				strcpy(ext, file_ext);
				////////////////////////////////////

				// force video by title id in file name
				if(strstr(file, "NTSC"))
					forced_video_mode = 1;
				if(strstr(file, "PAL"))
					forced_video_mode = 2;

				// detect sector size
				ret = cellFsOpen(file, CELL_FS_O_RDONLY, &discfd, 0, NULL, 0);
				
				if(ret == CELL_FS_SUCCEEDED)
				{
					char buffer[20];
					uint64_t v;

					u16 sec_size[7] = { 2352, 2048, 2336, 2448, 2328, 2340, 2368 };

					for(u8 n = 0; n < 7; n++)
					{
						cellFsLseek(discfd, base_offset + (sec_size[n] << 4) + 0x18, SEEK_SET, &v);
						cellFsRead(discfd, buffer, 20, &v);
						
						if((memcmp(buffer + 8, "PLAYSTATION ", 0xC) == 0) ||
						  ((memcmp(buffer + 1, "CD001", 5) == 0) && buffer[0] == 0x01)) 
						{
							cd_sector_size = sec_size[n]; 
							break;
						}
					}
					
					cellFsClose(discfd);
				}

				discfd = -1;
			}
			// --

			discfile_cd = malloc(sizeof(DiscFileCD) + (len + 1) + (trackscount * sizeof(ScsiTrackDescriptor)));
			page_allocate_auto(NULL, CD_CACHE_SIZE * cd_sector_size, 0x2F, (void **)&discfile_cd->cache);

			discfile_cd->num_sectors = cd_size / cd_sector_size;
			discfile_cd->numtracks = trackscount;
			discfile_cd->cached_sector = 0x80000000;
			discfile_cd->tracks = (ScsiTrackDescriptor *)(discfile_cd + 1);
			discfile_cd->file = (char *)(discfile_cd->tracks + trackscount);

			strcpy(discfile_cd->file, file);

			for (int i = 0; i < trackscount; i++)
				memcpy(&discfile_cd->tracks[i], &tracks[i], sizeof(ScsiTrackDescriptor));
		}
		else
			ret = EINVAL;
	}

	return ret;
}

int mount_psx_discfile(char *file, unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	int ret;

	mutex_lock(mutex, 0);

	ret = mount_ps_cd(file, trackscount, tracks);
	if (ret == 0)
	{
		disc_emulation = EMU_PSX;
		total_emulation = (!disc_being_mounted && real_disctype == 0);
	}

	mutex_unlock(mutex);
	return ret;
}

int mount_ps2_discfile(unsigned int filescount, char *files[], unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	if (filescount != 1)
		return EINVAL; // We don't support more than 1 file atm

	int is_cd = 0;
	int is_2352 = 0;
	int ret = 0;

	if(strcmp(files[0] + (strlen(files[0]) - 4), ".PNG") == 0) base_offset = _64KB_; // EXT

	if (trackscount > 1)
	{
		// We assume cd 2352 (cd_sector_size) here
		is_cd = 1;
		is_2352 = 1;
	}
	else
	{
		int fd;
		uint64_t pos, nread;
		uint8_t buf[0xB0];

		ret = cellFsOpen(files[0], CELL_FS_O_RDONLY, &fd, 0, NULL, 0);
		if (ret)
			return ret;

		cellFsLseek(fd, base_offset + 0x8000, SEEK_SET, &pos);
		ret = cellFsRead(fd, buf, sizeof(buf), &nread);
		cellFsClose(fd);

		if (ret)
			return ret;
		else if (nread != sizeof(buf))
			return EINVAL;

		if (buf[0] == 1 && memcmp(buf + 1, "CD001", 5) == 0)
		{
			// rootToc.tocSize == 0x800 -> CD; else DVD
			if (*(uint32_t *)&buf[0xAA] == 0x800)
				is_cd = 1;
		}
		else
		{
			// We assume it is a cd 2352 (cd_sector_size) iso, and thus, a cd
			is_cd = 1;
			is_2352 = 1;
		}
	}

	mutex_lock(mutex, 0);

	if (is_2352)
		ret = mount_ps_cd(files[0], trackscount, tracks);
	else
		ret = mount_common(filescount, files);

	if (ret == 0)
	{
		disc_emulation = (is_cd) ? EMU_PS2_CD : EMU_PS2_DVD;
		total_emulation = (!disc_being_mounted && real_disctype == 0);
	}

	mutex_unlock(mutex);
	return ret;
}

int umount_discfile(void)
{
	int ret = 0;

	mutex_lock(mutex, 0);

	if (disc_emulation)
		do_umount_discfile();
	else
		ret = -1;

	mutex_unlock(mutex);
	return ret;

}

// Deprecated
/*LV2_PATCHED_FUNCTION(int, fsloop_open, (const char *path, int flags, int *fd, int mode, void *arg, uint64_t size))
{
	int ret = cellFsOpen(path, flags, fd, mode, arg, size);

	if (ret == 0)
	{
		if (encrypted_image && strcmp(encrypted_image, path) == 0)
		{
			DPRINTF("Encrypted image open: %s\n", path);

			encrypted_image_fd = *fd;
		}
	}

	return ret;
}*/

// Deprecated
/*LV2_PATCHED_FUNCTION(int, fsloop_close, (int fd))
{
	int ret = cellFsClose(fd);

	if (ret == 0 && encrypted_image_fd == fd)
	{
		DPRINTF("encrypted image close\n");

		encrypted_image_fd = -1;
	}

	return cellFsClose(fd);
}*/

// Deprecated
/*LV2_PATCHED_FUNCTION(int, fsloop_read, (int fd, void *buf, uint64_t nbytes, uint64_t *nread))
{
	uint64_t pos;

	cellFsLseek(fd, 0, SEEK_CUR, &pos);

	int ret = cellFsRead(fd, buf, nbytes, nread);

	if (ret == 0 && fd == encrypted_image_fd)
	{
		if (pos&7 || nbytes&7)
		{
			DPRINTF("CRITICAL: we didn't expect this kind of read %lx %lx\n", pos, nbytes);

			while (1);
		}

		xtea_ctr(encrypted_image_keys, encrypted_image_nonce+(pos/8), buf, nbytes);
	}

	return ret;
}*/

int sys_storage_ext_get_disc_type(unsigned int *rdt, unsigned int *edt, unsigned int *fdt)
{
	mutex_lock(mutex, 0);
	copy_to_user(&real_disctype, get_secure_user_ptr(rdt), sizeof(real_disctype));
	copy_to_user(&effective_disctype, get_secure_user_ptr(edt), sizeof(effective_disctype));
	copy_to_user(&fake_disctype, get_secure_user_ptr(fdt), sizeof(fake_disctype));
	mutex_unlock(mutex);

	return SUCCEEDED;
}

int sys_storage_ext_read_ps3_disc(void *buf, uint64_t start_sector, uint32_t count)
{
	void *object, *unk1;
	fs_object_handle_t handle;
	int ret;

	object = NULL;
	unk1 = NULL;
	handle = NULL;

	ret = open_fs_object(NULL, "/dev_bdvd", &object, &unk1, &handle, NULL);
	if (ret)
		return ret;

	if (!object)
	{
		close_fs_object(NULL, handle);
		return ESRCH;
	}

	ret = (int)call_hooked_function_4(emu_read_bdvd1, (uint64_t)object, (uint64_t)get_secure_user_ptr(buf), count*2048, start_sector*2048);
	close_fs_object(NULL, handle);
	return ret;
}

int sys_storage_ext_fake_storage_event(uint64_t event, uint64_t param, uint64_t device)
{
	FakeStorageEventCmd cmd;

	mutex_lock(mutex, 0);

	cmd.event = event;
	cmd.param = param;
	cmd.device = device;

	int ret = event_port_send(command_port, CMD_FAKE_STORAGE_EVENT, (uint64_t)&cmd, 0);
	if (ret == 0)
	{
		event_t event;
		ret = event_queue_receive(result_queue, &event, 0);

		if (ret == 0)
			ret = (int)event.data1;
	}

	mutex_unlock(mutex);

	return ret;
}

int sys_storage_ext_get_emu_state(sys_emu_state_t *state)
{
	int ret;

	state = get_secure_user_ptr(state);

	if (!state)
		return EINVAL;

	if (state->size != sizeof(sys_emu_state_t))
	{
		//DPRINTF("Unknown structure size: %d, expected %ld\n", state->size, sizeof(sys_emu_state_t));
		return EINVAL;
	}

	mutex_lock(mutex, 0);

	ret = copy_to_user(&disc_emulation, &state->disc_emulation, sizeof(disc_emulation));
	if (ret == 0)
	{
		// No size check needed as that was done in mount
		if (disc_emulation != EMU_OFF)
		{
			if (discfile_cd)
				ret = copy_to_user(discfile_cd->file, state->firstfile_path, strlen(discfile_cd->file) + 1);
			else if (discfile)
				ret = copy_to_user(discfile->files[0], state->firstfile_path, strlen(discfile->files[0]) + 1);
			else
			{
				char c = 0;
				ret = copy_to_user(&c, state->firstfile_path, 1);
			}
		}
	}

	mutex_unlock(mutex);
	return ret;
}

static char **copy_user_pointer_array(char *input[], unsigned int count)
{
	if (!count || !input)
		return NULL;

	char **out = malloc(count * sizeof(char *));
	uint32_t *input32 = get_secure_user_ptr(input);

	for (int i = 0; i < count; i++)
		out[i] = (char *)(uint64_t)input32[i];

	return out;
}

int sys_storage_ext_mount_ps3_discfile(unsigned int filescount, char *files[])
{
	char **array = copy_user_pointer_array(files, filescount);
	if (!array)
		return EINVAL;

	umount_discfile();

	int ret = mount_ps3_discfile(filescount, array);
	free(array);
	return ret;
}

int sys_storage_ext_mount_dvd_discfile(unsigned int filescount, char *files[])
{
	char **array = copy_user_pointer_array(files, filescount);
	if (!array)
		return EINVAL;

	umount_discfile();

	int ret = mount_dvd_discfile(filescount, array);
	free(array);
	return ret;
}

int sys_storage_ext_mount_bd_discfile(unsigned int filescount, char *files[])
{

	char **array = copy_user_pointer_array(files, filescount);
	if (!array)
		return EINVAL;

	umount_discfile();

	int ret = mount_bd_discfile(filescount, array);
	free(array);
	return ret;
}

int sys_storage_ext_mount_psx_discfile(char *file, unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	file = get_secure_user_ptr(file);
	tracks = get_secure_user_ptr(tracks);

	if (!file || !tracks)
		return EINVAL;

	umount_discfile();

	return mount_psx_discfile(file, trackscount, tracks);
}

int sys_storage_ext_mount_ps2_discfile(unsigned int filescount, char *files[], unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	char **array = copy_user_pointer_array(files, filescount);
	if (!array)
		return EINVAL;

	tracks = get_secure_user_ptr(tracks);
	if (!tracks)
	{
		free(array);
		return EINVAL;
	}

	umount_discfile();

	int ret = mount_ps2_discfile(filescount, array, trackscount, tracks);
	free(array);
	return ret;
}

int sys_storage_ext_umount_discfile(void)
{
	return umount_discfile();
}

int sys_storage_ext_mount_discfile_proxy(sys_event_port_t result_port, sys_event_queue_t command_queue, int emu_type, uint64_t disc_size_bytes, uint32_t read_size, unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	process_t process;
	event_port_t proxy_result_port;
	event_queue_t proxy_command_queue;
	object_handle_t p, q;
	void *table;
	int ret;

	if(!vsh_process)
		return EINVAL;

	process = get_current_process();

	if (process != vsh_process)
		return ENOSYS;

	if (emu_type <= EMU_OFF || emu_type >= EMU_MAX || emu_type == EMU_PS2_CD || emu_type == EMU_PS2_DVD)
		return EINVAL;

	// -- AV: cd sector size
	get_cd_sector_size(trackscount);
	trackscount &= 0xff;

	//DPRINTF("CD Sector size: %i\n", cd_sector_size);
	//DPRINTF("Track count: %i\n", trackscount);

	if (emu_type == EMU_PSX)
	{
		if (trackscount >= 100 || !tracks)
			return EINVAL;
	}

	table = process->object_table;

	mutex_lock(mutex, 0);

	if (disc_emulation != EMU_OFF)
	{
		mutex_unlock(mutex);
		return EBUSY;
	}

	ret = open_shared_kernel_object(table, result_port, (void **)&proxy_result_port, &p, SYS_EVENT_PORT_OBJECT, 1);
	if (ret == 0)
	{
		ret = open_shared_kernel_object(table, command_queue, (void **)&proxy_command_queue, &q, SYS_EVENT_QUEUE_OBJECT, 1);
		if (ret == 0)
		{
			event_port_create(&proxy_command_port, EVENT_PORT_REMOTE);
			event_queue_create(&proxy_result_queue, SYNC_PRIORITY, 1, 1);

			ret = event_port_connect(proxy_command_port, proxy_command_queue);
			if (ret == 0)
			{
				ret = event_port_connect(proxy_result_port, proxy_result_queue);
				if (ret)
				{
					DPRINTF("Failed in connecting proxy result port/queue: %x\n", ret);

					event_port_disconnect(proxy_command_port);
				}
			}
			else
			{
				DPRINTF("Failed in connecting proxy command port/queue: %x\n", ret);
			}

			if (ret)
			{
				event_port_destroy(proxy_command_port);
				event_queue_destroy(proxy_result_queue);
			}

			close_kernel_object_handle(table, q);
		}

		close_kernel_object_handle(table, p);
	}
	else
	{
		DPRINTF("Cannot open even port %x (ret=%x)\n", result_port, ret);
	}

	if (ret == 0)
	{
		if (emu_type == EMU_PSX)
			discfile_proxy = malloc(sizeof(DiscFileProxy) + (trackscount * sizeof(ScsiTrackDescriptor)));
		else
			discfile_proxy = malloc(sizeof(DiscFileProxy));

		discfile_proxy->size = disc_size_bytes;
		discfile_proxy->read_size = read_size;
		discfile_proxy->cached_sector = NULL;

		if (emu_type == EMU_PSX)
		{
			tracks = get_secure_user_ptr(tracks);
			discfile_proxy->numtracks = trackscount;
			discfile_proxy->tracks = (ScsiTrackDescriptor *)(discfile_proxy + 1);
			copy_from_user(tracks, discfile_proxy->tracks, sizeof(ScsiTrackDescriptor)*trackscount);
		}
		else
		{
			discfile_proxy->numtracks = 0;
			discfile_proxy->tracks = NULL;
		}

		disc_emulation = emu_type;
		total_emulation = (!disc_being_mounted && real_disctype == 0);
	}

	mutex_unlock(mutex);
	return ret;
}

// Deprecated
/*int sys_storage_ext_mount_encrypted_image(char *image, char *mount_point, char *filesystem, uint64_t nonce)
{
	int ret;
	char loop_device[96];

	image = get_secure_user_ptr(image);
	mount_point = get_secure_user_ptr(mount_point);
	filesystem = get_secure_user_ptr(filesystem);

	if (!image)
	{
		if (encrypted_image)
		{
			map_path(mount_point, NULL, 0);
			cellFsUtilUmount(mount_point, 0, 0);
			free(encrypted_image);
			encrypted_image = NULL;
			encrypted_image_nonce = 0;
		}

		return SUCCEEDED;
	}

	if (encrypted_image)
		return EBUSY;

	ret = pathdup_from_user(image, &encrypted_image);
	if (ret)
		return ret;

	if (strlen(encrypted_image) >= 0x40)
		return EINVAL;

	encrypted_image_nonce = nonce;

	snprintf(loop_device, sizeof(loop_device), "CELL_FS_LOOP:%s", encrypted_image);
	*(uint32_t *)&loop_device[0x40] = 0;
	*(uint32_t *)&loop_device[0x44] = 0;

	ret = cellFsUtilMount_h(loop_device, filesystem, mount_point, 0, 1, 0, NULL, 0);
	if (ret)
	{
		DPRINTF("cellFsUtilMount failed: %x\n", ret);

		return ret;
	}

	map_path(mount_point, "/dev_usb000", FLAG_COPY|FLAG_PROTECT);
	return SUCCEEDED;
}*/

static INLINE void patch_ps2emu_entry(int ps2emu_type)
{
	int patch_count = 0;

	if (ps2emu_type == PS2EMU_SW)
	{
		condition_ps2softemu = 1;

		// No need to do the optical disc auth patch for ps2_netemu since there is no optical disc support
		return;
	}

	// Patch needed to support PS2 CD-R(W) and DVD+-R(W). Not necessary for isos.
	// Needed but not enough: patches at ps2 emus are necessary too!
	// Patch address may be different in different models
	/*if( (lv1_peekd(ps2emu_entry1_bc) == 0x409E00702FBE0001) && (lv1_peekd(ps2emu_entry2_bc) == 0x38800002409C0014) )
	{
		lv1_pokew(ps2emu_entry1_bc+0x10, LI(R3, 0x29));
		lv1_pokew(ps2emu_entry2_bc+0x10, LI(R3, 0x29));

	}
	else
	if( (lv1_peekd(ps2emu_entry1_semibc) == 0x409E00702FBE0001) && (lv1_peekd(ps2emu_entry2_semibc) == 0x38800002409C0014) )
	{
		lv1_pokew(ps2emu_entry1_semibc+0x10, LI(R3, 0x29));
		lv1_pokew(ps2emu_entry2_semibc+0x10, LI(R3, 0x29));

	}
	else*/
	for (u64 search_addr = 0x160000; search_addr < 0x300000; search_addr += 4)
	{
		if (lv1_peekd(search_addr) == 0x409E00702FBE0001)
		{
			//DPRINTF("PS2 auth patch at HV:%lx\n", search_addr + 0x10);
			lv1_pokew(search_addr + 0x10, LI(R3, 0x29));

			patch_count++;
		}
		else if (lv1_peekd(search_addr) == 0x38800002409C0014)
		{
			//DPRINTF("PS2 unauth patch at HV: %lx\n", search_addr + 0x10);
			lv1_pokew(search_addr + 0x10, LI(R3, 0x29));

			patch_count++;
		}

		if (patch_count == 2)
			break;
	}
}

void storage_ext_init(void)
{
	thread_t dispatch_thread;

	ps2emu_type = get_ps2emu_type();
	mutex_create(&mutex, SYNC_PRIORITY, SYNC_NOT_RECURSIVE);
	event_port_create(&command_port, EVENT_PORT_LOCAL);
	event_port_create(&result_port, EVENT_PORT_LOCAL);
	event_queue_create(&command_queue, SYNC_PRIORITY, 1, 1);
	event_queue_create(&result_queue, SYNC_PRIORITY, 1, 1);
	event_port_connect(command_port, command_queue);
	event_port_connect(result_port, result_queue);
	ppu_thread_create(&dispatch_thread, dispatch_thread_entry, 0, -0x1D8, 0x4000, 0, THREAD_NAME);
}

void storage_ext_patches(void)
{
	patch_ps2emu_entry(ps2emu_type);
	patch_jump(device_event_port_send_call, device_event);
	hook_function_on_precall_success(storage_get_device_info_symbol, post_storage_get_device_info, 2);

	// read_bdvd0 is the base function called by read_bdvd1 and read_bdvd2.
	// Hooking it would be enough for the other two to work, but anyways for reading efficiency let's hook those as well.
	hook_function_with_cond_postcall(read_bdvd0_symbol, emu_read_bdvd0, 8);
	hook_function_with_cond_postcall(read_bdvd1_symbol, emu_read_bdvd1, 4); // iso9660 driver func
	hook_function_with_cond_postcall(read_bdvd2_symbol, emu_read_bdvd2, 3);	 // udf driver func

	// High level functions
	hook_function_with_cond_postcall(storage_read_symbol, emu_storage_read, 7);
	hook_function_with_cond_postcall(get_syscall_address(SYS_STORAGE_ASYNC_READ), emu_sys_storage_async_read, 7);

	// Command functions
	hook_function_with_cond_postcall(storage_send_device_command_symbol, emu_storage_send_device_command, 7);
	hook_function_with_cond_postcall(get_syscall_address(SYS_STORAGE_ASYNC_SEND_DEVICE_COMMAND), emu_sys_storage_async_send_device_command, 7);

	// SS function
	hook_function_with_cond_postcall(get_syscall_address(SYS_SS_SEC_HW_FRAMEWORK), emu_disc_auth, 2);

	// For PS2
	patch_call(shutdown_copy_params_call, shutdown_copy_params_patched);

	// For initial setup and for psx vmode check
	hook_function_on_precall_success(cellFsUtilMount_symbol, post_cellFsUtilMount, 8);

	// For encrypted fsloop images
	//patch_call(fsloop_open_call, fsloop_open); // Deprecated
	//patch_call(fsloop_close_call, fsloop_close); // Deprecated
	//patch_call(fsloop_read_call, fsloop_read); // Deprecated
}

void unhook_all_storage_ext(void)
{
	suspend_intr();
	unhook_function_on_precall_success(storage_get_device_info_symbol, post_storage_get_device_info, 2);
	unhook_function_with_cond_postcall(read_bdvd0_symbol, emu_read_bdvd0, 8);
	unhook_function_with_cond_postcall(read_bdvd1_symbol, emu_read_bdvd1, 4);
	unhook_function_with_cond_postcall(read_bdvd2_symbol, emu_read_bdvd2, 3);
	unhook_function_with_cond_postcall(storage_read_symbol, emu_storage_read, 7);
	unhook_function_with_cond_postcall(get_syscall_address(SYS_STORAGE_ASYNC_READ), emu_sys_storage_async_read, 7);
	unhook_function_with_cond_postcall(storage_send_device_command_symbol, emu_storage_send_device_command, 7);
	unhook_function_with_cond_postcall(get_syscall_address(SYS_STORAGE_ASYNC_SEND_DEVICE_COMMAND), emu_sys_storage_async_send_device_command, 7);
	unhook_function_with_cond_postcall(get_syscall_address(864), emu_disc_auth, 2);

	//#ifdef DEBUG //Auto unload if not debug
		unhook_function_on_precall_success(cellFsUtilMount_symbol, post_cellFsUtilMount, 8);
	//#endif

	resume_intr();
}
