# cmd_vfs

Basic console commands to examine contents of filesystem in esp32.

Commands: ls, mkdir, rmdir, unlink, rename, copy, head, tail, uptime

```
>ls /littlefs
file    1537 2025.02.27 17:06:13 /littlefs/master.log
>mkdir /littlefs/test
Directory /littlefs/test created.
>ls /littlefs
file    1537 2025.02.27 17:06:13 /littlefs/master.log
dir  1073539744 1970.01.01 02:59:59 /littlefs/test
>copy /littlefs/master.log /littlefs/test/logfile.log
Content of file /littlefs/master.log copied to /littlefs/test/logfile.log.
>ls /littlefs/test
file    1537 2025.02.27 17:12:10 /littlefs/test/logfile.log
>unlink /littlefs/test/logfile.log
File /littlefs/test/logfile.log deleted.
>rename /littlefs/test /littlefs/test2
New name of /littlefs/test is /littlefs/test2.
>rmdir /littlefs/test2
Directory /littlefs/test2 deleted.
>ls /littlefs
file    1537 2025.02.27 17:06:13 /littlefs/master.log
>head /littlefs/master.log
1970.01.01 03:00:01.446419 ...
>tail /littlefs/master.log
 ...
2025.02.27 17:06:13.090470 ...
>uptime
Uptime is 0 days 00:10:25.
```
#Howto use
Add to idf_component.yml
```
  uqfus/cmd_vfs_basic:
    git: https://github.com/uqfus/cmd_vfs_basic.git
```
To register all console commands add
```
// component cmd_vfs https://github.com/uqfus/cmd_vfs_basic.git
 #include "cmd_vfs_basic.h"
esp_console_config_t console_config = ESP_CONSOLE_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_console_init(&console_config));
esp_console_register_help_command(); // Default 'help' command prints the list of registered commands
console_register_vfs_basic_func(); // cmd_vfs https://github.com/uqfus/cmd_vfs_basic.git
```
or register only needed
```
console_register_vfs_ls();
console_register_vfs_mkdir();
console_register_vfs_rmdir();
console_register_vfs_unlink();
console_register_vfs_rename();
console_register_vfs_copy();
console_register_vfs_head();
console_register_vfs_tail();
console_register_vfs_uptime();
```
