/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_4_70DEX)

#define TOC 											0x375850

#define create_kernel_object_symbol						0x125F4
#define destroy_kernel_object_symbol					0x11F58
#define destroy_shared_kernel_object_symbol				0x118CC
#define open_kernel_object_symbol						0x12C44
#define open_shared_kernel_object_symbol				0x12A54
#define close_kernel_object_handle_symbol				0x12074

#define alloc_symbol									0x681F0
#define dealloc_symbol									0x6862C
#define copy_to_user_symbol								0xFEB0
#define copy_from_user_symbol							0x100CC
#define copy_to_process_symbol							0xFF68
#define copy_from_process_symbol						0xFD78
#define page_allocate_symbol							0x63D60
#define page_free_symbol								0x637C4
#define page_export_to_proc_symbol						0x63EFC
#define page_unexport_from_proc_symbol					0x636B8
#define kernel_ea_to_lpar_addr_symbol					0x73C98
#define process_ea_to_lpar_addr_ex_symbol				0x7B838
#define set_pte_symbol                      			0x61D5C
#define map_process_memory_symbol						0x7B344
#define panic_symbol									0x275428

#define memcpy_symbol									0x82A04
#define memset_symbol									0x51010
#define memcmp_symbol									0x50320
#define memchr_symbol									0x502D0
#define printf_symbol									0x278900
#define printfnull_symbol								0x27D388
#define sprintf_symbol				    				0x52438
#define snprintf_symbol				    				0x523A4
#define strcpy_symbol									0x511BC
#define strncpy_symbol									0x51284
#define strlen_symbol									0x511E4
#define strcat_symbol									0x510EC
#define strcmp_symbol									0x51168
#define strncmp_symbol									0x51210
#define strchr_symbol									0x51124
#define strrchr_symbol									0x512F4

#define spin_lock_irqsave_ex_symbol						0x2755F8
#define spin_unlock_irqrestore_ex_symbol				0x2755CC

#define create_process_common_symbol					0x2723F0
#define create_process_base_symbol						0x271A88
#define load_process_symbol								0x5004
#define process_kill_symbol								0x2721F0

#define ppu_thread_create_symbol						0x145F8
#define ppu_thread_exit_symbol							0x146B0
#define ppu_thread_join_symbol							0x14704
#define ppu_thread_delay_symbol							0x2A6E4
#define create_kernel_thread_symbol						0x267CC
#define create_user_thread_symbol						0x26F08 
#define create_user_thread2_symbol						0x26D2C
#define start_thread_symbol								0x25864
#define run_thread_symbol								0x25028
#define register_thread_symbol							0x26E1F8
#define allocate_user_stack_symbol						0x26E9E0
#define deallocate_user_stack_symbol					0x26E948

#define mutex_create_symbol								0x13C88
#define mutex_destroy_symbol							0x13C20
#define mutex_lock_symbol								0x13C18
#define mutex_lock_ex_symbol							0x1F608
#define mutex_trylock_symbol							0x13C14
#define mutex_unlock_symbol								0x13C10

#define cond_create_symbol								0x13DF4
#define cond_destroy_symbol								0x13DA4
#define cond_wait_symbol								0x13D9C
#define cond_wait_ex_symbol								0x20BF4
#define cond_signal_symbol								0x13D78
#define cond_signal_all_symbol							0x13D54 

#define semaphore_initialize_symbol						0x34570
#define semaphore_wait_ex_symbol						0x34278
#define semaphore_trywait_symbol						0x33E60
#define semaphore_post_ex_symbol						0x33FAC

#define event_port_create_symbol						0x136C4
#define event_port_destroy_symbol						0x13B2C
#define event_port_connect_symbol						0x13BA4
#define event_port_disconnect_symbol					0x13AD0
#define event_port_send_symbol							0x136BC
#define event_port_send_ex_symbol						0x2D2C4

#define event_queue_create_symbol						0x139CC
#define event_queue_destroy_symbol						0x13954
#define event_queue_receive_symbol						0x13798
#define event_queue_tryreceive_symbol					0x13864

#define cellFsOpen_symbol								0x2B2748
#define cellFsClose_symbol								0x2B25B0
#define cellFsRead_symbol								0x2B26EC
#define cellFsWrite_symbol								0x2B2658
#define cellFsLseek_symbol								0x2B1EAC
#define cellFsStat_symbol								0x2B1F64
#define cellFsUtime_symbol 								0x2B38D4
#define cellFsUnlink_internal_symbol   		 			0x1A29D0

#define cellFsUtilMount_symbol							0x2B1C20 
#define cellFsUtilUmount_symbol 						0x2B1BF4 
#define cellFsUtilNewfs_symbol							0x2B356C 

