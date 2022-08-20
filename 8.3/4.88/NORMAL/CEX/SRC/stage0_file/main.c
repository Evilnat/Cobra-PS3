#include <lv2/lv2.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/libc.h>
#include <lv2/thread.h>
#include <lv2/patch.h>
#include <lv1/lv1.h>

#define CELL_OK  0

#define FLAG_NOCOBRA	"/dev_usb000/no_cobra"
#define STAGE2_USB0		"/dev_usb000/stage2.bin"
#define STAGE2_FILE		"/dev_flash/sys/stage2.bin"
#define STAGE2_FAIL		"/dev_blind/sys/stage2.bin"

void main(void)
{
	char *stage2_file = (char *)STAGE2_FILE;
	void *stage2 = NULL;
		
	f_desc_t f;
	int (* func)(void);	

	CellFsStat stat;
	int fd;
	uint64_t rs;

	for (int i = 0; i < 128; i++)
	{
		uint64_t pte0 = *(uint64_t *)(MKA(0xf000000 | (i << 7)));
		uint64_t pte1 = *(uint64_t *)(MKA(0xf000000 | ((i << 7) + 8)));
		
		lv1_write_htab_entry(0, i << 3, pte0, (pte1 & 0xff0000) | 0x190);
	}
	
	// use external stage2.bin if the file exists
	int ret = cellFsUtilMount_Usb000(); // mount dev_usb000
	if (ret == CELL_OK)
	{
		if (cellFsStat(FLAG_NOCOBRA, &stat) == CELL_OK)
			stage2_file = (char *)FLAG_NOCOBRA;
		else if (cellFsStat(STAGE2_USB0, &stat) == CELL_OK)
			stage2_file = (char *)STAGE2_USB0;
		else
			ret = -1; // use fail safe
	}
	
	// load stage2
	if (cellFsStat(stage2_file, &stat) == CELL_OK)
	{
		// Avoid loading an empty stage2 or with a size greater than 0x1FE00
		if(stat.st_size > 0 && stat.st_size < 0x1FE00)
		{
			if (cellFsOpen(stage2_file, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == CELL_OK)
			{
				stage2 = alloc(stat.st_size, 0x27);

				if(stage2)
				{		
					if (cellFsRead(fd, stage2, stat.st_size, &rs) != CELL_OK)
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
	
	if (stage2)		
	{
		// stage2 fail save by bguerville / AV
		cellFsUtilMount_h("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0, 0, 0, 0, 0);
		if (ret) cellFsRename(STAGE2_FAIL, STAGE2_FAIL ".bak");
		f.addr = stage2;	
	}
	else	
		f.addr = (void *)MKA(0x17e0);	
		
	func = (void *)&f;	
	func();
}
