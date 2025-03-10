#ifndef __HOMEBREW_BLOCKER_H__
#define __HOMEBREW_BLOCKER_H__

#define NPSIGNIN_LOCK  			"/dev_flash/vsh/resource/npsignin_plugin.lck"
#define NPSIGNIN_UNLOCK 		"/dev_flash/vsh/resource/npsignin_plugin.rco"

int ps3mapi_partial_disable_syscall8;
extern uint8_t allow_restore_sc; // allow re-create cfw syscalls accessing system update on XMB
extern uint8_t syscalls_mode; // disable CFW syscalls on boot

int block_homebrew(const char *path);
void restore_syscalls(const char *path);
void check_signin(const char *path);
void disable_cfw_syscalls(int mode);

#endif /* __HOMEBREW_BLOCKER_H__ */
