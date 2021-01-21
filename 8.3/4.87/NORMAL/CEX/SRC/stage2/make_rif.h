#ifndef __MAKE_RIF_H__
#define __MAKE_RIF_H__

#define IDPS_KEYBITS 	128
#define ACT_DAT_KEYBITS 128
#define RIF_KEYBITS 	128
#define RAP_KEYBITS 	128

//////// make_rif memory allocation ////////////
#define ALLOC_ACT_DAT	 	(uint8_t*)(make_rif_buf)
#define ALLOC_CONTENT_ID	(char*)(make_rif_buf + 0x20)
#define ALLOC_PATH_BUFFER	(char*)(make_rif_buf + 0x20 + 0x28)
#define ALLOC_RIF_BUFFER 	(uint8_t*)(make_rif_buf + 0x20 + 0x28)
////////////////////////////////////////////////

extern unsigned char RAP_KEY[];
extern unsigned char RAP_PBOX[];
extern unsigned char RAP_E1[];
extern unsigned char RAP_E2[];

uint8_t make_rif_buf[0x20 + 0x28 + 0x50 + 0x20 + 0x28]; // ACT_DAT[0x20] + CONTENT_ID[0x28] + RAP_PATH[0x50] + RIF_BUFFER[0x20] (rif_buffer reuse rap_path + 0x20 = 0x70)+0x28(signaturs)

void read_act_dat_and_make_rif(uint8_t *rap, uint8_t *act_dat, const char *content_id, const char *rif_path);
void make_rif(const char *path);

#endif /* __MAKE_RIF_H__ */
