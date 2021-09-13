#include "util.h"
#include <stdbool.h>

extern int system_call();
#define SYS_EXIT 1
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_READ 3
#define SYS_LSEEK 19
#define SYS_CLOSE 6
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define O_RDRW 0x002
#define O_WRONLY 0x001
#define O_CREATE 0x040
#define SEEK_SET 0
#define FILE_PREMISSION 0777

int main(int argc, char **argv)
{
    char lower_case;
    int err_file;
    int i = 0;
    bool is_debug_mode = false;
    int input_file = STDIN, output_file = STDOUT;
    char *input_file_name, *output_file_name;

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "-D", 2) == 0)
            /*TODO: ehat to do with the debug mode?
                can we use the strcnp ? ?*/
            {
                err_file = system_call(SYS_OPEN, STDERR, O_WRONLY, FILE_PREMISSION);
                is_debug_mode = true;
            }
            else if (strncmp(argv[i], "-i", 2) == 0)
            {
                input_file_name = argv[i] + 2;
                input_file = system_call(SYS_OPEN, input_file_name, O_RDRW, 0777);
                if (input_file < 0)
                {
                    char *err_str = "Error: couldn't open the input file\n";
                    system_call(SYS_WRITE, STDOUT, err_str, strlen(err_str));
                    system_call(SYS_EXIT);
                }
            }
            else if (strncmp(argv[i], "-o", 2) == 0)
            {
                output_file_name = argv[i] + 2;
                output_file = system_call(SYS_OPEN, output_file_name, O_WRONLY | O_CREATE, 0777);
            }
        }
    }

    if (input_file == STDIN)
    {
        char *str;
        str = "plese enter your input:\n";
        system_call(SYS_WRITE, output_file, str, strlen(str));
    }
    char user_input[1];
    int read;
    while ((read = (system_call(SYS_READ, input_file, user_input, 1))) > 0)
    {
        lower_case = user_input[0];
        if (user_input[0] == '\n')
        {
            system_call(SYS_WRITE, output_file, &user_input[0], 1);
        }
        else
        {
            if (user_input[0] <= 'Z' && user_input[0] >= 'A')
            {
                lower_case = lower_case + 32;
            }
            system_call(SYS_WRITE, output_file, &lower_case, 1);
        }
        /*
        if (is_debug_mode)
        {
            system_call(SYS_WRITE, STDOUT, str, strlen(str));
            system_call(SYS_WRITE, STDERR, itoa(SYS_READ), 1);
            system_call(SYS_WRITE, STDERR, "\n", 1);
            system_call(SYS_WRITE, STDERR, itoa(read), 1);
        }
        */
    }
    if (is_debug_mode)
    {
        system_call(SYS_CLOSE, err_file);
    }
    if (input_file != STDIN)
    {
        system_call(SYS_CLOSE, input_file);
    }
    if (output_file != STDOUT)
    {
        system_call(SYS_CLOSE, output_file);
    }

    return 0;
}
