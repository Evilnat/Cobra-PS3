/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __PS2EMU_SYMBOLS_H_S__
#define __PS2EMU_SYMBOLS_H_S__

#if defined(FIRMWARE_CEX)

#if defined(PS2HWEMU)

#define TOC									0x4EB8C0 

#define cdvd_send_atapi_command_symbol		0x22138 

#define ufs_open_symbol						0x4E0A8 
#define ufs_close_symbol					0x4E834 
#define ufs_read_symbol						0x4DB78 
#define ufs_write_symbol					0x4DC6C
#define ufs_fstat_symbol					0x4EB38

#define printf_symbol						0x3BC4

#define zeroalloc_symbol					0xFDFEC
#define malloc_symbol						0xFDF04
#define free_symbol							0xFE044
#define memcpy_symbol						0xDFAC
#define memset_symbol						0xD488
#define strcpy_symbol						0xED04
#define strncpy_symbol						0xED30
#define strcat_symbol						0xEED0
#define strlen_symbol						0xECD8
#define strcmp_symbol						0xED94
#define strncmp_symbol						0xEDE8
#define strchr_symbol						0xEE50
#define strrchr_symbol						0xEE8C

#define vuart_read_symbol					0x1E42C
#define vuart_write_symbol					0x1E37C

#define ps2_disc_auth_symbol				0x206B0
#define ps2_disc_auth_caller_symbol			0x512F8

#define overwritten_symbol					0x23D6C

#define arguments_symbol					0x500D68

#elif defined(PS2GXEMU) 

#define TOC									0x678548

#define cdvd_read_symbol					0x8E4AC
#define cdvd_send_atapi_command_symbol		0x8DA80
#define cdvd_send_device_command_symbol		0x8D994

#define ufs_open_symbol						0x24EBB0
#define ufs_close_symbol					0x24F358
#define ufs_read_symbol						0x24E8C0
#define ufs_write_symbol					0x24EA38
#define ufs_fstat_symbol					0x24E700

#define log_printf_symbol					0x1E499C

#define zeroalloc_symbol					0x1E48EC
#define malloc_symbol						0x1E488C
#define free_symbol							0x1E4938
#define memcpy_symbol						0x514BC
#define memset_symbol						0x51528
#define snprintf_symbol						0x52360
#define strcpy_symbol						0x51600
#define strncpy_symbol						0x99C10
#define strcat_symbol						0x51634
#define strlen_symbol						0x515C8
#define strcmp_symbol						0x99C3C
#define strchr_symbol						0x99B9C
#define strrchr_symbol						0x99BD0

#define vuart_read_symbol					0x49544
#define vuart_write_symbol					0x494A4

#define ps2_disc_auth_symbol				0x900FC
#define ps2_disc_auth_caller_symbol			0x24F95C

#define overwritten_symbol					0x8CAD8

/* Calls */
// #define reboot_parameters_vuart_call		0x58C50

/* Vars */
// #define arguments_symbol					0x6E7298

#elif defined(PS2NETEMU) 

#define TOC                					0x751280

#define cdvd_read_symbol   					0x1377C4 
#define ufs_open_symbol    					0x1ECC5C 
#define ufs_close_symbol   					0x1ECB10 
#define ufs_read_symbol    					0x1ED5CC 
#define ufs_write_symbol   					0x1ED458 
#define ufs_fstat_symbol   					0x1EC760 

#define decrypt_symbol						cdvd_read_symbol

#define log_printf_symbol  					0xB8E30 

#define memcpy_symbol      					0x118A90 
#define memset_symbol      					0x118A3C 
#define memcmp_symbol      					0x118860 
#define memchr_symbol      					0x1E6DD8 
#define snprintf_symbol    					0x119724 
#define strcpy_symbol      					0x1188DC 
#define strncpy_symbol     					0x1EB640 
#define strcat_symbol      					0x118914 
#define strlen_symbol      					0x1188A4 
#define strcmp_symbol      					0x1EB670 
#define strncmp_symbol     					0x1EB6F4 
#define strchr_symbol      					0x1EB5C8 
#define strrchr_symbol     					0x1EB5FC 

#endif 

#endif  /* FIRMWARE */

#endif /* __PS2EMU_SYMBOLS_H_S__ */
