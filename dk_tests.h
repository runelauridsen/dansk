////////////////////////////////////////////////////////////////
// rune: Test file parsing

typedef struct dk_test dk_test;
struct dk_test {
    str file_path;
    str name;
    str input;
    str output;
    dk_test *next;
};

typedef struct dk_tests dk_tests;
struct dk_tests {
    dk_test *first;
    dk_test *last;
    u64 count;
};

static dk_tests dk_tests_from_file(str file_path, arena *arena);

////////////////////////////////////////////////////////////////
// rune: Runner

static void dk_run_test_file(str file_name, str filter);
static void dk_run_test_numbers(void);
static void dk_run_tests(void);