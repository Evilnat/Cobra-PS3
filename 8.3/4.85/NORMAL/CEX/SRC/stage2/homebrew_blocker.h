#ifndef __HOMEBREW_BLOCKER_H__
#define __HOMEBREW_BLOCKER_H__

extern uint8_t allow_restore_sc; // allow re-create cfw syscalls accessing system update on XMB

int block_homebrew(const char *path);
void restore_syscalls(const char *path);

#endif /* __HOMEBREW_BLOCKER_H__ */
