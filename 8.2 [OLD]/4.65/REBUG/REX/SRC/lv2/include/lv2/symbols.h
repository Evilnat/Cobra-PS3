/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_CEX)

#define TOC 											0x34F960

#define create_kernel_object_symbol						0x11FB0
#define destroy_kernel_object_symbol					0x11914
#define destroy_shared_kernel_object_symbol				0x11288
#define open_kernel_object_symbol						0x12600
#define open_shared_kernel_object_symbol				0x12410
#define close_kernel_object_handle_symbol				0x11A30

#define alloc_symbol									0x6479C
#define dealloc_symbol									0x64BD8
#define copy_to_user_symbol								0xF86C
#define copy_from_user_symbol							0xFA88
#define copy_to_process_symbol							0xF924
#define copy_from_process_symbol						0xF734
#define process_read_memory_symbol						0x270B50
#define process_write_memory_symbol						0x2709C4
#define page_allocate_symbol							0x6030C
#define page_free_symbol								0x5FD70
#define page_export_to_proc_symbol						0x604A8
#define page_unexport_from_proc_symbol					0x5FC64
#define kernel_ea_to_lpar_addr_symbol					0x700C4
#define process_ea_to_lpar_addr_ex_symbol				0x776D8
#define set_pte_symbol                      			0x5E308
#define map_process_memory_symbol						0x771E4
#define panic_symbol									0x276258

#define memcpy_symbol									0x7E8A4
#define memset_symbol									0x4D66C
#define memcmp_symbol									0x4C97C
#define memchr_symbol									0x4C92C
#define printf_symbol									0x27933C
#define printfnull_symbol								0x27DDB0
#define sprintf_symbol				    				0x4EA94
#define snprintf_symbol				    				0x4EA00
#define strcpy_symbol									0x4D818
#define strncpy_symbol									0x4D8E0
#define strlen_symbol									0x4D840
#define strcat_symbol									0x4D748
#define strcmp_symbol									0x4D7C4
#define strncmp_symbol									0x4D86C
#define strchr_symbol									0x4D780
#define strrchr_symbol									0x4D950

#define spin_lock_irqsave_ex_symbol						0x276428
#define spin_unlock_irqrestore_ex_symbol				0x2763FC

#define create_process_common_symbol					0x2741C4
#define create_process_base_symbol						0x273880
#define load_process_symbol								0x5004
#define process_kill_symbol								0x273FD0

#define ppu_thread_create_symbol						0x13EC8
#define ppu_thread_exit_symbol							0x13F80
#define ppu_thread_join_symbol							0x13FD4
#define ppu_thread_delay_symbol							0x287A4
#define create_kernel_thread_symbol						0x24B20
#define create_user_thread_symbol						0x2525C 
#define create_user_thread2_symbol						0x25080
#define start_thread_symbol								0x23D4C
#define run_thread_symbol								0x2357C
#define register_thread_symbol							0x2705DC
#define allocate_user_stack_symbol						0x270DC4
#define deallocate_user_stack_symbol					0x270D2C

#define mutex_create_symbol								0x1363C
#define mutex_destroy_symbol							0x135D4
#define mutex_lock_symbol								0x135CC
#define mutex_lock_ex_symbol							0x1DB5C
#define mutex_trylock_symbol							0x135C8
#define mutex_unlock_symbol								0x135C4

#define cond_create_symbol								0x137A8
#define cond_destroy_symbol								0x13758
#define cond_wait_symbol								0x13750
#define cond_wait_ex_symbol								0x1F148
#define cond_signal_symbol								0x1372C
#define cond_signal_all_symbol							0x13708 

#define semaphore_initialize_symbol						0x30C54
#define semaphore_wait_ex_symbol						0x3095C
#define semaphore_trywait_symbol						0x30544
#define semaphore_post_ex_symbol						0x30690

#define event_port_create_symbol						0x13078
#define event_port_destroy_symbol						0x134E0
#define event_port_connect_symbol						0x13558
#define event_port_disconnect_symbol					0x13484
#define event_port_send_symbol							0x13070
#define event_port_send_ex_symbol						0x2B204