#define pathdup_from_user_symbol						0x2B7A0C
#define open_path_symbol								0x2B2480 
#define open_fs_object_symbol							0x190CF4
#define close_fs_object_symbol							0x18FC30

#define storage_get_device_info_symbol					0x28C988 
#define storage_get_device_config_symbol				0x28B014 
#define storage_open_symbol								0x28CBB0 
#define storage_close_symbol							0x28B39C 
#define storage_read_symbol								0x28A318 
#define storage_write_symbol							0x28A1E8 
#define storage_send_device_command_symbol				0x289E38 
#define storage_map_io_memory_symbol					0x28C834 
#define storage_unmap_io_memory_symbol		    		0x28C6F0 
#define new_medium_listener_object_symbol				0x9CC2C 
#define delete_medium_listener_object_symbol			0x9E464 
#define set_medium_event_callbacks_symbol				0x9E7C8 

#define cellUsbdRegisterLdd_symbol						0x2666E4 
#define cellUsbdUnregisterLdd_symbol					0x266694 
#define cellUsbdScanStaticDescriptor_symbol				0x2678E4 
#define cellUsbdOpenPipe_symbol			        		0x267994 
#define cellUsbdClosePipe_symbol						0x267944 
#define cellUsbdControlTransfer_symbol					0x267848 
#define cellUsbdBulkTransfer_symbol						0x2677C8 

#define decrypt_func_symbol								0x3828C
#define lv1_call_99_wrapper_symbol						0x5284C
#define modules_verification_symbol						0x5C484
#define authenticate_program_segment_symbol				0x5E2DC

#define prx_load_module_symbol							0x8D304
#define prx_start_module_symbol							0x8BFD0
#define prx_stop_module_symbol							0x8D3A8
#define prx_unload_module_symbol						0x8BD04
#define prx_get_module_info_symbol						0x8B6FC
#define prx_get_module_id_by_address_symbol				0x8B60C
#define prx_get_module_id_by_name_symbol				0x8B65C
#define prx_get_module_list_symbol						0x8B77C
#define load_module_by_fd_symbol						0x8C934
#define parse_sprx_symbol								0x8A628
#define open_prx_object_symbol							0x835D0
#define close_prx_object_symbol							0x83EE0
#define lock_prx_mutex_symbol							0x8B5B4
#define unlock_prx_mutex_symbol							0x8B5C0

#define extend_kstack_symbol							0x73BF0

#define get_pseudo_random_number_symbol					0x22E654
#define md5_reset_symbol								0x1681A4
#define md5_update_symbol								0x168C44
#define md5_final_symbol								0x168DC4
#define ss_get_open_psid_symbol							0x230B3C
#define update_mgr_read_eeprom_symbol					0x229F34
#define update_mgr_write_eeprom_symbol					0x229E68

#define ss_params_get_update_status_symbol				0x54458

#define syscall_table_symbol							0x38A368
#define syscall_call_offset								0x27DE6C

#define read_bdvd0_symbol								0x1AAAC4
#define read_bdvd1_symbol								0x1AC6F0
#define read_bdvd2_symbol								0x1B98A8

#define storage_internal_get_device_object_symbol		0x2898F0

#define hid_mgr_read_usb_symbol	    					0x107F40
#define hid_mgr_read_bt_symbol							0x101E18

#define bt_set_controller_info_internal_symbol			0xF5E84

/* Calls, jumps */
#define device_event_port_send_call						0x296B84

#define ss_pid_call_1									0x219744

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x834C4
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x8D1A0

#define shutdown_copy_params_call						0xAB4C

#define fsloop_open_call								0x2B28E0
#define fsloop_close_call								0x2B2930
#define fsloop_read_call								0x2B2970

/* Patches */
#define shutdown_patch_offset							0xAB38
#define module_sdk_version_patch_offset					0x26D820
#define module_add_parameter_to_parse_sprxpatch_offset	0x8CA10

#define user_thread_prio_patch							0x21EA8
#define user_thread_prio_patch2							0x21EB4

#define lic_patch										0x59BFC
#define ode_patch										0x22DAC8

/* Rtoc entries */

#define io_rtoc_entry_1									0x2EB8
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x65C0
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x22A8

#define device_event_rtoc_entry_1						0x2630

#define time_rtoc_entry_1								-0x75E0
#define time_rtoc_entry_2								-0x75E8

#define thread_rtoc_entry_1								-0x7660

#define process_rtoc_entry_1							-0x77A0

#define bt_rtoc_entry_1									-0x3558

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x26DAC
#define permissions_exception2							0xC8A00
#define permissions_exception3							0x21F7C

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x4098C0
#define patch_func2_offset 								0x5D77C
#define patch_func8_offset1 							0x59E7C
#define patch_func8_offset2 							0x59FE0
#define patch_func9_offset 								0x5E0C4
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x3A48B0

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
