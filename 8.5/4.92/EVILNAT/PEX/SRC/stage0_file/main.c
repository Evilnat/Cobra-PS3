#include <lv2/lv2.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/libc.h>
#include <lv2/thread.h>
#include <lv2/patch.h>
#include <lv1/lv1.h>

#define STAGE2_MAX_SIZE         0x1FE00

#define USB_FLAG				"/dev_flash/sys/usb_flag"

#if defined (FIRMWARE_CEX)
	#define STAGE2_FILE			"/dev_blind/sys/stage2.cex"
	#define STAGE2_USB0_FILE 	"/dev_usb000/stage2.cex"
#elif defined (FIRMWARE_DEX)
	#define STAGE2_FILE			"/dev_blind/sys/stage2.dex"
	#define STAGE2_USB0_FILE 	"/dev_usb000/stage2.dex"
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

	// Check if flag "/dev_flash/sys/usb_flag" exists to load external Cobra
	if(cellFsStat(USB_FLAG, &stat) == 0)
	{	
		// Mounting /dev_usb000 (Thanks to haxxxen)
		cellFsUtilMount_Usb000(); 
		ret = cellFsStat(STAGE2_USB0_FILE, &stat);
	}

	// Found /dev_usb000/stage2.cex/dex
	if(!ret)
		stage2_file = (char *)STAGE2_USB0_FILE;

	// Mounting /dev_flash with write permission on /dev_blind
	// We don't need to check in /dev_flash, we can do it on /dev_blind, reducing the size when compiling the payload
	cellFsUtilMount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0, 0, 0, 0, 0);

	// Load stage2
	if(cellFsStat(stage2_file, &stat) == 0)
	{
		// Avoid loading an empty stage2
		if(stat.st_size != 0 && stat.st_size < STAGE2_MAX_SIZE)
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

	// stage2 fail save by bguerville / AV
	// Disabling to avoid semibrick on NAND, will be enabled in stage2
	if(stage2 && ret)
		cellFsRename(STAGE2_FILE, STAGE2_FILE ".bak");	

	f.toc = (void *)MKA(TOC);
	f.addr = stage2 ? stage2 : (void *)MKA(0x17e0);
		
	func = (void *)&f;	
	func();
}
