#include <stddef.h>

#include <lv2/lv2.h>
#include <debug.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/libc.h>
#include <lv2/thread.h>
#include <lv2/patch.h>

#include <lv1/lv1.h>


#ifdef DEBUG
#define DPRINTF		_debug_printf
#define DPRINT_HEX	debug_print_hex
#else
#define DPRINTF(...)
#define DPRINT_HEX(a, b)
#endif

#define STAGE2_FILE	"/dev_flash/sys/stage2.bin"

//void flash_mount_clone(void);

int main(void)
{
	void *stage2 = NULL;
	
	f_desc_t f;
	int (* func)(void);	
	int ret=0;
	
#ifdef DEBUG		
	debug_init();
	DPRINTF("Stage 1 hello.\n");	
#endif

	if(ret==0)
	{
		CellFsStat stat;
		
		DPRINTF("Flash mounted\n");
				
		if (cellFsStat(STAGE2_FILE, &stat) == 0)
		{
			int fd;
			
			if (cellFsOpen(STAGE2_FILE, CELL_FS_O_RDONLY, &fd, 0, NULL, 0) == 0)
			{
				uint32_t psize = stat.st_size;
				
				DPRINTF("Payload size = %d\n", psize);
				
				stage2 = alloc(psize, 0x27);
				if (stage2)
				{
					uint64_t rs;
					
					if (cellFsRead(fd, stage2, psize, &rs) != 0)
					{
						DPRINTF("Stage 2 read fail.\n");
						dealloc(stage2, 0x27);
						stage2 = NULL;
					}
				}
				else				
					DPRINTF("Cannot allocate stage2\n");				
				
				cellFsClose(fd);
			}
		}
		else		
			DPRINTF("There is no stage 2, booting system.\n");		
	}
	
	if (stage2)
	{
		f.toc = (void *)MKA(TOC);
		f.addr = stage2;			
		func = (void *)&f;	
		DPRINTF("Calling stage 2...\n");
		func();
	}
	
	return ret;
}
