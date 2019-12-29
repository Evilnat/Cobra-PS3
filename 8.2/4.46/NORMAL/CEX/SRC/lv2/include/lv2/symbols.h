/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_CEX)

#define TOC 											0x348DF0

#define create_kernel_object_symbol						0x12014
#define destroy_kernel_object_symbol					0x11978
#define destroy_shared_kernel_object_symbol				0x112EC
#define open_kernel_object_symbol						0x12664
#define open_shared_kernel_object_symbol				0x12474
#define close_kernel_object_handle_symbol				0x11A94

#define alloc_symbol									0x62F78
#define dealloc_symbol									0x633B4
#define copy_to_user_symbol								0xF85C
#define copy_from_user_symbol							0xFA78
#define copy_to_process_symbol							0xF914
#define copy_from_process_symbol						0xF724
#define process_read_memory_symbol						0x298084
#define process_write_memory_symbol						0x297EF8
#define page_allocate_symbol							0x5EAE8
#define page_free_symbol								0x5E54C
#define page_export_to_proc_symbol						0x5EC84
#define page_unexport_from_proc_symbol					0x5E440
#define kernel_ea_to_lpar_addr_symbol					0x6E86C
#define process_ea_to_lpar_addr_ex_symbol				0x75E80
#define set_pte_symbol                      			0x5CAE4
#define map_process_memory_symbol						0x7598C
#define panic_symbol									0x29D730

#define memcpy_symbol									0x7D04C
#define memset_symbol									0x4D494
#define memcmp_symbol									0x4C7A4
#define memchr_symbol									0x4C754
#define printf_symbol									0x2A0800
#define printfnull_symbol								0x2A5270
#define sprintf_symbol				    				0x4E8BC
#define snprintf_symbol				    				0x4E828
#define strcpy_symbol									0x4D640
#define strncpy_symbol									0x4D708
#define strlen_symbol									0x4D668
#define strcat_symbol									0x4D570
#define strcmp_symbol									0x4D5EC
#define strncmp_symbol									0x4D694
#define strchr_symbol									0x4D5A8
#define strrchr_symbol									0x4D778

#define spin_lock_irqsave_ex_symbol						0x29D900
#define spin_unlock_irqrestore_ex_symbol				0x29D8D4

#define create_process_common_symbol					0x29B6F8
#define create_process_base_symbol						0x29ADB4
#define load_process_symbol								0x5004
#define process_kill_symbol								0x29B504

#define ppu_thread_create_symbol						0x13F2C
#define ppu_thread_exit_symbol							0x13FE4
#define ppu_thread_join_symbol							0x14038
#define ppu_thread_delay_symbol							0x285CC
#define create_kernel_thread_symbol						0x24950
#define create_user_thread_symbol						0x2508C
#define create_user_thread2_symbol						0x24EB0
#define start_thread_symbol								0x23B7C
#define run_thread_symbol								0x233AC
#define register_thread_symbol							0x297B10
#define allocate_user_stack_symbol						0x2982F8
#define deallocate_user_stack_symbol					0x298260

#define mutex_create_symbol								0x136A0
#define mutex_destroy_symbol							0x13638
#define mutex_lock_symbol								0x13630
#define mutex_lock_ex_symbol							0x1D98C
#define mutex_trylock_symbol							0x1362C
#define mutex_unlock_symbol								0x13628

#define cond_create_symbol								0x1380C
#define cond_destroy_symbol								0x137BC
#define cond_wait_symbol								0x137B4
#define cond_wait_ex_symbol								0x1EF78
#define cond_signal_symbol								0x13790
#define cond_signal_all_symbol							0x1376C

#define semaphore_initialize_symbol						0x30A7C
#define semaphore_wait_ex_symbol						0x30784
#define semaphore_trywait_symbol						0x3036C
#define semaphore_post_ex_symbol						0x304B8

#define event_port_create_symbol						0x130DC
#define event_port_destroy_symbol						0x13544
#define event_port_connect_symbol						0x135BC
#define event_port_disconnect_symbol					0x134E8
#define event_port_send_symbol							0x130D4
#define event_port_send_ex_symbol						0x2B02C

#define event_queue_create_symbol						0x133E4
#define event_queue_destroy_symbol						0x1336C
#define event_queue_receive_symbol						0x131B0
#define event_queue_tryreceive_symbol					0x1327C

#define cellFsOpen_symbol								0x2C4A78
#define cellFsClose_symbol								0x2C48E0
#define cellFsRead_symbol								0x2C4A1C
#define cellFsWrite_symbol								0x2C4988
#define cellFsLseek_symbol								0x2C4010
#define cellFsStat_symbol								0x2C4294
#define cellFsUtime_symbol 								0x2C5CB0
#define cellFsUnlink_internal_symbol   		 			0x1AAF70
#define cellFsRename_internal_symbol   		 			0x1A5F7C

#define cellFsUtilMount_symbol							0x2C3DF0
#define cellFsUtilUmount_symbol 						0x2C3DC4
#define cellFsUtilNewfs_symbol							0x2C5748