#define event_queue_create_symbol						0x13380
#define event_queue_destroy_symbol						0x13308
#define event_queue_receive_symbol						0x1314C
#define event_queue_tryreceive_symbol					0x13218

#define cellFsOpen_symbol								0x2A0590
#define cellFsClose_symbol								0x2A03F8
#define cellFsRead_symbol								0x2A0534
#define cellFsWrite_symbol								0x2A04A0
#define cellFsLseek_symbol								0x29FB28
#define cellFsStat_symbol								0x29FDAC
#define cellFsUtime_symbol 								0x2A1898
#define cellFsUnlink_internal_symbol   		 			0x19C4B4
#define cellFsRename_internal_symbol   		 			0x196F6C

#define cellFsUtilMount_symbol							0x29F908 
#define cellFsUtilUmount_symbol 						0x29F8DC 
#define cellFsUtilNewfs_symbol							0x2A1330 

#define pathdup_from_user_symbol						0x1A23A8
#define open_path_symbol								0x2A02C8 
#define open_fs_object_symbol							0x18A7D8
#define close_fs_object_symbol							0x189714

#define storage_get_device_info_symbol					0x282790 
#define storage_get_device_config_symbol				0x281C20 
#define storage_open_symbol								0x2821A0 
#define storage_close_symbol							0x281F90 
#define storage_read_symbol								0x281500 
#define storage_write_symbol							0x2813D0 
#define storage_send_device_command_symbol				0x28108C 
#define storage_map_io_memory_symbol					0x28264C 
#define storage_unmap_io_memory_symbol		    		0x282518 
#define new_medium_listener_object_symbol				0x9743C 
#define delete_medium_listener_object_symbol			0x98C74 
#define set_medium_event_callbacks_symbol				0x98FD8 

#define cellUsbdRegisterLdd_symbol						0x268F0C 
#define cellUsbdUnregisterLdd_symbol					0x268EBC 
#define cellUsbdScanStaticDescriptor_symbol				0x26A10C 
#define cellUsbdOpenPipe_symbol			        		0x26A1BC 
#define cellUsbdClosePipe_symbol						0x26A16C 
#define cellUsbdControlTransfer_symbol					0x26A070 
#define cellUsbdBulkTransfer_symbol						0x269FF0 

#define decrypt_func_symbol								0x34970
#define lv1_call_99_wrapper_symbol						0x4EEA8
#define modules_verification_symbol						0x58A4C
#define authenticate_program_segment_symbol				0x5A888

#define prx_load_module_symbol							0x88B7C
#define prx_start_module_symbol							0x87848
#define prx_stop_module_symbol							0x88C20
#define prx_unload_module_symbol						0x8757C
#define prx_get_module_info_symbol						0x87004
#define prx_get_module_id_by_address_symbol				0x86F14
#define prx_get_module_id_by_name_symbol				0x86F64
#define prx_get_module_list_symbol						0x87084
#define load_module_by_fd_symbol						0x881AC
#define parse_sprx_symbol								0x85F30
#define open_prx_object_symbol							0x7F470
#define close_prx_object_symbol							0x7FD80
#define lock_prx_mutex_symbol							0x86EBC
#define unlock_prx_mutex_symbol							0x86EC8

#define extend_kstack_symbol							0x7001C

#define get_pseudo_random_number_symbol					0x230E7C
#define md5_reset_symbol								0x161C88
#define md5_update_symbol								0x162728
#define md5_final_symbol								0x1628A8
#define ss_get_open_psid_symbol							0x233364
#define update_mgr_read_eeprom_symbol					0x22C774
#define update_mgr_write_eeprom_symbol					0x22C6BC

/* RAP Activation */
#define SHA1_init_symbol								0x2CE480
#define SHA1_update_symbol								0x2CE4E0
#define SHA1_final_symbol								0x2CE694
#define aescbccfb_dec_symbol							0x2CC974
#define aescbccfb_enc_symbol							0x2CC724

