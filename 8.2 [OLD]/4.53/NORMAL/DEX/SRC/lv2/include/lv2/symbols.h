/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_DEX)

#define TOC 											0x370620

#define create_kernel_object_symbol						0x1265C
#define destroy_kernel_object_symbol					0x11FC0
#define destroy_shared_kernel_object_symbol				0x11934
#define open_kernel_object_symbol						0x12CAC
#define open_shared_kernel_object_symbol				0x12ABC
#define close_kernel_object_handle_symbol				0x120DC

#define alloc_symbol									0x66948
#define dealloc_symbol									0x66D84
#define copy_to_user_symbol								0xFEA4
#define copy_from_user_symbol							0x100C0
#define copy_to_process_symbol							0xFF5C
#define copy_from_process_symbol						0xFD6C
#define process_read_memory_symbol						0x276E88
#define process_write_memory_symbol						0x276CFC
#define page_allocate_symbol							0x624B8
#define page_free_symbol								0x61F1C
#define page_export_to_proc_symbol						0x62654
#define page_unexport_from_proc_symbol					0x61E10
#define kernel_ea_to_lpar_addr_symbol					0x723BC
#define process_ea_to_lpar_addr_ex_symbol				0x79F5C
#define set_pte_symbol                      			0x604B4
#define map_process_memory_symbol						0x79A68
#define panic_symbol									0x27DB1C

#define memcpy_symbol									0x81128
#define memset_symbol									0x50E3C
#define memcmp_symbol									0x5014C
#define memchr_symbol									0x500FC
#define printf_symbol									0x280FE0
#define printfnull_symbol								0x285A64
#define sprintf_symbol				    				0x52264
#define snprintf_symbol				    				0x521D0
#define strcpy_symbol									0x50FE8
#define strncpy_symbol									0x510B0
#define strlen_symbol									0x51010
#define strcat_symbol									0x50F18
#define strcmp_symbol									0x50F94
#define strncmp_symbol									0x5103C
#define strchr_symbol									0x50F50
#define strrchr_symbol									0x51120

#define spin_lock_irqsave_ex_symbol						0x27DCEC
#define spin_unlock_irqrestore_ex_symbol				0x27DCC0

#define create_process_common_symbol					0x27AB00
#define create_process_base_symbol						0x27A1A4
#define load_process_symbol								0x5004
#define process_kill_symbol								0x27A900

#define ppu_thread_create_symbol						0x14660
#define ppu_thread_exit_symbol							0x14718
#define ppu_thread_join_symbol							0x1476C
#define ppu_thread_delay_symbol							0x2A510
#define create_kernel_thread_symbol						0x26600
#define create_user_thread_symbol						0x26D3C
#define create_user_thread2_symbol						0x26B60
#define start_thread_symbol								0x25698
#define run_thread_symbol								0x24E5C
#define register_thread_symbol							0x276914
#define allocate_user_stack_symbol						0x2770FC
#define deallocate_user_stack_symbol					0x277064

#define mutex_create_symbol								0x13CF0
#define mutex_destroy_symbol							0x13C88
#define mutex_lock_symbol								0x13C80
#define mutex_lock_ex_symbol							0x1F43C
#define mutex_trylock_symbol							0x13C7C
#define mutex_unlock_symbol								0x13C78

#define cond_create_symbol								0x13E5C
#define cond_destroy_symbol								0x13E0C
#define cond_wait_symbol								0x13E04
#define cond_wait_ex_symbol								0x20A28
#define cond_signal_symbol								0x13DE0
#define cond_signal_all_symbol							0x13DBC 

#define semaphore_initialize_symbol						0x3439C
#define semaphore_wait_ex_symbol						0x340A4
#define semaphore_trywait_symbol						0x33C8C
#define semaphore_post_ex_symbol						0x33DD8

#define event_port_create_symbol						0x1372C
#define event_port_destroy_symbol						0x13B94
#define event_port_connect_symbol						0x13C0C
#define event_port_disconnect_symbol					0x13B38
#define event_port_send_symbol							0x13724
#define event_port_send_ex_symbol						0x2D0F0

#define event_queue_create_symbol						0x13A34
#define event_queue_destroy_symbol						0x139BC
#define event_queue_receive_symbol						0x13800
#define event_queue_tryreceive_symbol					0x138CC

#define cellFsOpen_symbol								0x2B8688
#define cellFsClose_symbol								0x2B84F0
#define cellFsRead_symbol								0x2B862C
#define cellFsWrite_symbol								0x2B8598
#define cellFsLseek_symbol								0x2B7DEC
#define cellFsStat_symbol								0x2B7EA4
#define cellFsUtime_symbol 								0x2B9F68
#define cellFsUnlink_internal_symbol   		 			0x1A26DC
#define cellFsRename_internal_symbol   		 			0x19D1F8

