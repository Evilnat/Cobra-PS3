#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define SWAP32(x) ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

#define MAKE_JUMP_VALUE(addr, to) ((0x12 << 26) | ((((to-(uint64_t)(addr))>>2)&0xFFFFFF) << 2))
#define MAKE_CALL_VALUE(addr, to) ((0x12 << 26) | ((((to-(uint64_t)(addr))>>2)&0xFFFFFF) << 2)) | 1

#define ADDITIONAL_SIZE		0x10000

typedef struct 
{
	uint32_t	p_type;		/* entry type */
	uint32_t	p_flags;	/* entry flags */
	uint64_t	p_offset;	/* file offset */
	uint64_t	p_vaddr;	/* virtual address */
	uint64_t	p_paddr;	/* physical address */
	uint64_t	p_filesz;	/* file size */
	uint64_t	p_memsz;	/* memory size */
	uint64_t	p_align;	/* memory/file alignment */
} __attribute__((packed)) Elf64_Phdr;

typedef struct 
{
	uint32_t	sh_name;		/* section name */
	uint32_t	sh_type;		/* SHT_... */
	uint64_t	sh_flags;		/* SHF_... */
	uint64_t	sh_addr;		/* virtual address */
	uint64_t	sh_offset;		/* file offset */
	uint64_t	sh_size;		/* section size */
	uint32_t	sh_link;		/* misc info */
	uint32_t	sh_info;		/* misc info */
	uint64_t	sh_addralign;	/* memory alignment */
	uint64_t	sh_entsize;		/* entry size if table */
} __attribute__((packed)) Elf64_Shdr ;

#define	SHF_WRITE				0x01 	/* sh_flags */
#define	SHF_ALLOC				0x02
#define	SHF_EXECINSTR			0x04
#define	SHF_MERGE				0x10
#define	SHF_STRINGS				0x20
#define	SHF_INFO_LINK			0x40
#define	SHF_LINK_ORDER			0x80
#define	SHF_OS_NONCONFORMING	0x100
#define	SHF_GROUP				0x200
#define	SHF_TLS					0x400

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

static void patch_self(char *src, char *dst, uint32_t sh_offset, uint32_t data_address, int extend_ph, int extend_sh)
{
	char *buf;
	FILE *f;
	
	f = fopen(src, "rb");
	if (!f)
	{
		printf("Cannot open %s\n", src);
		exit(-1);
	}
	
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	buf = malloc(size);
	
	fread(buf, 1, size, f);
	fclose(f);
	
	Elf64_Phdr *phdr = (Elf64_Phdr *)(buf + 0xD0);
	
	if (extend_ph)	
		phdr->p_memsz = swap64(swap64(phdr->p_memsz) + ADDITIONAL_SIZE);
	
	Elf64_Shdr *shdr = (Elf64_Shdr *)(buf + sh_offset);
	
	while (1)
	{
		uint64_t flags = swap64(shdr->sh_flags);
		
		if (swap64(shdr->sh_addr) == data_address)
		{
			// Grant execution on data
			shdr->sh_flags = swap64(flags | SHF_EXECINSTR); 
			if (extend_sh)
				shdr->sh_size = swap64(swap64(shdr->sh_size) + ADDITIONAL_SIZE);
			break;
		}
		
		if (flags & SHF_EXECINSTR)
		{
			// Write permissions on code
			shdr->sh_flags = swap64(flags | SHF_WRITE);
		}
		
		shdr++;
	}
	
	f = fopen(dst, "wb");
	fwrite(buf, 1, size, f);
	fclose(f);
}

int main(int argc, char *argv[])
{
	char *payload, *self_input, *self_output;
	uint32_t text_offset, payload_addr, payload_size, sh_addr, data_addr;
	uint32_t patch_addr, patch_data;
	FILE *bin, *elf;
	uint8_t *payload_buf;
	
	if (argc < 10)
	{
		printf("Usage: %s payload self_input self_output text_offset payload_addr sh_addr data_addr patch_addr patch_data\n", argv[0]);
		return -1;
	}
	
	payload = argv[1];
	self_input = argv[2];
	self_output = argv[3];
	sscanf(argv[4], "0x%x", &text_offset);
	sscanf(argv[5], "0x%x", &payload_addr);
	sscanf(argv[6], "0x%x", &sh_addr);
	sscanf(argv[7], "0x%x", &data_addr);
	sscanf(argv[8], "0x%x", &patch_addr);
	sscanf(argv[9], "0x%x", &patch_data);
	
	patch_self(self_input, "temp.self", sh_addr, data_addr, (patch_addr != 0xFFFFFFFF) && (patch_addr != 0xFFFFFFFE), (patch_addr != 0xFFFFFFFF));
	
	command3("scetool", "--decrypt", "temp.self", "temp.elf");	
	
	bin = fopen(payload, "rb");
	if (!bin)
	{
		printf("Cannot open %s\n", payload);
		return -2;
	}
	
	fseek(bin, 0, SEEK_END);
	payload_size = ftell(bin);
	fseek(bin, 0, SEEK_SET);
	
	payload_buf = malloc(payload_size);
	if (!payload_buf)
	{
		printf("Cannot allocate %d bytes.\n", payload_size);
		return -3;
	}
	
	fread(payload_buf, 1, payload_size, bin);
	fclose(bin);
	
	elf = fopen("temp.elf", "rb+");
	if (!elf)
	{
		printf("Cannot open temp.elf\n");
		return -4;
	}
	
	fseek(elf, text_offset + payload_addr, SEEK_SET);
	fwrite(payload_buf, 1, payload_size, elf);
	
	if (patch_addr != 0 && patch_addr != 0xffffffff && patch_addr != 0xfffffffe)
	{
		fseek(elf, text_offset + patch_addr, SEEK_SET);
		patch_data = SWAP32(patch_data);
		fwrite(&patch_data, 1, 4, elf);
	}
	
	fclose(elf);

	command8("scetool", "--template", self_input, "--sce-type=SELF", "--compress-data=TRUE", "--self-type=LV2", "--encrypt", "temp.elf", self_output);
		
	return 0;
}
