#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MKA(a)	(0x8000000000000000ULL | a) 
#define MAKE_JUMP_VALUE(addr, to) ((0x12 << 26) | ((((to-(uint64_t)(addr))>>2) & 0xFFFFFF) << 2))
#define MAKE_CALL_VALUE(addr, to) ((0x12 << 26) | ((((to-(uint64_t)(addr))>>2) & 0xFFFFFF) << 2)) | 1

static uint32_t swap32(uint32_t data)
{
	uint32_t ret = (data << 24) & 0xff000000;
	ret |= (data << 8) & 0x00ff0000;
	ret |= (data >> 8) & 0x0000ff00;
	ret |= (data >> 24) & 0x000000ff;
	return ret;
}

static uint64_t swap64(uint64_t data)
{
	uint64_t ret = (data << 56) & 0xff00000000000000ULL;
	ret |= ((data << 40) & 0x00ff000000000000ULL);
	ret |= ((data << 24) & 0x0000ff0000000000ULL);
	ret |= ((data << 8) & 0x000000ff00000000ULL);
	ret |= ((data >> 8) & 0x00000000ff000000ULL);
	ret |= ((data >> 24) & 0x0000000000ff0000ULL);
	ret |= ((data >> 40) & 0x000000000000ff00ULL);
	ret |= ((data >> 56) & 0x00000000000000ffULL);
	return ret;
}

static void command3(char *cmd, char *arg1, char *arg2, char *arg3)
{
	char buf[2048];
	
	snprintf(buf, sizeof(buf), "%s %s %s %s", cmd, arg1, arg2, arg3);
	system(buf);
}

static void command8(char *cmd, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5, char *arg6, char *arg7, char *arg8)
{
	char buf[2048];
	
	snprintf(buf, sizeof(buf), "%s %s %s %s %s %s %s %s %s", cmd, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	system(buf);
}

int main(int argc, char **argv)
{
	if(argc != 5)
	{
		printf("usage:kernel payload stage0_addr nocfw_kern_plugin");
		exit(0);
	}

	command3("scetool", "--decrypt", argv[1], "lv2_kernel.elf");	
	FILE *kernel = fopen("lv2_kernel.elf", "rb");

	if(!kernel)
	{
		printf("no kernel found");
		exit(0);
	}

	fseek(kernel, 0, SEEK_END);
	
	int kernel_len = ftell(kernel);
	fseek(kernel, 0, SEEK_SET);
	uint8_t *kernel_buf = (uint8_t *)malloc(kernel_len + 1);
	fread(kernel_buf, kernel_len, 1, kernel);
	
	FILE *plugin = fopen(argv[4], "rb");
	if(!plugin)
	{
		printf("no plugin found!");
		exit(0);
	}

	fseek(plugin, 0, SEEK_END);
	
	int plugin_len = ftell(plugin);
	printf("plugin_len:%x\n", plugin_len);

	fseek(plugin, 0, SEEK_SET);
	uint8_t *plugin_buf = (uint8_t *)malloc(plugin_len);	
	fread(plugin_buf, plugin_len, 1, plugin);
	fclose(plugin);

	memcpy(kernel_buf + 0x10000 + 0x17e0, plugin_buf, plugin_len);
	free(plugin_buf);

	FILE *payload = fopen(argv[2], "rb");
	if(!payload)
	{
		printf("no payload found!");
		exit(0);
	}
	
	fseek(payload, 0, SEEK_END);
	
	int payload_len = ftell(payload);
	printf("payload_len:%x\n", payload_len);

	fseek(payload, 0, SEEK_SET);
	
	uint8_t *buf = (uint8_t *)malloc(payload_len);
	fread(buf, payload_len, 1, payload);
	uint32_t stage0_addr = 0;
	sscanf(argv[3], "0x%x", &stage0_addr);
	uint32_t inst = MAKE_JUMP_VALUE(stage0_addr, 0x1000);
 	inst = swap32(inst);
	memcpy(kernel_buf + 0x10000 + stage0_addr, &inst, 0x4);
	
	uint64_t opcode = 0x386000014e800020;
	opcode = swap64(opcode);
	memcpy(kernel_buf + 0x10000 + 0x3d90, &opcode, 0x8);
	memcpy(kernel_buf + 0x11000, buf, payload_len);
	
	FILE *tmp = fopen("temp.elf", "wb");
	fwrite(kernel_buf, kernel_len,1,tmp);
	fclose(tmp);
	free(kernel_buf);
	free(buf);
	
	command8("scetool", "--template", argv[1], "--sce-type=SELF", "--compress-data=TRUE", "--self-type=LV2", "--encrypt", "temp.elf", "lv2_kernel.self.cobra.self");
	
	remove("temp.elf");
	remove("lv2_kernel.elf");
	
	return 0;
}
	