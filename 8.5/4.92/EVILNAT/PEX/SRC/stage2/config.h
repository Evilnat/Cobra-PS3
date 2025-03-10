#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_SPOOF_VERSION			0x0999
#define MAX_SPOOF_REVISION			99999
#define MAX_SPOOF_REVISION_CHARS	5

typedef struct
{
	uint16_t size;
	uint16_t checksum;
	uint8_t bd_video_region;
	uint8_t dvd_video_region;
	uint8_t ps2softemu;
	uint32_t spoof_version;
	uint32_t spoof_revision;
	uint8_t fan_speed; 		    // 0 = Disabled | 1 = SYSCON | Dynamic Fan Controller (2 = Max 60°C | 3 = Max 65°C | 4 = Max 70°C | 5 = Max 75°C) | 0x33 to 0xFF = Manual
	uint8_t ps2_speed;		    // 0 = Disabled | 1 = SYSCON | 0x60 | 0x65 | 0x70 | 0x75 | 0x80 | 0x85 | 0x90
	uint8_t allow_restore_sc;   // 0 = Does not allow to restore CFW syscalls | 1 = Allow to restore CFW syscalls 
	uint8_t skip_existing_rif;  // 0 = Does not skip if .rif already exists | 1 = Skip if .rif already exists
	uint8_t color_disc; 	    // 0 = Default (PS2 DVD: yellow disc icon - PS2 CD: blue disc icon) | 1 = PS2 CD/DVD: blue disc icon
	uint8_t syscalls_mode;      // 0 = CFW syscalls are enabled on boot (Default) | 1 = Fully disable CFW syscalls on boot | 2 = Keep PS3M_API Features only
	uint8_t gameboot_mode; 	    // 0 = Disabled (Default) | 1 = Enabled
	uint8_t epilepsy_warning;   // 0 = Disabled (Default) | 1 = Enabled
	uint8_t coldboot_mode; 	    // 0 = Enabled (Default)  | 1 = Disabled 
	uint8_t hidden_trophy_mode; // 0 = Enabled (Default)  | 1 = Disabled (Will show hidden trophy data)
} __attribute__((packed)) CobraConfig;

extern CobraConfig config;

int read_cobra_config(void);
int save_cobra_config(void);

// Syscalls
int sys_read_cobra_config(CobraConfig *cfg);
int sys_write_cobra_config(CobraConfig *cfg);

#endif /* __CONFIG_H__ */


