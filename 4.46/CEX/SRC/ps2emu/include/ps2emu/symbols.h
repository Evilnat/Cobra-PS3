/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __PS2EMU_SYMBOLS_H_S__
#define __PS2EMU_SYMBOLS_H_S__

#if defined(FIRMWARE_4_46)

#if defined(PS2HWEMU)

#define TOC 								0x4EB840

#define cdvd_send_atapi_command_symbol		0x220B8

#define ufs_open_symbol						0x4E028
#define ufs_close_symbol					0x4E7B4
#define ufs_read_symbol						0x4DAF8
#define ufs_write_symbol					0x4DBEC
#define ufs_fstat_symbol					0x4EAB8

#define printf_symbol						0x3BC4

#define zeroalloc_symbol					0xFDF60
#define malloc_symbol						0xFDE78
#define free_symbol							0xFDFB8
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
#define strncpy_symbol						0x99BC0
#define strcat_symbol						0x51634
#define strlen_symbol						0x515C8
#define strcmp_symbol						0x99BEC
#define strchr_symbol						0x99B4C
#define strrchr_symbol						0x99B80

#define vuart_read_symbol					0x49544
#define vuart_write_symbol					0x494A4

#define ps2_disc_auth_symbol				0x900BC
#define ps2_disc_auth_caller_symbol			0x24F91C

#define overwritten_symbol					0x8CA98

#elif defined(PS2NETEMU)

#define TOC                					0x74FC80

#define	cdvd_read_symbol					0x1332F4
#define ufs_open_symbol						0x1E87A4
#define ufs_close_symbol					0x1E8658
#define ufs_read_symbol						0x1E9114
#define ufs_write_symbol					0x1E8FA0
#define ufs_fstat_symbol					0x1E82A8

#define log_printf_symbol					0xB7340

#define memcpy_symbol						0x115310
#define memset_symbol						0x1152BC
#define memcmp_symbol						0x1150E0
#define memchr_symbol						0x1E2908
#define snprintf_symbol						0x115FA4
#define strcpy_symbol						0x11515C
#define strncpy_symbol						0x1E7188
#define strcat_symbol						0x115194
#define strlen_symbol						0x115124
#define strcmp_symbol						0x1E71B8
#define strncmp_symbol						0x1E723C
#define strchr_symbol						0x1E7110
#define strrchr_symbol						0x1E7144

#endif

#endif  /* FIRMWARE */

#endif /* __PS2EMU_SYMBOLS_H_S__ */
