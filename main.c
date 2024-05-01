////////////////////////////////////////////////////////////////
// rune: Source files

#include "base/base.h"
#include "dk.h"
#include "dk.c"
#include "dk_tests.h"
#include "dk_tests.c"

////////////////////////////////////////////////////////////////
// rune: Command line program

typedef struct dk_cmdline dk_cmdline;
struct dk_cmdline {
    int argc;
    char **argv;
};

static char *dk_cmdline_peek(dk_cmdline *cmd) {
    char *ret = null;
    if (cmd->argc > 0) {
        ret = cmd->argv[0];
    }
    return ret;
}

static char *dk_cmdline_pop(dk_cmdline *cmd) {
    char *ret = null;
    if (cmd->argc > 0) {
        ret = cmd->argv[0];
        cmd->argc--;
        cmd->argv++;
    }
    return ret;
}

static bool dk_cmdline_subcommand(dk_cmdline *cmd, char *look_for) {
    bool ret = false;
    if (strcmp(dk_cmdline_peek(cmd), look_for) == 0) {
        dk_cmdline_pop(cmd);
        ret = true;
    }
    return ret;
}

static bool dk_cmdline_read_file(dk_cmdline *cmd, str *file_name, str *file_data, arena *arena) {
    bool ret = false;
    char *arg = dk_cmdline_pop(cmd);
    if (arg) {
        *file_name = str_from_cstr(arg);
        *file_data = os_read_entire_file(*file_name, arena, null);

        if (file_data->len > 0) {
            ret = true;
        } else {
            println("Could not read file: %", *file_name);
        }
    } else {
        println("No file name given.");
    }

    return ret;
}

int main(int argc, char **argv) {
#if _WIN32
    SetConsoleOutputCP(65001); // NOTE(rune): UTF8 codepage
#endif

    static char *usage =
        "Usage:                                                                    \n"
        "    dansk help                    Print this message                      \n"
        "    dansk run <program.dk>        Build program.dk and run in interpreter \n"
        "    dansk test                    Run tests                               \n";

    arena *arena = arena_create_default();
    temp_arena = arena_create_default();

    if (argc == 1) {
        println("%", usage);
    } else {
        dk_cmdline cmd = { argc, argv };
        char *executeable = dk_cmdline_pop(&cmd);

        // rune: help subcommand
        if (dk_cmdline_subcommand(&cmd, "help")) {
            println(usage);
        }

        // rune: run subcommand
        else if (dk_cmdline_subcommand(&cmd, "run")) {
            str file_name = { 0 };
            str file_data = { 0 };
            if (dk_cmdline_read_file(&cmd, &file_name, &file_data, arena)) {
                dk_err_sink err = { 0 };
                dk_program program = dk_program_from_str(file_data, &err, arena);
                if (err.err_list.count == 0) {
                    str output = dk_run_program(program, arena);
                    print(output);
                } else {
                    dk_print_err(err.err_list.first, arena);
                }
            }
        }

        // rune: test subcommand
        else if (dk_cmdline_subcommand(&cmd, "test")) {
            dk_run_tests();
        }

        else {
            println("Unknown subcommand %", dk_cmdline_pop(&cmd));
        }
    }

    arena_destroy(arena);

    return 0;
}
