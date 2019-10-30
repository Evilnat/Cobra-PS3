/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_4_84DEX)

#define TOC 											0x3759C0

#define create_kernel_object_symbol						0x125F8
#define destroy_kernel_object_symbol					0x11F5C
#define destroy_shared_kernel_object_symbol				0x118D0
#define open_kernel_object_symbol						0x12C48
#define open_shared_kernel_object_symbol				0x12A58
#define close_kernel_object_handle_symbol				0x12078

#define alloc_symbol									0x681F4
#define dealloc_symbol									0x68630
#define copy_to_user_symbol								0xFEB4
#define copy_from_user_symbol							0x100D0
#define copy_to_process_symbol							0xFF6C
#define copy_from_process_symbol						0xFD7C
#define process_read_memory_symbol						0x26E7E4
#define process_write_memory_symbol						0x26E658
#define page_allocate_symbol							0x63D64
#define page_free_symbol								0x637C8
#define page_export_to_proc_symbol						0x63F00
#define page_unexport_from_proc_symbol					0x636BC
#define kernel_ea_to_lpar_addr_symbol					0x73C9C
#define process_ea_to_lpar_addr_ex_symbol				0x7B83C
#define set_pte_symbol                      			0x61D60
#define map_process_memory_symbol						0x7B348
#define panic_symbol									0x2754A0

#define memcpy_symbol									0x82A08
#define memset_symbol									0x51014
#define memcmp_symbol									0x50324
#define memchr_symbol									0x502D4
#define printf_symbol									0x278978
#define printfnull_symbol								0x27D400
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

#define spin_lock_irqsave_ex_symbol						0x275670
#define spin_unlock_irqrestore_ex_symbol				0x275644

#define create_process_common_symbol					0x272468
#define create_process_base_symbol						0x271B00
#define load_process_symbol								0x5004
#define process_kill_symbol								0x272268

#define ppu_thread_create_symbol						0x145FC
#define ppu_thread_exit_symbol							0x146B4
#define ppu_thread_join_symbol							0x14708
#define ppu_thread_delay_symbol							0x2A6E8
#define create_kernel_thread_symbol						0x267D0
#define create_user_thread_symbol						0x26F0C
#define create_user_thread2_symbol						0x26D30
#define start_thread_symbol								0x25868
#define run_thread_symbol								0x2502C
#define register_thread_symbol							0x26E270
#define allocate_user_stack_symbol						0x26EA58
#define deallocate_user_stack_symbol					0x26E9C0

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

#define cellFsOpen_symbol								0x2B2898
#define cellFsClose_symbol								0x2B2700
#define cellFsRead_symbol								0x2B283C
#define cellFsWrite_symbol								0x2B27A8
#define cellFsLseek_symbol								0x2B1FFC
#define cellFsStat_symbol								0x2B20B4
#define cellFsUtime_symbol 								0x2B3A24
#define cellFsUnlink_internal_symbol   		 			0x1A29D8
#define cellFsRename_internal_symbol   		 			0x19D490

#define cellFsUtilMount_symbol							0x2B1D70
#define cellFsUtilUmount_symbol 						0x2B1D44
#define cellFsUtilNewfs_symbol							0x2B36BC

#define pathdup_from_user_symbol						0x2B7B5C
#define open_path_symbol								0x2B25D0
#define open_fs_object_symbol							0x190CFC
#define close_fs_object_symbol							0x18FC38

#define storage_get_device_info_symbol					0x28CA00
#define storage_get_device_config_symbol				0x28B08C
#define storage_open_symbol								0x28CC28
#define storage_close_symbol							0x28B414
#define storage_read_symbol								0x28A390
#define storage_write_symbol							0x28A260
#define storage_send_device_command_symbol				0x289EB0
#define storage_map_io_memory_symbol					0x28C8AC
#define storage_unmap_io_memory_symbol		    		0x28C768
#define new_medium_listener_object_symbol				0x9CC34
#define delete_medium_listener_object_symbol			0x9E46C
#define set_medium_event_callbacks_symbol				0x9E7D0

