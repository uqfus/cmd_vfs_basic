#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void console_register_vfs_ls(void);
void console_register_vfs_mkdir(void);
void console_register_vfs_rmdir(void);
void console_register_vfs_unlink(void);
void console_register_vfs_rename(void);
void console_register_vfs_copy(void);
void console_register_vfs_head(void);
void console_register_vfs_tail(void);
void console_register_vfs_uptime(void);

void console_register_vfs_basic_func(void);

#ifdef __cplusplus
}
#endif
