#ifndef __PERMISSIONS_H__
#define __PERMISSIONS_H__

extern uint32_t access_pid;

//int  is_system_process(process_t process);
void permissions_patches(void);

// Syscalls
int sys_permissions_get_access(void);
int sys_permissions_remove_access(void);

//void unhook_all_permissions(void);

#endif

