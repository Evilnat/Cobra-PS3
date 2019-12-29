/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_CEX)

#define TOC 											0x34B2E0

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
#define process_read_memory_symbol						0x270564
#define process_write_memory_symbol						0x2703D8
#define page_allocate_symbol							0x5EAE8
#define page_free_symbol								0x5E54C
#define page_export_to_proc_symbol						0x5EC84
#define page_unexport_from_proc_symbol					0x5E440
#define kernel_ea_to_lpar_addr_symbol					0x6E86C
#define process_ea_to_lpar_addr_ex_symbol				0x75E80
#define set_pte_symbol                      			0x5CAE4
#define map_process_memory_symbol						0x7598C
#define panic_symbol									0x275C10

#define memcpy_symbol									0x7D04C
#define memset_symbol									0x4D494
#define memcmp_symbol									0x4C7A4
#define memchr_symbol									0x4C754
#define printf_symbol									0x278CE0
#define printfnull_symbol								0x27D750
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

#define spin_lock_irqsave_ex_symbol						0x275DE0
#define spin_unlock_irqrestore_ex_symbol				0x275DB4

#define create_process_common_symbol					0x273BD8
#define create_process_base_symbol						0x273294
#define load_process_symbol								0x5004
#define process_kill_symbol								0x2739E4

#define ppu_thread_create_symbol						0x13F2C
#define ppu_thread_exit_symbol							0x13FE4
#define ppu_thread_join_symbol							0x14038
#define ppu_thread_delay_symbol							0x285CC
#define create_kernel_thread_symbol						0x24950
#define create_user_thread_symbol						0x2508C
#define create_user_thread2_symbol						0x24EB0
#define start_thread_symbol								0x23B7C
#define run_thread_symbol								0x233AC
#define register_thread_symbol							0x26FFF0
#define allocate_user_stack_symbol						0x2707D8
#define deallocate_user_stack_symbol					0x270740

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

#define cellFsOpen_symbol								0x29E1C0
#define cellFsClose_symbol								0x29E028
#define cellFsRead_symbol								0x29E164
#define cellFsWrite_symbol								0x29E0D0
#define cellFsLseek_symbol								0x29D758
#define cellFsStat_symbol								0x29D9DC
#define cellFsUtime_symbol 								0x29F4C8
#define cellFsUnlink_internal_symbol   		 			0x19C350
#define cellFsRename_internal_symbol   		 			0x196E6C

#define cellFsUtilMount_symbol							0x29D538
#define cellFsUtilUmount_symbol 						0x29D50C
#define cellFsUtilNewfs_symbol							0x29EF60

#define pathdup_from_user_symbol						0x1A2244
#define open_path_symbol								0x29DEF8
#define open_fs_object_symbol							0x18A6F0
#define close_fs_object_symbol							0x18962C

#define storage_get_device_info_symbol					0x282130
#define storage_get_device_config_symbol				0x2815C0
#define storage_open_symbol								0x281B40
#define storage_close_symbol							0x281930
#define storage_read_symbol								0x280EA0
#define storage_write_symbol							0x280D70
#define storage_send_device_command_symbol				0x280A2C
#define storage_map_io_memory_symbol					0x281FEC
#define storage_unmap_io_memory_symbol		    		0x281EB8
#define new_medium_listener_object_symbol				0x95AF4
#define delete_medium_listener_object_symbol			0x9732C
#define set_medium_event_callbacks_symbol				0x97690

#define cellUsbdRegisterLdd_symbol						0x268924
#define cellUsbdUnregisterLdd_symbol					0x2688D4
#define cellUsbdScanStaticDescriptor_symbol				0x269B24
#define cellUsbdOpenPipe_symbol			        		0x269BD4
#define cellUsbdClosePipe_symbol						0x269B84
#define cellUsbdControlTransfer_symbol					0x269A88
#define cellUsbdBulkTransfer_symbol						0x269A08

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

#define get_pseudo_random_number_symbol					0x230B04
#define md5_reset_symbol								0x161BA0
#define md5_update_symbol								0x162640
#define md5_final_symbol								0x1627C0
#define ss_get_open_psid_symbol							0x232FAC
#define update_mgr_read_eeprom_symbol					0x22C610
#define update_mgr_write_eeprom_symbol					0x22C558

/* RAP Activation */
#define SHA1_init_symbol								0x2CB9B0
#define SHA1_update_symbol								0x2CBA10
#define SHA1_final_symbol								0x2CBBC4
#define aescbccfb_dec_symbol							0x2C9EA4
#define aescbccfb_enc_symbol							0x2C9C54

#define ss_params_get_update_status_symbol				0x508DC

#define syscall_table_symbol							0x35F300
#define syscall_call_offset								0x27E234

#define read_bdvd0_symbol								0x1AD1B4
#define read_bdvd1_symbol								0x1AEDE0
#define read_bdvd2_symbol								0x1BBF98

#define storage_internal_get_device_object_symbol		0x2804E4

#define hid_mgr_read_usb_symbol	    					0x102490
#define hid_mgr_read_bt_symbol							0xFC368

#define bt_set_controller_info_internal_symbol			0xF03D4

/* Calls, jumps */
#define device_event_port_send_call						0x28A69C

#define ss_pid_call_1									0x21BE34

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x7DAFC
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x870D0

#define shutdown_copy_params_call						0xAABC

#define fsloop_open_call								0x29E358
#define fsloop_close_call								0x29E3A8
#define fsloop_read_call								0x29E3E8

/* Patches */
#define shutdown_patch_offset							0xAAA8
#define module_sdk_version_patch_offset					0x26F820
#define module_add_parameter_to_parse_sprxpatch_offset	0x86940

#define user_thread_prio_patch							0x2022C
#define user_thread_prio_patch2							0x20238

#define lic_patch										0x55D8C
#define ode_patch										0x23010C

/* Rtoc entries */

#define io_rtoc_entry_1									-0x1E8
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x6688
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x1DC8

#define device_event_rtoc_entry_1						0x2038

#define time_rtoc_entry_1								-0x7640
#define time_rtoc_entry_2								-0x7648

#define thread_rtoc_entry_1								-0x76C0

#define process_rtoc_entry_1							-0x7800

#define bt_rtoc_entry_1									-0x3670

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x24F30
#define permissions_exception2							0xC3574
#define permissions_exception3							0x20300

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x3DE440
#define patch_func2_offset 								0x5920C
#define patch_func8_offset1 							0x55FE4
#define patch_func8_offset2 							0x56148
#define patch_func9_offset 								0x59B0C
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x379430

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x6F698

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