#define ss_params_get_update_status_symbol				0x50AB4

#define syscall_table_symbol							0x363A18
#define syscall_call_offset								0x27E894

#define read_bdvd0_symbol								0x1AD318
#define read_bdvd1_symbol								0x1AEF44
#define read_bdvd2_symbol								0x1BC0FC

#define storage_internal_get_device_object_symbol		0x280B44

#define hid_mgr_read_usb_symbol	    					0x1023BC
#define hid_mgr_read_bt_symbol							0xFC294

#define bt_set_controller_info_internal_symbol			0xF0300

/* Calls, jumps */
#define device_event_port_send_call						0x28ACFC

#define ss_pid_call_1									0x21BF98

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x7F364
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x88A18

#define shutdown_copy_params_call						0xAACC

#define fsloop_open_call								0x2A0728
#define fsloop_close_call								0x2A0778
#define fsloop_read_call								0x2A07B8

/* Patches */
#define shutdown_patch_offset							0xAAB8
#define module_sdk_version_patch_offset					0x26FE0C
#define module_add_parameter_to_parse_sprxpatch_offset	0x88288

#define user_thread_prio_patch							0x203FC
#define user_thread_prio_patch2							0x20408

#define lic_patch										0x56230
#define ode_patch										0x2302F0

/* Rtoc entries */

#define io_rtoc_entry_1									-0x1F0
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x66A0
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x1DD8

#define device_event_rtoc_entry_1						0x2048

#define time_rtoc_entry_1								-0x7640
#define time_rtoc_entry_2								-0x7648

#define thread_rtoc_entry_1								-0x76C0

#define process_rtoc_entry_1							-0x7800

#define bt_rtoc_entry_1									-0x3680

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x25100
#define permissions_exception2							0xC3210
#define permissions_exception3							0x204D0

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x3E2BC0
#define patch_func2_offset 								0x59D28
#define patch_func8_offset1 							0x564B0
#define patch_func8_offset2 							0x56614
#define patch_func9_offset 								0x5A670
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x37DBB0

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x70EF0

#elif defined(FIRMWARE_DEX)

#define TOC 											0x375510

#define create_kernel_object_symbol						0x125F8
#define destroy_kernel_object_symbol					0x11F5C
#define destroy_shared_kernel_object_symbol				0x118D0
#define open_kernel_object_symbol						0x12C48
#define open_shared_kernel_object_symbol				0x12A58
#define close_kernel_object_handle_symbol				0x12078

#define alloc_symbol									0x6816C
#define dealloc_symbol									0x685A8
#define copy_to_user_symbol								0xFEB4
#define copy_from_user_symbol							0x100D0
#define copy_to_process_symbol							0xFF6C
#define copy_from_process_symbol						0xFD7C
#define process_read_memory_symbol						0x277474
#define process_write_memory_symbol						0x2772E8
#define page_allocate_symbol							0x63CDC
#define page_free_symbol								0x63740
#define page_export_to_proc_symbol						0x63E78
#define page_unexport_from_proc_symbol					0x63634
#define kernel_ea_to_lpar_addr_symbol					0x73C14
#define process_ea_to_lpar_addr_ex_symbol				0x7B7B4
#define set_pte_symbol                      			0x61CD8
#define map_process_memory_symbol						0x7B2C0
#define panic_symbol									0x27E130

#define memcpy_symbol									0x82980
#define memset_symbol									0x51014
#define memcmp_symbol									0x50324
#define memchr_symbol									0x502D4
#define printf_symbol									0x281608
#define printfnull_symbol								0x286090
#define sprintf_symbol				    				0x5243C
#define snprintf_symbol				    				0x523A8
#define strcpy_symbol									0x511C0
#define strncpy_symbol									0x51288
#define strlen_symbol									0x511E8
#define strcat_symbol									0x510F0
#define strcmp_symbol									0x5116C
#define strncmp_symbol									0x51214
#define strchr_symbol									0x51128
#define strrchr_symbol									0x512F8

