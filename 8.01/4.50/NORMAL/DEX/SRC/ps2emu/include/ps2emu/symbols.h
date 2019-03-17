/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __PS2EMU_SYMBOLS_H_S__
#define __PS2EMU_SYMBOLS_H_S__

#if defined(FIRMWARE_4_50DEX)

#if defined(PS2HWEMU)

#define TOC 								0x4EB840

#define cdvd_send_atapi_command_symbol		0x220B8

#define ufs_open_symbol						0x4E028
#define ufs_close_symbol					0x4E7B4
#define ufs_read_symbol						0x4DAF8
#define ufs_write_symbol					0x4DBEC
#define ufs_fstat_symbol					0x4EAB8

#define printf_symbol						0x3BC4

#define zeroalloc_symbol					0xFDF6C
#define malloc_symbol						0xFDE84
#define free_symbol							0xFDFC4
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

#define ps2_disc_auth_symbol				0x20630
#define ps2_disc_auth_caller_symbol			0x51278

#define overwritten_symbol					0x23CEC

#define arguments_symbol					0x4FED68

#elif defined(PS2GXEMU)

#define TOC 								0x678548

#define cdvd_read_symbol					0x8E46C
#define cdvd_send_atapi_command_symbol		0x8DA40
#define cdvd_send_device_command_symbol		0x8D954

#define ufs_open_symbol						0x24EB70
#define ufs_close_symbol					0x24F318
#define ufs_read_symbol						0x24E880
#define ufs_write_symbol					0x24E9F8
#define ufs_fstat_symbol					0x24E6C0

#define log_printf_symbol					0x1E495C

#define zeroalloc_symbol					0x1E48AC
#define malloc_symbol						0x1E484C
#define free_symbol							0x1E48F8
#define memcpy_symbol						0x514BC
#define memset_symbol						0x51528
#define snprintf_symbol						0x52360
#define strcpy_symbol						0x51600
#define strncpy_symbol						0x99BD0
#define strcat_symbol						0x51634
#define strlen_symbol						0x515C8
#define strcmp_symbol						0x99BFC
#define strchr_symbol						0x99B5C
#define strrchr_symbol						0x99B90

#define vuart_read_symbol					0x49544
#define vuart_write_symbol					0x494A4

#define ps2_disc_auth_symbol				0x900BC
#define ps2_disc_auth_caller_symbol			0x24F91C

#define overwritten_symbol					0x8CA98

#elif defined(PS2NETEMU)

#define TOC                					0x751180

#define cdvd_read_symbol   					0x1368BC
#define ufs_open_symbol    					0x1EBD5C
#define ufs_close_symbol   					0x1EBC10
#define ufs_read_symbol    					0x1EC6CC
#define ufs_write_symbol   					0x1EC558
#define ufs_fstat_symbol   					0x1EB860

#define decrypt_symbol						cdvd_read_symbol

#define log_printf_symbol  					0xB87F0

#define memcpy_symbol      					0x1183D0
#define memset_symbol      					0x11837C
#define memcmp_symbol      					0x1181A0
#define memchr_symbol      					0x1E5ED8
#define snprintf_symbol    					0x119064
#define strcpy_symbol      					0x11821C
#define strncpy_symbol     					0x1EA740
#define strcat_symbol      					0x118254
#define strlen_symbol      					0x1181E4
#define strcmp_symbol      					0x1EA770
#define strncmp_symbol     					0x1EA7F4
#define strchr_symbol      					0x1EA6C8
#define strrchr_symbol     					0x1EA6FC

#endif

#endif  /* FIRMWARE */

#endif /* __PS2EMU_SYMBOLS_H_S__ */