#define cellFsUtilMount_symbol							0x2B7B60 
#define cellFsUtilUmount_symbol 						0x2B7B34 
#define cellFsUtilNewfs_symbol							0x2B94AC 

#define pathdup_from_user_symbol						0x1A86F4
#define open_path_symbol								0x2B83C0 
#define open_fs_object_symbol							0x190A7C
#define close_fs_object_symbol							0x18F9B8

#define storage_get_device_info_symbol					0x295064 
#define storage_get_device_config_symbol				0x2936F0 
#define storage_open_symbol								0x29528C 
#define storage_close_symbol							0x293A78 
#define storage_read_symbol								0x2929F4 
#define storage_write_symbol							0x2928C4 
#define storage_send_device_command_symbol				0x292514 
#define storage_map_io_memory_symbol					0x294F10 
#define storage_unmap_io_memory_symbol		    		0x294DCC 
#define new_medium_listener_object_symbol				0x9B264 
#define delete_medium_listener_object_symbol			0x9CA9C 
#define set_medium_event_callbacks_symbol				0x9CE00 

#define cellUsbdRegisterLdd_symbol						0x26EE00 
#define cellUsbdUnregisterLdd_symbol					0x26EDB0 
#define cellUsbdScanStaticDescriptor_symbol				0x270000 
#define cellUsbdOpenPipe_symbol			        		0x2700B0 
#define cellUsbdClosePipe_symbol						0x270060 
#define cellUsbdControlTransfer_symbol					0x26FF64
#define cellUsbdBulkTransfer_symbol						0x26FEE4

#define decrypt_func_symbol								0x380B8
#define lv1_call_99_wrapper_symbol						0x52678
#define modules_verification_symbol						0x5C1F8
#define authenticate_program_segment_symbol				0x5D634

#define prx_load_module_symbol							0x8B938
#define prx_start_module_symbol							0x8A604
#define prx_stop_module_symbol							0x8B9DC
#define prx_unload_module_symbol						0x8A338
#define prx_get_module_info_symbol						0x89D30
#define prx_get_module_id_by_address_symbol				0x89C40
#define prx_get_module_id_by_name_symbol				0x89C90
#define prx_get_module_list_symbol						0x89DB0
#define load_module_by_fd_symbol						0x8AF68
#define parse_sprx_symbol								0x88C5C
#define open_prx_object_symbol							0x81CE4
#define close_prx_object_symbol							0x825F4
#define lock_prx_mutex_symbol							0x89BE8
#define unlock_prx_mutex_symbol							0x89BF4

#define extend_kstack_symbol							0x72314

#define get_pseudo_random_number_symbol					0x236FE0
#define md5_reset_symbol								0x167F2C
#define md5_update_symbol								0x1689CC
#define md5_final_symbol								0x168B4C
#define ss_get_open_psid_symbol							0x239488
#define update_mgr_read_eeprom_symbol					0x232AD4
#define update_mgr_write_eeprom_symbol					0x232A08

/* RAP Activation */
#define SHA1_init_symbol								0x2ECAF0
#define SHA1_update_symbol								0x2ECB50
#define SHA1_final_symbol								0x2ECD04
#define aescbccfb_dec_symbol							0x2EAFE4
#define aescbccfb_enc_symbol							0x2EAD94

#define ss_params_get_update_status_symbol				0x54284

#define syscall_table_symbol							0x385108
#define syscall_call_offset								0x286548

#define read_bdvd0_symbol								0x1B3664
#define read_bdvd1_symbol								0x1B5290
#define read_bdvd2_symbol								0x1C2448

#define storage_internal_get_device_object_symbol		0x291FCC

#define hid_mgr_read_usb_symbol	    					0x107F94
#define hid_mgr_read_bt_symbol							0x101E6C

#define bt_set_controller_info_internal_symbol			0xF5ED8

/* Calls, jumps */
#define device_event_port_send_call						0x29F260

#define ss_pid_call_1									0x2222E4

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x81BD8
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x8B7D4

#define shutdown_copy_params_call						0xAB3C

#define fsloop_open_call								0x2B8820
#define fsloop_close_call								0x2B8870
#define fsloop_read_call								0x2B88B0

/* Patches */
#define shutdown_patch_offset							0xAB28
#define module_sdk_version_patch_offset					0x275F3C
#define module_add_parameter_to_parse_sprxpatch_offset	0x8B044

#define user_thread_prio_patch							0x21CDC
#define user_thread_prio_patch2							0x21CE8

#define lic_patch										0x5975C
#define ode_patch										0x2365E8

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
#define permissions_exception1							0x26BE0
#define permissions_exception2							0xC8CE4
#define permissions_exception3							0x21DB0

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x4045C0
#define patch_func2_offset 								0x5CBDC
#define patch_func8_offset1 							0x599B4
#define patch_func8_offset2 							0x59B18
#define patch_func9_offset 								0x5D4DC
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x39F5B0

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x73328

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