#define pathdup_from_user_symbol						0x1B1000
#define open_path_symbol								0x2C47B0
#define open_fs_object_symbol							0x190420
#define close_fs_object_symbol							0x18F41C

#define storage_get_device_info_symbol					0x2A9C50
#define storage_get_device_config_symbol				0x2A90E0
#define storage_open_symbol								0x2A9660
#define storage_close_symbol							0x2A9450
#define storage_read_symbol								0x2A89C0
#define storage_write_symbol							0x2A8890
#define storage_send_device_command_symbol				0x2A854C
#define storage_map_io_memory_symbol					0x2A9B0C
#define storage_unmap_io_memory_symbol		    		0x2A99D8
#define new_medium_listener_object_symbol				0x95AF4
#define delete_medium_listener_object_symbol			0x9732C
#define set_medium_event_callbacks_symbol				0x97690

#define cellUsbdRegisterLdd_symbol						0x290444
#define cellUsbdUnregisterLdd_symbol					0x2903F4
#define cellUsbdScanStaticDescriptor_symbol				0x291644
#define cellUsbdOpenPipe_symbol			        		0x2916F4
#define cellUsbdClosePipe_symbol						0x2916A4
#define cellUsbdControlTransfer_symbol					0x2915A8
#define cellUsbdBulkTransfer_symbol						0x291528

#define decrypt_func_symbol								0x34798
#define lv1_call_99_wrapper_symbol						0x4ECD0
#define modules_verification_symbol						0x58828
#define authenticate_program_segment_symbol				0x59C64

#define prx_load_module_symbol							0x87234
#define prx_start_module_symbol							0x85F00
#define prx_stop_module_symbol							0x872D8
#define prx_unload_module_symbol						0x85C34
#define prx_get_module_info_symbol						0x856BC
#define prx_get_module_id_by_address_symbol				0x855CC
#define prx_get_module_id_by_name_symbol				0x8561C
#define prx_get_module_list_symbol						0x8573C
#define load_module_by_fd_symbol						0x86864
#define parse_sprx_symbol								0x845E8
#define open_prx_object_symbol							0x7DC08
#define close_prx_object_symbol							0x7E518
#define lock_prx_mutex_symbol							0x85574
#define unlock_prx_mutex_symbol							0x85580

#define extend_kstack_symbol							0x6E7C4

#define get_pseudo_random_number_symbol					0x2587A0
#define md5_reset_symbol								0x162DB0
#define md5_update_symbol								0x163850
#define md5_final_symbol								0x1639D0
#define ss_get_open_psid_symbol							0x25AB68
#define update_mgr_read_eeprom_symbol					0x2542AC
#define update_mgr_write_eeprom_symbol					0x2541F4

/* RAP Activation */
#define SHA1_init_symbol								0x2C9A38
#define SHA1_update_symbol								0x2C9A98
#define SHA1_final_symbol								0x2C9C4C
#define aescbccfb_dec_symbol							0x2C7F2C
#define aescbccfb_enc_symbol							0x2C7CDC

#define ss_params_get_update_status_symbol				0x508DC

#define syscall_table_symbol							0x35E860
#define syscall_call_offset								0x2A5D54

#define read_bdvd0_symbol								0x1BBF70
#define read_bdvd1_symbol								0x1BDB9C
#define read_bdvd2_symbol								0x1CAD54

#define storage_internal_get_device_object_symbol		0x2A8004

#define hid_mgr_read_usb_symbol	    					0x1037EC
#define hid_mgr_read_bt_symbol							0xFD6C4

#define bt_set_controller_info_internal_symbol			0xF1730

/* Calls, jumps */
#define device_event_port_send_call						0x2B2184

#define ss_pid_call_1									0x243AD0

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x7DAFC
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x870D0

#define shutdown_copy_params_call						0xAABC

#define fsloop_open_call								0x2C4C10
#define fsloop_close_call								0x2C4C60
#define fsloop_read_call								0x2C4CA0

/* Patches */
#define shutdown_patch_offset							0xAAA8
#define module_sdk_version_patch_offset					0x297340
#define module_add_parameter_to_parse_sprxpatch_offset	0x86940

#define user_thread_prio_patch							0x2022C
#define user_thread_prio_patch2							0x20238

#define lic_patch										0x55D8C
#define ode_patch										0x257DA8

/* Rtoc entries */

#define io_rtoc_entry_1									-0x150
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x6688
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x1E98

#define device_event_rtoc_entry_1						0x2108

#define time_rtoc_entry_1								-0x7640
#define time_rtoc_entry_2								-0x7648

#define thread_rtoc_entry_1								-0x76C0

#define process_rtoc_entry_1							-0x7800

#define bt_rtoc_entry_1									-0x35D8

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x24F30
#define permissions_exception2							0xC3554
#define permissions_exception3							0x20300

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x3DBE40
#define patch_func2_offset 								0x5920C
#define patch_func8_offset1 							0x55FE4
#define patch_func8_offset2 							0x56148
#define patch_func9_offset 								0x59B0C
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x376E30

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x6F698

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
