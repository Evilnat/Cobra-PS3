/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_4_50DEX)

#define TOC 											0x36EC40

#define create_kernel_object_symbol						0x12658
#define destroy_kernel_object_symbol					0x11FBC
#define destroy_shared_kernel_object_symbol				0x11930
#define open_kernel_object_symbol						0x12CA8
#define open_shared_kernel_object_symbol				0x12AB8
#define close_kernel_object_handle_symbol				0x120D8

#define alloc_symbol									0x66944
#define dealloc_symbol									0x66D80
#define copy_to_user_symbol								0xFEA0
#define copy_from_user_symbol							0x100BC
#define copy_to_process_symbol							0xFF58
#define copy_from_process_symbol						0xFD68
#define page_allocate_symbol							0x624B4
#define page_free_symbol								0x61F18
#define page_export_to_proc_symbol						0x62650
#define page_unexport_from_proc_symbol					0x61E0C
#define kernel_ea_to_lpar_addr_symbol					0x723B8
#define process_ea_to_lpar_addr_ex_symbol				0x79F58
#define set_pte_symbol                      			0x604B0
#define map_process_memory_symbol						0x79A64
#define panic_symbol									0x27D944

#define memcpy_symbol									0x81124
#define memset_symbol									0x50E38
#define memcmp_symbol									0x50148
#define memchr_symbol									0x500F8
#define printf_symbol									0x280E08
#define printfnull_symbol								0x28588C
#define sprintf_symbol				    				0x52260
#define snprintf_symbol				    				0x521CC
#define strcpy_symbol									0x50FE4
#define strncpy_symbol									0x510AC
#define strlen_symbol									0x5100C
#define strcat_symbol									0x50F14
#define strcmp_symbol									0x50F90
#define strncmp_symbol									0x51038
#define strchr_symbol									0x50F4C
#define strrchr_symbol									0x5111C

#define spin_lock_irqsave_ex_symbol						0x27DB14
#define spin_unlock_irqrestore_ex_symbol				0x27DAE8

#define create_process_common_symbol					0x27A928
#define create_process_base_symbol						0x279FCC
#define load_process_symbol								0x5004
#define process_kill_symbol								0x27A728

#define ppu_thread_create_symbol						0x1465C
#define ppu_thread_exit_symbol							0x14714
#define ppu_thread_join_symbol							0x14768
#define ppu_thread_delay_symbol							0x2A50C
#define create_kernel_thread_symbol						0x265FC
#define create_user_thread_symbol						0x26D38 
#define create_user_thread2_symbol						0x26B5C
#define start_thread_symbol								0x25694
#define run_thread_symbol								0x24E58
#define register_thread_symbol							0x27673C
#define allocate_user_stack_symbol						0x276F24
#define deallocate_user_stack_symbol					0x276E8C

#define mutex_create_symbol								0x13CEC
#define mutex_destroy_symbol							0x13C84
#define mutex_lock_symbol								0x13C7C
#define mutex_lock_ex_symbol							0x1F438
#define mutex_trylock_symbol							0x13C78
#define mutex_unlock_symbol								0x13C74

#define cond_create_symbol								0x13E58
#define cond_destroy_symbol								0x13E08
#define cond_wait_symbol								0x13E00
#define cond_wait_ex_symbol								0x20A24
#define cond_signal_symbol								0x13DDC
#define cond_signal_all_symbol							0x13DB8 

#define semaphore_initialize_symbol						0x34398
#define semaphore_wait_ex_symbol						0x340A0
#define semaphore_trywait_symbol						0x33C88
#define semaphore_post_ex_symbol						0x33DD4

#define event_port_create_symbol						0x13728
#define event_port_destroy_symbol						0x13B90
#define event_port_connect_symbol						0x13C08
#define event_port_disconnect_symbol					0x13B34
#define event_port_send_symbol							0x13720
#define event_port_send_ex_symbol						0x2D0EC

#define event_queue_create_symbol						0x13A30
#define event_queue_destroy_symbol						0x139B8
#define event_queue_receive_symbol						0x137FC
#define event_queue_tryreceive_symbol					0x138C8

#define cellFsOpen_symbol								0x2B84B0
#define cellFsClose_symbol								0x2B8318
#define cellFsRead_symbol								0x2B8454
#define cellFsWrite_symbol								0x2B83C0
#define cellFsLseek_symbol								0x2B7C14
#define cellFsStat_symbol								0x2B7CCC
#define cellFsUtime_symbol 								0x2B963C
#define cellFsUnlink_internal_symbol   		 			0x1A2330

#define cellFsUtilMount_symbol							0x2B7988 
#define cellFsUtilUmount_symbol 						0x2B795C 
#define cellFsUtilNewfs_symbol							0x2B92D4 

