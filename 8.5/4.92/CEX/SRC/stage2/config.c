#include <lv2/lv2.h>
#include <lv2/error.h>
#include <lv2/libc.h>
#include <lv2/io.h>
#include <lv2/memory.h>
#include "common.h"
#include "config.h"
#include "region.h"
#include "modulespatch.h"
#include "fan_control.h"
#include "homebrew_blocker.h"
#include "make_rif.h"
#include "storage_ext.h"

#define COBRA_CONFIG_FILE	"/dev_hdd0/vm/cobra_cfg.bin"

CobraConfig config;

static uint8_t cfg_template[0x15] = 
{ 
	0x00, 0x15, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void check_and_correct(CobraConfig *cfg)
{
	uint32_t i;
	int found = 0;
	
	for (i = 1; i <= BD_VIDEO_REGION_MAX; i *= 2)
	{
		if (cfg->bd_video_region == i)
		{
			found = 1;
			break;
		}
	}
	
	if (!found)
		cfg->bd_video_region = 0;
	else
		found = 0;
	
	for (i = 1; i <= DVD_VIDEO_REGION_MAX; i *= 2)
	{
		if (cfg->dvd_video_region == i)
		{
			found = 1;
			break;
		}
	}
	
	if (!found)
		cfg->dvd_video_region = 0;	
	
	if (cfg->ps2softemu > 1)
		cfg->ps2softemu = 0;
	
	// Disabled
	/*if (cfg->spoof_version > MAX_SPOOF_VERSION)	
		cfg->spoof_version = 0;	
	else 
	{
		uint8_t h, l;
		
		h = (cfg->spoof_version & 0xFF) >> 4;
		l = cfg->spoof_version & 0xF;
		
		if (h > 9 || l > 9)		
			cfg->spoof_version = 0;		
	}*/

	// Disabled
	/*if (cfg->spoof_revision > MAX_SPOOF_REVISION)
		cfg->spoof_revision = 0;*/

	cfg->spoof_version = 0;

	cfg->spoof_revision = 0;	

	/*if(cfg->allow_restore_sc > 1)
		cfg->allow_restore_sc = 0;*/

	/*if(cfg->skip_existing_rif > 1)
		cfg->skip_existing_rif = 0;*/

	/*if(cfg->color_disc > 1)
		cfg->color_disc = 0;*/

	if(cfg->syscalls_mode > 2)
		cfg->syscalls_mode = 0;	

	if(cfg->gameboot_mode > 1)
		cfg->gameboot_mode = 0;	

	/*if(cfg->epilepsy_warning > 1)
		cfg->epilepsy_warning = 0;*/

	if(cfg->coldboot_mode > 1)
		cfg->coldboot_mode = 0;		

	if(cfg->hidden_trophy_mode > 1)
		cfg->hidden_trophy_mode = 1;

	if(cfg->ps2_speed > 1 && cfg->ps2_speed < 0x60)
		cfg->ps2_speed = 0;
		
	if (cfg->size > sizeof(CobraConfig))
		cfg->size = sizeof(CobraConfig);

	// Optimized by aldostools
	bd_video_region = config.bd_video_region;
	dvd_video_region = config.dvd_video_region;
	fan_speed = config.fan_speed;
	ps2_speed = config.ps2_speed;
	//allow_restore_sc = config.allow_restore_sc;
	//skip_existing_rif = config.skip_existing_rif;
	//color_disc = config.color_disc;
	syscalls_mode = config.syscalls_mode;
	gameboot_mode = config.gameboot_mode;
	//epilepsy_warning = config.epilepsy_warning;
	coldboot_mode = config.coldboot_mode;
	hidden_trophy_mode = config.hidden_trophy_mode;
}

static uint16_t checksum(CobraConfig *cfg)
{
	uint8_t *buf = &cfg->bd_video_region;
	uint16_t size = cfg->size - sizeof(cfg->size) - sizeof(cfg->checksum);
	uint16_t cs = 0;
	
	for (int i = 0; i < size; i++)	
		cs += buf[i];	
	
	return cs;
}
 
int read_cobra_config(void)
{
	int fd, ret;
	uint64_t write;
	CellFsStat stat;

	// Create "/dev_hdd0/vm/cobra_cfg.bin" file if it does not exist
	// This fixes issues in some PS3 that can't create/save Cobra config
	ret = cellFsStat(COBRA_CONFIG_FILE, &stat);
	if(ret != CELL_FS_SUCCEEDED || stat.st_size != 0x15)
	{
		if(cellFsOpen(COBRA_CONFIG_FILE, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			cellFsWrite(fd, cfg_template, 0x15, &write);
			cellFsClose(fd);
		}		
	}
	
	if(cellFsOpen(COBRA_CONFIG_FILE, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == 0)
	{
		uint64_t r;
		
		cellFsRead(fd, &config, sizeof(config), &r);
		cellFsClose(fd);
	}
	
	// Removed. 
	// Cobra config size has different size and maybe conflicts with legacy apps
	/*if (config.size > 4096 || checksum(&config) != config.checksum)	
		memset(&config, 0, sizeof(config));		
	else	*/

	check_and_correct(&config);	
	
	config.size = sizeof(config);

	// Removed. Now condition_ps2softemu has another meaning and it is set automatically in storage_ext if no BC console
	//condition_ps2softemu = config.ps2softemu;

	// Disabled to reduce stage2 size
	/*DPRINTF("Configuration read.\n+ bd_video_region = %d\n+ dvd_video_region = %d\n+ fan_speed = %02X\n+ ps2_speed = %02X\n+ syscalls_mode = %02X\n+ gameboot_mode = %02X\n+ coldboot_mode = %02X\n+ hidden_trophy_mode = %02X\n\n",
		bd_video_region, dvd_video_region, fan_speed, ps2_speed, syscalls_mode, gameboot_mode, coldboot_mode, hidden_trophy_mode);*/
	
	return SUCCEEDED;
}

int write_cobra_config(void)
{
	int fd;
	uint64_t w;
	int ret = 0;
	
	if (cellFsOpen(COBRA_CONFIG_FILE, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, 0666, NULL, 0) != 0)	
		return ENODEV;	
	
	if (cellFsWrite(fd, &config, sizeof(config), &w) != 0)
		ret = ENODEV;
	
	cellFsClose(fd);	
	return ret;
}

int sys_read_cobra_config(CobraConfig *cfg)
{
	int erase_size, copy_size;
	
	cfg = get_secure_user_ptr(cfg);
	
	if (cfg->size > 4096)
		return EINVAL;	
	
	erase_size = cfg->size - sizeof(config.size);
	if (erase_size < 0)
		erase_size = 0;
	
	memset(&cfg->checksum, 0, erase_size);
	
	copy_size = ((cfg->size > config.size) ? config.size : cfg->size) - sizeof(config.size);
	if (copy_size < 0)
		copy_size = 0;
	
	memcpy(&cfg->checksum, &config.checksum, copy_size);
	return SUCCEEDED;
}

int sys_write_cobra_config(CobraConfig *cfg)
{
	int copy_size;
	
	cfg = get_secure_user_ptr(cfg);
	
	if (cfg->size > 4096)
		return EINVAL;	
	
	check_and_correct(cfg);
	
	cfg->checksum = checksum(cfg);

	copy_size = cfg->size - sizeof(config.size);	
	if (copy_size < 0)
		copy_size = 0;
	
	memcpy(&config.checksum, &cfg->checksum, copy_size);
	
	return write_cobra_config();
}
