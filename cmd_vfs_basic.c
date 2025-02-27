#include <fcntl.h>     // open O_RDONLY O_WRONLY O_CREAT
#include <dirent.h>    // opendir readdir dirent
#include <errno.h>     // strerror errno
#include <sys/stat.h>  // stat
#include "esp_console.h" // esp_console_cmd_register
#include "argtable3/argtable3.h" // argument parser
#include "esp_vfs.h"   // lseek close read write
#include "esp_timer.h" // esp_timer_get_time
//#include "esp_log.h"   // ESP_LOG_BUFFER_HEX

#include "cmd_vfs_basic.h"

// MARK: ls
static struct {
    struct arg_str *dir;
    struct arg_end *end;
} ls_args;

static int ls_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &ls_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, ls_args.end, argv[0]);
        return 0;
    }

    char entry_name[MAXNAMLEN+2];
    struct dirent *entry;
    struct stat entry_stat;
    DIR *dir = opendir(ls_args.dir->sval[0]);
    if (dir == NULL)
    {
        printf("Error occurred during opendir(%s) - %s.\n", ls_args.dir->sval[0], strerror(errno));
        printf("Dump all registered paths:\n");
        esp_vfs_dump_registered_paths(stdout);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(entry_name, sizeof(entry_name), "%s/%s", ls_args.dir->sval[0], entry->d_name);
        if (stat(entry_name, &entry_stat) == -1)
        {
            printf("Failed to stat(%s) - %s.\n", entry->d_name, strerror(errno));
            continue;
        }

        struct tm timeinfo;
        localtime_r(&entry_stat.st_mtim.tv_sec, &timeinfo);
        char timestr[20];
        strftime(timestr, sizeof(timestr), "%Y.%m.%d %H:%M:%S", &timeinfo); // 19 chars
        printf("%s %7ld %s %s\n", (entry->d_type == DT_DIR ? "dir " : "file"), entry_stat.st_size, timestr, entry_name);
//        ESP_LOG_BUFFER_HEX(__func__, &entry_stat, sizeof(entry_stat));
    }
    closedir(dir);
    return 0;
}

void console_register_vfs_ls(void)
{
    ls_args.dir = arg_str1(NULL, NULL, "</path/directory>", "full path to directory to be listed.");
    ls_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "ls",
        .help = "List directory contents.\nListing format:\n"
        "<dir/file> <size> <last modification timestamp> <full path name>",
        .hint = NULL,
        .func = &ls_func,
        .argtable = &ls_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: mkdir
static struct {
    struct arg_str *dir;
    struct arg_end *end;
} mkdir_args;

static int mkdir_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &mkdir_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, mkdir_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(mkdir_args.dir->sval[0], &entry_stat) == -1) // if not found
    {
        if (mkdir(mkdir_args.dir->sval[0], 0755) == -1)
        {
            printf("Failed to mkdir(%s) - %s.\n", mkdir_args.dir->sval[0], strerror(errno));
            return 0;
        }
        printf("Directory %s created.\n", mkdir_args.dir->sval[0]);
        return 0;
    }
    printf("Error. Directory or file %s already exist.\n", mkdir_args.dir->sval[0]);
    return 0;
}

void console_register_vfs_mkdir(void)
{
    mkdir_args.dir = arg_str1(NULL, NULL, "</path/directory>", "full path to directory to be created.");
    mkdir_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "mkdir",
        .help = "Create the directory.",
        .hint = NULL,
        .func = &mkdir_func,
        .argtable = &mkdir_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: rmdir
static struct {
    struct arg_str *dir;
    struct arg_end *end;
} rmdir_args;

static int rmdir_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &rmdir_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, rmdir_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(rmdir_args.dir->sval[0], &entry_stat) == 0) // if found
    {
        if (rmdir(rmdir_args.dir->sval[0]) == -1)
        {
            printf("Failed to rmdir(%s) - %s.\n", rmdir_args.dir->sval[0], strerror(errno));
            return 0;
        }
        printf("Directory %s deleted.\n", rmdir_args.dir->sval[0]);
        return 0;
    }
    printf("Error. Directory %s not exist.\n", rmdir_args.dir->sval[0]);
    return 0;
}

