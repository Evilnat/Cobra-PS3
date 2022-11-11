#include <lv2/lv2.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/libc.h>
#include <lv2/thread.h>
#include <lv2/patch.h>
#include <lv1/lv1.h>

#define STAGE2_FILE		"/dev_flash/rebug/cobra/stage2.cex"
#define FLAG_NOCOBRA	"/dev_usb000/no_cobra"
#define STAGE2_USB0		"/dev_usb000/stage2.cex"

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

	// Use external stage2.bin if the file exists
	if (cellFsUtilMount_Usb000() == 0) // Mount dev_usb000
	{
		if (cellFsStat(STAGE2_USB0, &stat) == 0)
			stage2_file = (char *)STAGE2_USB0;
	}

	// Check if exists no_cobra flag in external device
	if (cellFsStat(FLAG_NOCOBRA, &stat) != 0)
	{
		// Load stage2
		if (cellFsStat(stage2_file, &stat) == 0)
		{
			// Avoid loading an empty stage2 or with a size higher than 0x1FE00
			if(stat.st_size != 0 && stat.st_size < 0x1FE00)
			{
				if (cellFsOpen(stage2_file, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == 0)
				{
					stage2 = alloc(stat.st_size, 0x27);

					if(stage2)
					{		
						if (cellFsRead(fd, stage2, stat.st_size, &rs) != 0)
						{
							dealloc(stage2, 0x27);
							stage2 = NULL;
						}						
					}				
					
					cellFsClose(fd);
				}
			}
		}
	}

	f.toc = (void *)MKA(TOC);
	
	if(stage2)		
		f.addr = stage2;
	else	
		f.addr = (void *)MKA(0x17e0);	
		
	func = (void *)&f;	
	func();
}