#define cellUsbdRegisterLdd_symbol						0x266758
#define cellUsbdUnregisterLdd_symbol					0x266708
#define cellUsbdScanStaticDescriptor_symbol				0x267958
#define cellUsbdOpenPipe_symbol			        		0x267A08
#define cellUsbdClosePipe_symbol						0x2679B8
#define cellUsbdControlTransfer_symbol					0x2678BC
#define cellUsbdBulkTransfer_symbol						0x26783C

#define decrypt_func_symbol								0x38290
#define lv1_call_99_wrapper_symbol						0x52850
#define modules_verification_symbol						0x5C488
#define authenticate_program_segment_symbol				0x5E2E0

#define prx_load_module_symbol							0x8D308
#define prx_start_module_symbol							0x8BFD4
#define prx_stop_module_symbol							0x8D3AC
#define prx_unload_module_symbol						0x8BD08
#define prx_get_module_info_symbol						0x8B700
#define prx_get_module_id_by_address_symbol				0x8B610
#define prx_get_module_id_by_name_symbol				0x8B660
#define prx_get_module_list_symbol						0x8B780
#define load_module_by_fd_symbol						0x8C938
#define parse_sprx_symbol								0x8A62C
#define open_prx_object_symbol							0x835D4
#define close_prx_object_symbol							0x83EE4
#define lock_prx_mutex_symbol							0x8B5B8
#define unlock_prx_mutex_symbol							0x8B5C4

#define extend_kstack_symbol							0x73BF4

#define get_pseudo_random_number_symbol					0x22E65C
#define md5_reset_symbol								0x1681AC
#define md5_update_symbol								0x168C4C
#define md5_final_symbol								0x168DCC
#define ss_get_open_psid_symbol							0x230B44
#define update_mgr_read_eeprom_symbol					0x229F3C
#define update_mgr_write_eeprom_symbol					0x229E70

/* RAP Activation */
#define SHA1_init_symbol								0x2F0228
#define SHA1_update_symbol								0x2F0288
#define SHA1_final_symbol								0x2F043C
#define aescbccfb_dec_symbol							0x2EE71C
#define aescbccfb_enc_symbol							0x2EE4CC

#define ss_params_get_update_status_symbol				0x5445C

#define syscall_table_symbol							0x38A4E8
#define syscall_call_offset								0x27DEE4

#define read_bdvd0_symbol								0x1AAACC
#define read_bdvd1_symbol								0x1AC6F8
#define read_bdvd2_symbol								0x1B98B0

#define storage_internal_get_device_object_symbol		0x289968

#define hid_mgr_read_usb_symbol	    					0x107F48
#define hid_mgr_read_bt_symbol							0x101E20

#define bt_set_controller_info_internal_symbol			0xF5E8C

/* Calls, jumps */
#define device_event_port_send_call						0x296BFC

#define ss_pid_call_1									0x21974C

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x834C8
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x8D1A4

#define shutdown_copy_params_call						0xAB4C

#define fsloop_open_call								0x2B2A30
#define fsloop_close_call								0x2B2A80
#define fsloop_read_call								0x2B2AC0

/* Patches */
#define shutdown_patch_offset							0xAB38
#define module_sdk_version_patch_offset					0x26D898
#define module_add_parameter_to_parse_sprxpatch_offset	0x8CA14

#define user_thread_prio_patch							0x21EAC
#define user_thread_prio_patch2							0x21EB8

#define lic_patch										0x59C00
#define ode_patch										0x22DAD0

/* Rtoc entries */

#define io_rtoc_entry_1									0x2EC0
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
#define permissions_exception2							0xC8A08
#define permissions_exception3							0x21F80

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x409A40
#define patch_func2_offset 								0x5D780
#define patch_func8_offset1 							0x59E80
#define patch_func8_offset2 							0x59FE4
#define patch_func9_offset 								0x5E0C8
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x3A4A30

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x74C08

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
