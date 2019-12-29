/* Keep this file language agnostic. Only preprocessor here. */

#ifndef __FIRMWARE_SYMBOLS_H_S__
#define __FIRMWARE_SYMBOLS_H_S__

#if defined(FIRMWARE_CEX)

#define TOC 											0x34E620

#define create_kernel_object_symbol						0x12024
#define destroy_kernel_object_symbol					0x11988
#define destroy_shared_kernel_object_symbol				0x112FC
#define open_kernel_object_symbol						0x12674
#define open_shared_kernel_object_symbol				0x12484
#define close_kernel_object_handle_symbol				0x11AA4

#define alloc_symbol									0x643B4
#define dealloc_symbol									0x647F0
#define copy_to_user_symbol								0xF86C
#define copy_from_user_symbol							0xFA88
#define copy_to_process_symbol							0xF924
#define copy_from_process_symbol						0xF734
#define process_read_memory_symbol						0x271DB0
#define process_write_memory_symbol						0x271C24
#define page_allocate_symbol							0x5FF24
#define page_free_symbol								0x5F988
#define page_export_to_proc_symbol						0x600C0
#define page_unexport_from_proc_symbol					0x5F87C
#define kernel_ea_to_lpar_addr_symbol					0x6FCA8
#define process_ea_to_lpar_addr_ex_symbol				0x772BC
#define set_pte_symbol                      			0x5DF20
#define map_process_memory_symbol						0x76DC8
#define panic_symbol									0x277460

#define memcpy_symbol									0x7E488
#define memset_symbol									0x4D6D8
#define memcmp_symbol									0x4C9E8
#define memchr_symbol									0x4C998
#define printf_symbol									0x27A530
#define printfnull_symbol								0x27EFA0
#define sprintf_symbol				    				0x4EB00
#define snprintf_symbol				    				0x4EA6C
#define strcpy_symbol									0x4D884
#define strncpy_symbol									0x4D94C
#define strlen_symbol									0x4D8AC
#define strcat_symbol									0x4D7B4
#define strcmp_symbol									0x4D830
#define strncmp_symbol									0x4D8D8
#define strchr_symbol									0x4D7EC
#define strrchr_symbol									0x4D9BC

#define spin_lock_irqsave_ex_symbol						0x277630
#define spin_unlock_irqrestore_ex_symbol				0x277604

#define create_process_common_symbol					0x275430
#define create_process_base_symbol						0x274AE0
#define load_process_symbol								0x5004
#define process_kill_symbol								0x27523C

#define ppu_thread_create_symbol						0x13F3C
#define ppu_thread_exit_symbol							0x13FF4
#define ppu_thread_join_symbol							0x14048
#define ppu_thread_delay_symbol							0x28810
#define create_kernel_thread_symbol						0x24B94
#define create_user_thread_symbol						0x252D0
#define create_user_thread2_symbol						0x250F4
#define start_thread_symbol								0x23DC0
#define run_thread_symbol								0x235F0
#define register_thread_symbol							0x27183C
#define allocate_user_stack_symbol						0x272024
#define deallocate_user_stack_symbol					0x271F8C

#define mutex_create_symbol								0x136B0
#define mutex_destroy_symbol							0x13648
#define mutex_lock_symbol								0x13640
#define mutex_lock_ex_symbol							0x1DBD0
#define mutex_trylock_symbol							0x1363C
#define mutex_unlock_symbol								0x13638

#define cond_create_symbol								0x1381C
#define cond_destroy_symbol								0x137CC
#define cond_wait_symbol								0x137C4
#define cond_wait_ex_symbol								0x1F1BC
#define cond_signal_symbol								0x137A0
#define cond_signal_all_symbol							0x1377C

#define semaphore_initialize_symbol						0x30CC0
#define semaphore_wait_ex_symbol						0x309C8
#define semaphore_trywait_symbol						0x305B0
#define semaphore_post_ex_symbol						0x306FC

#define event_port_create_symbol						0x130EC
#define event_port_destroy_symbol						0x13554
#define event_port_connect_symbol						0x135CC
#define event_port_disconnect_symbol					0x134F8
#define event_port_send_symbol							0x130E4
#define event_port_send_ex_symbol						0x2B270

#define event_queue_create_symbol						0x133F4
#define event_queue_destroy_symbol						0x1337C
#define event_queue_receive_symbol						0x131C0
#define event_queue_tryreceive_symbol					0x1328C

#define cellFsOpen_symbol								0x29FA10
#define cellFsClose_symbol								0x29F878
#define cellFsRead_symbol								0x29F9B4
#define cellFsWrite_symbol								0x29F920
#define cellFsLseek_symbol								0x29EFA8
#define cellFsStat_symbol								0x29F22C
#define cellFsUtime_symbol 								0x2A0D18
#define cellFsUnlink_internal_symbol   		 			0x19D83C
#define cellFsRename_internal_symbol   		 			0x1982F4

#define cellFsUtilMount_symbol							0x29ED88
#define cellFsUtilUmount_symbol 						0x29ED5C
#define cellFsUtilNewfs_symbol							0x2A07B0

#define pathdup_from_user_symbol						0x1A3730
#define open_path_symbol								0x29F748
#define open_fs_object_symbol							0x18BB60
#define close_fs_object_symbol							0x18AA9C

