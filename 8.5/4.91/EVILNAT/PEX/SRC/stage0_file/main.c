#include <lv2/lv2.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/libc.h>
#include <lv2/thread.h>
#include <lv2/patch.h>
#include <lv1/lv1.h>

#define STAGE2_FLASH			"/dev_flash/sys/stage2"
#define STAGE2_BLIND			"/dev_blind/sys/stage2"
#define STAGE2_USB0				"/dev_usb000/stage2"
#define USB_FLAG				"/dev_flash/sys/usb_flag"

#if defined (FIRMWARE_CEX)
	#define STAGE2_FILE			STAGE2_FLASH ".cex"
	#define STAGE2_DEVBLIND		STAGE2_BLIND ".cex"
	#define STAGE2_USB0_FILE 	STAGE2_USB0  ".cex"
#elif defined (FIRMWARE_DEX)
	#define STAGE2_FILE			STAGE2_FLASH ".dex"
	#define STAGE2_DEVBLIND		STAGE2_BLIND ".dex"
	#define STAGE2_USB0_FILE 	STAGE2_USB0  ".dex"
#endif

void main(void)
{
	void *stage2 = NULL;
	char *stage2_file = (char *)STAGE2_FILE;
	
	f_desc_t f;
	int (* func)(void);	

	CellFsStat stat;
	int fd, ret = 1; // use fail safe
	uint64_t rs;

	for (int i = 0; i < 128; i++)
	{
		uint64_t pte0 = *(uint64_t *)(MKA(0xf000000 | (i << 7)));
		uint64_t pte1 = *(uint64_t *)(MKA(0xf000000 | ((i << 7) + 8)));		
		lv1_write_htab_entry(0, i << 3, pte0, (pte1 & 0xff0000) | 0x190);
	}

	// Check if usb_flag exists in /dev_flash
	if(cellFsStat(USB_FLAG, &stat) == 0)
	{
		// Mounting /dev_usb000 (Thanks to haxxxen)
		ret = cellFsUtilMount_Usb000();

		if(ret == 0)
		{
			// Use external stage2.bin if the file exists
			if(cellFsStat(STAGE2_USB0_FILE, &stat) == 0)
				stage2_file = (char *)STAGE2_USB0_FILE;
			else
				ret = 1; // use fail safe
		}
	}

	// Load stage2
	if(cellFsStat(stage2_file, &stat) == 0)
	{
		// Avoid loading an empty stage2 or with a size higher than 0x1FE00
		if(stat.st_size != 0 && stat.st_size < 0x1FE00)
		{
			if(cellFsOpen(stage2_file, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == 0)
			{
				stage2 = alloc(stat.st_size, 0x27);

				if(stage2)
				{		
					if(cellFsRead(fd, stage2, stat.st_size, &rs) != 0)
					{
						dealloc(stage2, 0x27);
						stage2 = NULL;
					}						
				}	

				cellFsClose(fd);
			}
		}
	}

	f.toc = (void *)MKA(TOC);
	
	if(stage2)		
	{
		if(ret)
		{
			// stage2 fail save by bguerville / AV
			// Disabling to avoid semibrick on NAND, will be enabled in stage2
			cellFsUtilMount_h("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0, 0, 0, 0, 0);
			cellFsRename(STAGE2_DEVBLIND, STAGE2_DEVBLIND ".bak");
		}

		f.addr = stage2;	
	}
	else	
		f.addr = (void *)MKA(0x17e0);	
		
	func = (void *)&f;	
	func();
}
