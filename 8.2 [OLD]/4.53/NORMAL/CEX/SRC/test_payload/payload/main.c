#include <lv2/lv2.h>
#include <lv2/memory.h>
#include <lv2/io.h>
#include <lv2/libc.h>
#include <lv2/thread.h>
#include <lv2/patch.h>
#include <lv1/lv1.h>

void main(void)
{
	int file;
	uint64_t size;
	uint8_t buf[5] = {0x53,0x43,0x45,0x00};
	if (cellFsOpen("/dev_hdd0/test_payload_in", CELL_FS_O_WRONLY|CELL_FS_O_CREAT|CELL_FS_O_TRUNC, &file, 0666, NULL, 0) == 0)
	{
		cellFsWrite(file, buf, 4, &size);
		cellFsClose(file);
	}
}