void console_register_vfs_rmdir(void)
{
    rmdir_args.dir = arg_str1(NULL, NULL, "</path/directory>", "full path to directory to be deleted.");
    rmdir_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "rmdir",
        .help = "Delete empty directory.",
        .hint = NULL,
        .func = &rmdir_func,
        .argtable = &rmdir_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: unlink
static struct {
    struct arg_str *dir;
    struct arg_end *end;
} unlink_args;

static int unlink_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &unlink_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, unlink_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(unlink_args.dir->sval[0], &entry_stat) == 0) // if found
    {
        if (unlink(unlink_args.dir->sval[0]) == -1)
        {
            printf("Failed to unlink(%s) - %s.\n", unlink_args.dir->sval[0], strerror(errno));
            return 0;
        }
        printf("File %s deleted.\n", unlink_args.dir->sval[0]);
        return 0;
    }
    printf("Error. File %s not exist.\n", unlink_args.dir->sval[0]);
    return 0;
}

void console_register_vfs_unlink(void)
{
    unlink_args.dir = arg_str1(NULL, NULL, "</path/filename>", "name of file to be deleted with full path.");
    unlink_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "unlink",
        .help = "Remove the specified file.",
        .hint = NULL,
        .func = &unlink_func,
        .argtable = &unlink_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: rename
static struct {
    struct arg_str *src;
    struct arg_str *dest;
    struct arg_end *end;
} rename_args;

static int rename_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &rename_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, rename_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(rename_args.src->sval[0], &entry_stat) == 0) // if found
    {
        if (rename(rename_args.src->sval[0], rename_args.dest->sval[0]) == -1)
        {
            printf("Failed to rename(%s,%s) - %s.\n", rename_args.src->sval[0], rename_args.dest->sval[0], strerror(errno));
            return 0;
        }
        printf("New name of %s is %s.\n", rename_args.src->sval[0], rename_args.dest->sval[0]);
        return 0;
    }
    printf("Error %s not exist.\n", rename_args.src->sval[0]);
    return 0;
}

void console_register_vfs_rename(void)
{
    rename_args.src = arg_str1(NULL, NULL, "</path/name>", "name of file/directory to be renamed with full path.");
    rename_args.dest = arg_str1(NULL, NULL, "</path/newname>", "new name of file/directory with full path.");
    rename_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "rename",
        .help = "Change the name and/or location of a file. Rename directory.",
        .hint = NULL,
        .func = &rename_func,
        .argtable = &rename_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: copy
static struct {
    struct arg_str *src;
    struct arg_str *dest;
    struct arg_end *end;
} copy_args;

static int copy_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &copy_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, copy_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(copy_args.src->sval[0], &entry_stat) == -1) // if not found
    {
        printf("Error. Source file %s not exist.\n", copy_args.src->sval[0]);
        return 0;
    }
    if (stat(copy_args.dest->sval[0], &entry_stat) == 0) // if found
    {
        printf("Error. Destination file %s already exist.\n", copy_args.dest->sval[0]);
        return 0;
    }

    int src = open(copy_args.src->sval[0], O_RDONLY);
    if (src == -1)
    {
        printf("Failed to open source file %s - %s.\n", copy_args.src->sval[0], strerror(errno));
        return 0;
    }
    int dest = open(copy_args.dest->sval[0], O_WRONLY | O_CREAT, 0644);
    if (dest == -1)
    {
        close(src);
        printf("Failed to open destination file %s - %s.\n", copy_args.dest->sval[0], strerror(errno));
        return 0;
    }

    const size_t BUFSIZE = 4096;
    char *buf = malloc(BUFSIZE);
    if (buf == NULL)
    {
        close(src);
        close(dest);
        printf("Failed to allocate memory.\n");
        return 0;
    }
    int readed;
    do {
        readed = read(src, buf, BUFSIZE);
        write(dest, buf, readed);
    } while (readed > 0);
    free(buf);
    close(src);
    close(dest);

    printf("Content of file %s copied to %s.\n", copy_args.src->sval[0], copy_args.dest->sval[0]);
    return 0;
}