#define spin_lock_irqsave_ex_symbol						0x27E300
#define spin_unlock_irqrestore_ex_symbol				0x27E2D4

#define create_process_common_symbol					0x27B0F8
#define create_process_base_symbol						0x27A790
#define load_process_symbol								0x5004
#define process_kill_symbol								0x27AEF8

#define ppu_thread_create_symbol						0x145FC
#define ppu_thread_exit_symbol							0x146B4
#define ppu_thread_join_symbol							0x14708
#define ppu_thread_delay_symbol							0x2A6E8
#define create_kernel_thread_symbol						0x267D0
#define create_user_thread_symbol						0x26F0C
#define create_user_thread2_symbol						0x26D30
#define start_thread_symbol								0x25868
#define run_thread_symbol								0x2502C
#define register_thread_symbol							0x276F00
#define allocate_user_stack_symbol						0x2776E8
#define deallocate_user_stack_symbol					0x277650

#define mutex_create_symbol								0x13C8C
#define mutex_destroy_symbol							0x13C24
#define mutex_lock_symbol								0x13C1C
#define mutex_lock_ex_symbol							0x1F60C
#define mutex_trylock_symbol							0x13C18
#define mutex_unlock_symbol								0x13C14

#define cond_create_symbol								0x13DF8
#define cond_destroy_symbol								0x13DA8
#define cond_wait_symbol								0x13DA0
#define cond_wait_ex_symbol								0x20BF8
#define cond_signal_symbol								0x13D7C
#define cond_signal_all_symbol							0x13D58

#define semaphore_initialize_symbol						0x34574
#define semaphore_wait_ex_symbol						0x3427C
#define semaphore_trywait_symbol						0x33E64
#define semaphore_post_ex_symbol						0x33FB0

#define event_port_create_symbol						0x136C8
#define event_port_destroy_symbol						0x13B30
#define event_port_connect_symbol						0x13BA8
#define event_port_disconnect_symbol					0x13AD4
#define event_port_send_symbol							0x136C0
#define event_port_send_ex_symbol						0x2D2C8

#define event_queue_create_symbol						0x139D0
#define event_queue_destroy_symbol						0x13958
#define event_queue_receive_symbol						0x1379C
#define event_queue_tryreceive_symbol					0x13868

#define cellFsOpen_symbol								0x2BB2D8
#define cellFsClose_symbol								0x2BB140
#define cellFsRead_symbol								0x2BB27C
#define cellFsWrite_symbol								0x2BB1E8
#define cellFsLseek_symbol								0x2BAA3C
#define cellFsStat_symbol								0x2BAAF4
#define cellFsUtime_symbol 								0x2BC464
#define cellFsUnlink_internal_symbol   		 			0x1A2840
#define cellFsRename_internal_symbol   		 			0x19D2F8

#define cellFsUtilMount_symbol							0x2BA7B0
#define cellFsUtilUmount_symbol 						0x2BA784
#define cellFsUtilNewfs_symbol							0x2BC0FC

#define pathdup_from_user_symbol						0x1A8858
#define open_path_symbol								0x2BB010
#define open_fs_object_symbol							0x190B64
#define close_fs_object_symbol							0x18FAA0

#define storage_get_device_info_symbol					0x295690
#define storage_get_device_config_symbol				0x293D1C
#define storage_open_symbol								0x2958B8
#define storage_close_symbol							0x2940A4
#define storage_read_symbol								0x293020
#define storage_write_symbol							0x292EF0
#define storage_send_device_command_symbol				0x292B40
#define storage_map_io_memory_symbol					0x29553C
#define storage_unmap_io_memory_symbol		    		0x2953F8
#define new_medium_listener_object_symbol				0x9CBAC
#define delete_medium_listener_object_symbol			0x9E3E4
#define set_medium_event_callbacks_symbol				0x9E748

