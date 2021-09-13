#include "util.h"
#include "dirent.h"

extern int system_call();
#define SYS_EXIT 1
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_READ 3
#define SYS_LSEEK 19
#define SYS_CLOSE 6
#define SYS_GETDENTS 141

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define SEEK_SET 0

#define O_RDONLY 0x000
#define O_RDRW 0x002
#define O_WRONLY 0x001
#define O_CREATE 0x040

/*file types*/
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

struct linux_dirent
{
    unsigned long d_ino;
    unsigned long d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen; /* Length of this linux_dirent */
    char d_name[];           /* Filename (null-terminated) */
} linux_dirent;

int write(int print_to, char *str)
{
    int retrurn_code = system_call(SYS_WRITE, print_to, str, strlen(str));
    return retrurn_code;
}

/*prints the dirent debug data*/
void write_dirent_debug(struct linux_dirent *dirent)
{
    write(STDERR, "Dirent Name: ");
    write(STDERR, (dirent->d_name));
    write(STDERR, "\nDirent Length: ");
    write(STDERR, itoa(dirent->d_reclen));
    write(STDERR, "\n");
}

/*prints the debug prints according to the op code that it gets*/
void write_debug(int op_code, int return_code)
{
    char *return_code_str = itoa(return_code);
    if (op_code == SYS_WRITE)
    {
        write(STDERR, "sys call ID: 4 = write, return code: ");
        write(STDERR, return_code_str);
    }
    else if (op_code == SYS_GETDENTS)
    {
        write(STDERR, "sys call ID: 141 = getdents, return code: ");
        write(STDERR, return_code_str);
    }
    else if (op_code == SYS_OPEN)
    {
        write(STDERR, "sys call ID: 5 = open, return code: ");
        write(STDERR, return_code_str);
    }
    write(STDERR, "\n");
}
/* gets a dirent and prints the file data*/
void print_file(struct linux_dirent *dirent, int is_debug_mode)
{
    write(STDOUT, "\nfile name: ");
    int return_code = write(STDOUT, dirent->d_name);
    write(STDOUT, "\n");
    if (is_debug_mode)
    {
        write_debug(SYS_WRITE, return_code);
        write_dirent_debug(dirent);
    }
}

char *get_file_type(int type)
{
    switch (type)
    {
    case (DT_REG):
        return "regular";
    case (DT_DIR):
        return "directory";
    case (DT_FIFO):
        return "FIFO";
    case (DT_SOCK):
        return "socket";
    case (DT_LNK):
        return "symlink";
    case (DT_BLK):
        return "block dev";
    case (DT_CHR):
        return "char dev";
    default:
        return "???";
    }
}

int main(int argc, char **argv)
{
    int is_p = 0;
    int i = 0;
    int is_debug_mode = 0;
    char *prefix;

    /*checks the "mode" that we're in (debug, -p...)*/
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "-D", 2) == 0)
            {
                is_debug_mode = 1;
            }
            if (strncmp(argv[i], "-p", 2) == 0)
            {
                is_p = 1;
                prefix = argv[i] + 2;
            }
        }
    }

    int fd = system_call(SYS_OPEN, ".", O_RDONLY, 0);
    if (is_debug_mode)
        write_debug(SYS_OPEN, fd);
    int buf_length = 8192;
    char dirent_buf[buf_length];
    int indx = 0;
    char type;
    struct linux_dirent *dirent;
    /*checks if we've opened the folder successfully */
    if (fd < 0)
    {
        /*TODO: should print to stderr in case of debug mode?*/
        if (is_debug_mode)
            write_debug(SYS_WRITE, system_call(SYS_WRITE, STDERR, "ERROR: directory wasn't found", 30));
        else
            system_call(SYS_WRITE, STDERR, "ERROR: directory wasn't found", 30);
        system_call(SYS_EXIT, 0x55);
    }
    /*saves the number of bytes that were successfully read*/
    int bytes_read = system_call(SYS_GETDENTS, fd, dirent_buf, buf_length);
    if (is_debug_mode)
        write_debug(SYS_GETDENTS, bytes_read);
    if (bytes_read < 0)
    {
        /*TODO: should print to stderr in case of debug mode?*/
        write_debug(SYS_WRITE, system_call(SYS_WRITE, STDERR, "ERROR: could not load directory", 32));
        system_call(SYS_EXIT, 0x55);
    }
    int header = write(STDOUT, "Flame 2 strikes!\n");
    if (is_debug_mode)
    {
        write_debug(SYS_WRITE, header);
    }
    while (indx < bytes_read)
    {
        dirent = (struct linux_dirent *)(dirent_buf + indx);
        type = *(dirent_buf + indx + dirent->d_reclen - 1);
        if (!is_p)
        {
            print_file(dirent, is_debug_mode);
        }
        else if (is_p && strncmp(dirent->d_name, prefix, strlen(prefix)) == 0)
        {
            print_file(dirent, is_debug_mode);
            write(STDOUT, "file type: ");
            write(STDOUT, get_file_type(type));
            write(STDOUT, "\n");
        }
        indx = indx + dirent->d_reclen;
    }

    return 0;
}
