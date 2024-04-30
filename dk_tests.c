////////////////////////////////////////////////////////////////
// rune: Test file parsing

static str dk_lf_normalize(str s, arena *arena) {
    str normalized = arena_push_str(arena, s.len, 0);
    i64 i = 0;
    i64 j = 0;
    while (i < s.len) {
        u8 c0 = i + 0 < s.len ? s.v[i + 0] : '\0';
        u8 c1 = i + 1 < s.len ? s.v[i + 1] : '\0';

        if (c0 == '\r' && c1 == '\n') {
            i++;
        } else {
            normalized.v[j++] = s.v[i++];
        }
    }
    normalized.len = j;
    return normalized;
}

static dk_tests dk_tests_from_file(str file_path, arena *arena) {
    dk_tests tests = { 0 };

    // rune: Read file
    str file_data = os_read_entire_file(file_path, arena, null);
    file_data = dk_lf_normalize(file_data, arena);

    // rune: Split into sections
    str_list sections = str_split_by_delim(file_data, str("════════════════════════════════════════════════════════════════"), arena);
    for_list (str_node, node, sections) {
        // rune: Parse section
        str section = str_trim(node->v);
        str_list section_parts = str_split_by_delim(section, str("────────────────────────────────────────────────────────────────"), arena);
        if (section_parts.count >= 3) {
            str part1 = str_trim(section_parts.first->v);
            str part2 = str_trim(section_parts.first->next->v);
            str part3 = str_trim(section_parts.first->next->next->v);

            // rune: Add to list of tests
            dk_test *t   = arena_push_struct(arena, dk_test);
            t->file_path = file_path;
            t->name      = part1;
            t->input     = part2;
            t->output    = part3;

            slist_push(&tests, t);
            tests.count += 1;
        } else {
            // rune: Could not parse
            print(ANSI_FG_RED);
            println("Invalid test file format.");
            println("File path: %", file_path);
            println("Byte pos:  %", section.v - file_data.v);
            print(ANSI_FG_DEFAULT);
        }
    }

    return tests;
}

////////////////////////////////////////////////////////////////
// rune: Runner

static void dk_run_test_file(str file_path, str filter) {
    // rune: Setup test context
    test_ctx ctx = { 0 };
    ctx.name = file_path;
    test_ctx(&ctx) {
        // rune: Parse test file.
        dk_tests tests = dk_tests_from_file(file_path, test_arena());

        // rune: Loop over tests in file.
        for_list (dk_test, test, tests) {
            if (str_idx_of_str(test->name, filter) != -1) {
                test_scope(test->name) {
                    // rune: Run test.
                    str actual_output = { 0 };
                    dk_err_sink err_sink = { 0 };
                    dk_program program = dk_program_from_str(test->input, &err_sink, test_arena());

                    if (err_sink.err_list.count > 0) {
                        actual_output = err_sink.err_list.first->msg;
                    } else {
                        actual_output = dk_run_program(program, test_arena());
                    }

                    // rune: Check result. We don't care about whitespace.
                    actual_output     = str_trim(actual_output);
                    str expect_output = str_trim(test->output);
                    test_assert_eq(loc(), actual_output, expect_output);
                }
            }
        }
    }
}

static void dk_run_test_numbers(void) {
    test_ctx ctx = { 0 };
    ctx.name = str("Danish number parsing");
    test_ctx(&ctx) {
        typedef struct test_case test_case;
        struct test_case {
            str input;
            i64 expect;
        };

        static readonly test_case test_cases[] = {
            { STR("nitten"),                                                                                       19 },
            { STR("ethundredefireogtredive"),                                                                     134 },
            { STR("tretusindetohundrede"),                                                                       3200 },
            { STR("tretusinde-tohundrede"),                                                                      3200 },
            { STR("treogtredivehundredeogatten"),                                                                3318 },
            { STR("ettusind"),                                                                                   1000 },
            { STR("tusind"),                                                                                     1000 },

            { STR("tohundredeseksogtyvemillioner-tretusinde-syvoghalvtreds"),                                226003057 },
            { STR("to-tusind-seks-og-tyve-millioner-fire-og-tredive-tusind-seks-hundrede-fem-og-fyrre"),    2026034645 },
            { STR("to-hundrede-seks-og-tyve-tusind"),                                                           226000 },
            { STR("to-tusind-seks-og-tyve-millioner"),                                                      2026000000 },
            { STR("et-tusind-to-hundrede-seks-og-tyve"),                                                          1226 },
            { STR("to-hundrede-millioner"),                                                                  200000000 },

            { STR("toogfyrrea"),                                                                                    -1 },
            { STR("og"),                                                                                            -1 },
            { STR("-"),                                                                                             -1 },
        };

        for_sarray (test_case, it, test_cases) {
            test_scope("% == %" ANSI_RESET, it->input, it->expect) {
                i64 actual = dk_number_from_str(it->input);
                test_assert_eq(loc(), actual, it->expect);
            }
        }
    }
}

static void dk_run_tests(void) {
    dk_run_test_numbers();
    dk_run_test_file(str("dk_tests.dk"), str(""));
}