#define cellUsbdRegisterLdd_symbol						0x26F3E8
#define cellUsbdUnregisterLdd_symbol					0x26F398
#define cellUsbdScanStaticDescriptor_symbol				0x2705E8
#define cellUsbdOpenPipe_symbol			        		0x270698
#define cellUsbdClosePipe_symbol						0x270648
#define cellUsbdControlTransfer_symbol					0x27054C
#define cellUsbdBulkTransfer_symbol						0x2704CC

#define decrypt_func_symbol								0x38290
#define lv1_call_99_wrapper_symbol						0x52850
#define modules_verification_symbol						0x5C41C
#define authenticate_program_segment_symbol				0x5E258

#define prx_load_module_symbol							0x8D280
#define prx_start_module_symbol							0x8BF4C
#define prx_stop_module_symbol							0x8D324
#define prx_unload_module_symbol						0x8BC80
#define prx_get_module_info_symbol						0x8B678
#define prx_get_module_id_by_address_symbol				0x8B588
#define prx_get_module_id_by_name_symbol				0x8B5D8
#define prx_get_module_list_symbol						0x8B6F8
#define load_module_by_fd_symbol						0x8C8B0
#define parse_sprx_symbol								0x8A5A4
#define open_prx_object_symbol							0x8354C
#define close_prx_object_symbol							0x83E5C
#define lock_prx_mutex_symbol							0x8B530
#define unlock_prx_mutex_symbol							0x8B53C

#define extend_kstack_symbol							0x73B6C

#define get_pseudo_random_number_symbol					0x237358
#define md5_reset_symbol								0x168014
#define md5_update_symbol								0x168AB4
#define md5_final_symbol								0x168C34
#define ss_get_open_psid_symbol							0x239840
#define update_mgr_read_eeprom_symbol					0x232C38
#define update_mgr_write_eeprom_symbol					0x232B6C

/* RAP Activation */
#define SHA1_init_symbol								0x2EFDCC
#define SHA1_update_symbol								0x2EFE2C
#define SHA1_final_symbol								0x2EFFE0
#define aescbccfb_dec_symbol							0x2EE2C0
#define aescbccfb_enc_symbol							0x2EE070

#define ss_params_get_update_status_symbol				0x5445C

#define syscall_table_symbol							0x38A120
#define syscall_call_offset								0x286B74

#define read_bdvd0_symbol								0x1B37C8
#define read_bdvd1_symbol								0x1B53F4
#define read_bdvd2_symbol								0x1C25AC

#define storage_internal_get_device_object_symbol		0x2925F8

#define hid_mgr_read_usb_symbol	    					0x107EC0
#define hid_mgr_read_bt_symbol							0x101D98

#define bt_set_controller_info_internal_symbol			0xF5E04

/* Calls, jumps */
#define device_event_port_send_call						0x29F88C

#define ss_pid_call_1									0x222448

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x83440
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x8D11C

#define shutdown_copy_params_call						0xAB4C

#define fsloop_open_call								0x2BB470
#define fsloop_close_call								0x2BB4C0
#define fsloop_read_call								0x2BB500

/* Patches */
#define shutdown_patch_offset							0xAB38
#define module_sdk_version_patch_offset					0x276528
#define module_add_parameter_to_parse_sprxpatch_offset	0x8C98C

#define user_thread_prio_patch							0x21EAC
#define user_thread_prio_patch2							0x21EB8

#define lic_patch										0x59C00
#define ode_patch										0x28D0DC

/* Rtoc entries */

#define io_rtoc_entry_1									-0xC0
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x65C0
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x22B0

#define device_event_rtoc_entry_1						0x2638

#define time_rtoc_entry_1								-0x75E0
#define time_rtoc_entry_2								-0x75E8

#define thread_rtoc_entry_1								-0x7660

#define process_rtoc_entry_1							-0x77A0

#define bt_rtoc_entry_1									-0x3558

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x26DB0
#define permissions_exception2							0xC8980
#define permissions_exception3							0x21F80

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x4095C0
#define patch_func2_offset 								0x5D6F8
#define patch_func8_offset1 							0x59E80
#define patch_func8_offset2 							0x59FE4
#define patch_func9_offset 								0x5E040
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x3A45B0

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x74B80

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