#define pathdup_from_user_symbol						0x1A858C
#define open_path_symbol								0x2B81E8 
#define open_fs_object_symbol							0x190A68
#define close_fs_object_symbol							0x18F9A4

#define storage_get_device_info_symbol					0x294E8C 
#define storage_get_device_config_symbol				0x293518 
#define storage_open_symbol								0x2950B4 
#define storage_close_symbol							0x2938A0 
#define storage_read_symbol								0x29281C 
#define storage_write_symbol							0x2926EC 
#define storage_send_device_command_symbol				0x29233C 
#define storage_map_io_memory_symbol					0x294D38 
#define storage_unmap_io_memory_symbol		    		0x294BF4 
#define new_medium_listener_object_symbol				0x9B25C 
#define delete_medium_listener_object_symbol			0x9CA94 
#define set_medium_event_callbacks_symbol				0x9CDF8 

#define cellUsbdRegisterLdd_symbol						0x26EC2C 
#define cellUsbdUnregisterLdd_symbol					0x26EBDC 
#define cellUsbdScanStaticDescriptor_symbol				0x26FE2C 
#define cellUsbdOpenPipe_symbol			        		0x26FEDC 
#define cellUsbdClosePipe_symbol						0x26FE8C 
#define cellUsbdControlTransfer_symbol					0x26FD90 
#define cellUsbdBulkTransfer_symbol						0x26FD10 

#define decrypt_func_symbol								0x380B4
#define lv1_call_99_wrapper_symbol						0x52674
#define modules_verification_symbol						0x5C1F4
#define authenticate_program_segment_symbol				0x5D630

#define prx_load_module_symbol							0x8B934
#define prx_start_module_symbol							0x8A600
#define prx_stop_module_symbol							0x8B9D8
#define prx_unload_module_symbol						0x8A334
#define prx_get_module_info_symbol						0x89D2C
#define prx_get_module_id_by_address_symbol				0x89C3C
#define prx_get_module_id_by_name_symbol				0x89C8C
#define prx_get_module_list_symbol						0x89DAC
#define load_module_by_fd_symbol						0x8AF64
#define parse_sprx_symbol								0x88C58
#define open_prx_object_symbol							0x81CE0
#define close_prx_object_symbol							0x825F0
#define lock_prx_mutex_symbol							0x89BE4
#define unlock_prx_mutex_symbol							0x89BF0

#define extend_kstack_symbol							0x72310

#define get_pseudo_random_number_symbol					0x236E0C
#define md5_reset_symbol								0x167F24
#define md5_update_symbol								0x1689C4
#define md5_final_symbol								0x168B44
#define ss_get_open_psid_symbol							0x2392B4
#define update_mgr_read_eeprom_symbol					0x232900
#define update_mgr_write_eeprom_symbol					0x232834

#define ss_params_get_update_status_symbol				0x54280

#define syscall_table_symbol							0x383658
#define syscall_call_offset								0x286370

#define read_bdvd0_symbol								0x1B34FC
#define read_bdvd1_symbol								0x1B5128
#define read_bdvd2_symbol								0x1C22B0

#define storage_internal_get_device_object_symbol		0x291DF4

#define hid_mgr_read_usb_symbol	    					0x107F8C
#define hid_mgr_read_bt_symbol							0x101E64

#define bt_set_controller_info_internal_symbol			0xF5ED0

/* Calls, jumps */
#define device_event_port_send_call						0x29F088

#define ss_pid_call_1									0x222110

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x81BD4
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x8B7D0

#define shutdown_copy_params_call						0xAB3C

#define fsloop_open_call								0x2B8648
#define fsloop_close_call								0x2B8698
#define fsloop_read_call								0x2B86D8

/* Patches */
#define shutdown_patch_offset							0xAB28
#define module_sdk_version_patch_offset					0x275D64
#define module_add_parameter_to_parse_sprxpatch_offset	0x8B040

#define user_thread_prio_patch							0x21CD8
#define user_thread_prio_patch2							0x21CE4

#define lic_patch										0x59758
#define ode_patch										0x236414

/* Rtoc entries */

#define io_rtoc_entry_1									-0xB8
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x65A8
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x22A0

#define device_event_rtoc_entry_1						0x2628

#define time_rtoc_entry_1								-0x75E0
#define time_rtoc_entry_2								-0x75E8

#define thread_rtoc_entry_1								-0x7660

#define process_rtoc_entry_1							-0x77A0

#define bt_rtoc_entry_1									-0x3548

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x26BDC
#define permissions_exception2							0xC8CDC
#define permissions_exception3							0x21DAC

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x402AC0
#define patch_func2_offset 								0x5CBD8
#define patch_func8_offset1 							0x599B0
#define patch_func8_offset2 							0x59B14
#define patch_func9_offset 								0x5D4D8
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x39DAB0

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