#define storage_get_device_info_symbol					0x283980
#define storage_get_device_config_symbol				0x282E10
#define storage_open_symbol								0x283390
#define storage_close_symbol							0x283180
#define storage_read_symbol								0x2826F0
#define storage_write_symbol							0x2825C0
#define storage_send_device_command_symbol				0x28227C
#define storage_map_io_memory_symbol					0x28383C
#define storage_unmap_io_memory_symbol		    		0x283708
#define new_medium_listener_object_symbol				0x96F40
#define delete_medium_listener_object_symbol			0x98778
#define set_medium_event_callbacks_symbol				0x98ADC

#define cellUsbdRegisterLdd_symbol						0x26A16C
#define cellUsbdUnregisterLdd_symbol					0x26A11C
#define cellUsbdScanStaticDescriptor_symbol				0x26B36C
#define cellUsbdOpenPipe_symbol			        		0x26B41C
#define cellUsbdClosePipe_symbol						0x26B3CC
#define cellUsbdControlTransfer_symbol					0x26B2D0
#define cellUsbdBulkTransfer_symbol						0x26B250

#define decrypt_func_symbol								0x349DC
#define lv1_call_99_wrapper_symbol						0x4EF14
#define modules_verification_symbol						0x586E8
#define authenticate_program_segment_symbol				0x5A51C

#define prx_load_module_symbol							0x88680
#define prx_start_module_symbol							0x8734C
#define prx_stop_module_symbol							0x88724
#define prx_unload_module_symbol						0x87080
#define prx_get_module_info_symbol						0x86B08
#define prx_get_module_id_by_address_symbol				0x86A18
#define prx_get_module_id_by_name_symbol				0x86A68
#define prx_get_module_list_symbol						0x86B88
#define load_module_by_fd_symbol						0x87CB0
#define parse_sprx_symbol								0x85A34
#define open_prx_object_symbol							0x7F054
#define close_prx_object_symbol							0x7F964
#define lock_prx_mutex_symbol							0x869C0
#define unlock_prx_mutex_symbol							0x869CC

#define extend_kstack_symbol							0x6FC00

#define get_pseudo_random_number_symbol					0x2321D4
#define md5_reset_symbol								0x163004
#define md5_update_symbol								0x163AA4
#define md5_final_symbol								0x163C24
#define ss_get_open_psid_symbol							0x2346BC
#define update_mgr_read_eeprom_symbol					0x22DACC
#define update_mgr_write_eeprom_symbol					0x22DA14

/* RAP Activation */
#define SHA1_init_symbol								0x2CD1F8
#define SHA1_update_symbol								0x2CD258
#define SHA1_final_symbol								0x2CD40C
#define aescbccfb_dec_symbol							0x2CB6EC
#define aescbccfb_enc_symbol							0x2CB49C

#define ss_params_get_update_status_symbol				0x50B20

#define syscall_table_symbol							0x362680
#define syscall_call_offset								0x27FA84

#define read_bdvd0_symbol								0x1AE6A0
#define read_bdvd1_symbol								0x1B02CC
#define read_bdvd2_symbol								0x1BD454

#define storage_internal_get_device_object_symbol		0x281D34

#define hid_mgr_read_usb_symbol	    					0x1038F4
#define hid_mgr_read_bt_symbol							0xFD7CC

#define bt_set_controller_info_internal_symbol			0xF1838

/* Calls, jumps */
#define device_event_port_send_call						0x28BEEC

#define ss_pid_call_1									0x21D2F0

#define process_map_caller_call							0x4D24

#define read_module_header_call							0x7EF48
#define read_module_body_call							0x671C
#define load_module_by_fd_call1							0x8851C

#define shutdown_copy_params_call						0xAACC

#define fsloop_open_call								0x29FBA8
#define fsloop_close_call								0x29FBF8
#define fsloop_read_call								0x29FC38

/* Patches */
#define shutdown_patch_offset							0xAAB8
#define module_sdk_version_patch_offset					0x27106C
#define module_add_parameter_to_parse_sprxpatch_offset	0x87D8C

#define user_thread_prio_patch							0x20470
#define user_thread_prio_patch2							0x2047C

#define lic_patch										0x56024
#define ode_patch										0x231648

/* Rtoc entries */

#define io_rtoc_entry_1									-0x1B8
#define io_sub_rtoc_entry_1								-0x7EA0
#define decrypt_rtoc_entry_2 							-0x66A8
#define decrypter_data_entry							-0x7F10

#define storage_rtoc_entry_1							0x1E10

#define device_event_rtoc_entry_1						0x2080

#define time_rtoc_entry_1								-0x7640
#define time_rtoc_entry_2								-0x7648

#define thread_rtoc_entry_1								-0x76C0

#define process_rtoc_entry_1							-0x7800

#define bt_rtoc_entry_1									-0x3640

/* Permissions */
#define permissions_func_symbol							0x3560
#define permissions_exception1							0x25174
#define permissions_exception2							0xC49D8
#define permissions_exception3							0x20544

/* Legacy patches with no names yet */
/* Kernel offsets */
#define patch_data1_offset								0x3E17C0
#define patch_func2_offset 								0x599BC
#define patch_func8_offset1 							0x562A4
#define patch_func8_offset2 							0x56408
#define patch_func9_offset 								0x5A304
#define mem_base2										0x3D90

/* vars */
#define thread_info_symbol								0x37C7B0

/* mmapper flags temp patch */
#define mmapper_flags_temp_patch						0x70AD4

#endif /* FIRMWARE */

#endif /* __FIRMWARE_SYMBOLS_H_S__ */