void console_register_vfs_copy(void)
{
    copy_args.src = arg_str1(NULL, NULL, "</path/source>", "name of source file to be copied with full path.");
    copy_args.dest = arg_str1(NULL, NULL, "</path/destination>", "name of destination file with full path.");
    copy_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "copy",
        .help = "Copy content of a source file to a new file.",
        .hint = NULL,
        .func = &copy_func,
        .argtable = &copy_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: head
static struct {
    struct arg_str *src;
    struct arg_end *end;
} head_args;

static int head_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &head_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, head_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(head_args.src->sval[0], &entry_stat) == -1) // if not found
    {
        printf("Error. Source file %s not exist.\n", head_args.src->sval[0]);
        return 0;
    }

    int src = open(head_args.src->sval[0], O_RDONLY);
    if (src == -1)
    {
        printf("Failed to open source file %s - %s.\n", head_args.src->sval[0], strerror(errno));
        return 0;
    }

    const size_t BUFSIZE = 1024;
    char buf[BUFSIZE+1];
    int readed = read(src, buf, BUFSIZE);
    buf[readed] = 0;
    printf(buf);
    close(src);
    printf("\n");
    return 0;
}

void console_register_vfs_head(void)
{
    head_args.src = arg_str1(NULL, NULL, "</path/filename>", "name of file to be printed with full path.");
    head_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "head",
        .help = "Print head content of the specified file to console.",
        .hint = NULL,
        .func = &head_func,
        .argtable = &head_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: tail
static struct {
    struct arg_str *src;
    struct arg_end *end;
} tail_args;

static int tail_func(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &tail_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, tail_args.end, argv[0]);
        return 0;
    }

    struct stat entry_stat;
    if (stat(tail_args.src->sval[0], &entry_stat) == -1) // if not found
    {
        printf("Error. Source file %s not exist.\n", tail_args.src->sval[0]);
        return 0;
    }

    int src = open(tail_args.src->sval[0], O_RDONLY);
    if (src == -1)
    {
        printf("Failed to open source file %s - %s.\n", tail_args.src->sval[0], strerror(errno));
        return 0;
    }

    const size_t BUFSIZE = 1024;
    char buf[BUFSIZE+1];
    lseek(src, entry_stat.st_size > BUFSIZE ? entry_stat.st_size - BUFSIZE : 0, SEEK_SET);
    int readed = read(src, buf, BUFSIZE);
    buf[readed] = 0;
    printf(buf);
    close(src);
    printf("\n");
    return 0;
}

void console_register_vfs_tail(void)
{
    tail_args.src = arg_str1(NULL, NULL, "</path/filename>", "name of file to be printed with full path.");
    tail_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "tail",
        .help = "Print tail content of the specified file to console.",
        .hint = NULL,
        .func = &tail_func,
        .argtable = &tail_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: uptime
static int uptime_func(int argc, char **argv)
{
    uint64_t uptime = esp_timer_get_time() / 1000000; // seconds since boot
    uint16_t uptime_days =  uptime / (24 * 60 * 60);
    uptime =                uptime % (24 * 60 * 60);
    uint16_t uptime_hours = uptime / (60 * 60);
    uptime =                uptime % (60 * 60);
    uint16_t uptime_mins =  uptime / 60;
    uint16_t uptime_secs =  uptime % 60;
    printf("Uptime is %u days %02u:%02u:%02u.\n", uptime_days, uptime_hours, uptime_mins, uptime_secs);
    return 0;
}

void console_register_vfs_uptime(void)
{
    const esp_console_cmd_t cmd = {
        .command = "uptime",
        .help = "Tell how long the system has been running.",
        .hint = NULL,
        .func = &uptime_func,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// MARK: register func
void console_register_vfs_basic_func(void)
{
    console_register_vfs_ls();
    console_register_vfs_mkdir();
    console_register_vfs_rmdir();
    console_register_vfs_unlink();
    console_register_vfs_rename();
    console_register_vfs_copy();
    console_register_vfs_head();
    console_register_vfs_tail();
    console_register_vfs_uptime();
    return;
}