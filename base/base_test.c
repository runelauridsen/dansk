////////////////////////////////////////////////////////////////
// rune: Test context

static void test_ctx_begin(test_ctx *ctx) {
    assert(test_g_ctx == null && "Nested test context not allowed.");
    test_g_ctx = ctx;
    ctx->failed_count = 0;
    ctx->passed_count = 0;
    ctx->duration_total = 0.0;
    ctx->arena = arena_create(kilobytes(64), ARENA_KIND_LINEAR);
}

static void test_ctx_end(test_ctx *ctx) {
    test_g_ctx = null;

    print("\n");
    if (ctx->name.len > 0) print("%: ", ctx->name);

    // rune: Passed
    if (ctx->failed_count == 0) print(ANSI_FG_BRIGHT_GREEN);
    print("%/% ", ctx->passed_count, ctx->passed_count + ctx->failed_count);
    print(ANSI_FG_DEFAULT);
    print("passed ");

    // rune: Failed
    if (ctx->failed_count > 0) print(ANSI_FG_RED);
    print("(% failed) ", ctx->failed_count);

    // rune: Duration
    print(ANSI_FG_GRAY);
    print("% ms", ctx->duration_total);

    // rune: Seperator
    print("\n");
    print(ANSI_FG_GRAY);
    print("\n");
    print("================================================================\n");
    print("\n");
    print(ANSI_RESET);

    arena_destroy(ctx->arena);
}

static arena *test_arena(void) {
    test_ctx *ctx = test_g_ctx;
    return ctx->arena;
}

////////////////////////////////////////////////////////////////
// rune: Test scope

static void test_scope_begin(str loc, args name_args) {
    test_ctx *ctx = test_g_ctx;

    assert(ctx != null && "No test context set.");
    assert(ctx->curr_test.in_progress == false && "Nested test scopes not allowed.");

    arena_scope_begin(ctx->arena);

    // rune: Run user defined setup.
    if (ctx->before) ctx->before(ctx->param);

    // rune: Reset per-test data.
    ctx->curr_test.failed_assertions.count = 0;
    ctx->curr_test.failed_assertions.first = null;
    ctx->curr_test.failed_assertions.last  = null;
    ctx->curr_test.in_progress             = true;
    ctx->curr_test.loc                     = loc;
    ctx->curr_test.name                    = arena_print_args(ctx->arena, name_args);
    ctx->curr_test.timestamp_begin         = os_get_performance_timestamp();
}

static void test_scope_end(void) {
    test_ctx *ctx = test_g_ctx;

    // rune: Finish per-test data.
    ctx->curr_test.in_progress   = false;
    ctx->curr_test.timestamp_end = os_get_performance_timestamp();
    f64 elapsed_millis           = os_get_millis_between(ctx->curr_test.timestamp_begin, ctx->curr_test.timestamp_end);
    ctx->duration_total         += elapsed_millis;

    // rune: Run user defined teardown.
    if (ctx->after) ctx->after(ctx->param);

    // rune: Print failed assertions.
    for_list (test_assertion, a, ctx->curr_test.failed_assertions) {
        print("\n");
        print(ANSI_FG_GRAY);
        print("% ", a->loc);
        print(ANSI_FG_BRIGHT_RED);
        print("Assertion failed", ctx->curr_test.name);
        print("\n");

        if (a->msg.len != 0) {
            print(ANSI_FG_GRAY);
            print("% ", a->loc);
            print(ANSI_FG_BRIGHT_RED);
            print("    Message: %", a->msg);
            print("\n");
        }

        if (a->expr.len != 0) {
            print(ANSI_FG_GRAY);
            print("% ", a->loc);
            print(ANSI_FG_BRIGHT_RED);
            print("    Expr: %", a->expr);
            print("\n");
        }

        if (a->expect.tag != 0) {
            print(ANSI_FG_GRAY);
            print("% ", a->loc);
            print(ANSI_FG_BRIGHT_RED);
            print("    Expect: %(literal)", a->expect);
            print("\n");
        }

        if (a->actual.tag != 0) {
            print(ANSI_FG_GRAY);
            print("% ", a->loc);
            print(ANSI_FG_BRIGHT_RED);
            print("    Actual: %(literal)", a->actual);
            print("\n");
        }
    }
    print(ANSI_FG_DEFAULT);

    // rune: Print result.
    print(ANSI_FG_GRAY    "% ", ctx->curr_test.loc);
    if (ctx->curr_test.failed_assertions.count) {
        print(ANSI_FG_RED "❌ ");
        ctx->failed_count++;
    } else {
        print(ANSI_FG_BRIGHT_GREEN "✔  ");
        ctx->passed_count++;
    }

    print(ANSI_FG_DEFAULT "% ", ctx->curr_test.name);
    print(ANSI_FG_GRAY "% ms ", elapsed_millis);
    print(ANSI_FG_DEFAULT "\n");

    arena_scope_end(ctx->arena);
}

////////////////////////////////////////////////////////////////
// rune: Assertions

static test_assertion *test_push_failed_assertion(void) {
    test_ctx *ctx = test_g_ctx;
    test_assertion *a = arena_push_struct(ctx->arena, test_assertion);
    slist_push(&ctx->curr_test.failed_assertions, a);
    ctx->curr_test.failed_assertions.count++;
    return a;
}

static bool test_assert_(str loc, bool condition, str expr) {
    if (condition == false) {
        test_assertion *a = test_push_failed_assertion();
        a->loc  = loc;
        a->expr = expr;
    }

    return condition;
}

static bool test_assert_eq_any(str loc, any actual, any expect) {
    bool eq = any_eq(expect, actual);
    if (eq == false) {
        test_assertion *a = test_push_failed_assertion();
        a->loc    = loc;
        a->actual = actual;
        a->expect = expect;

        if (a->actual.tag == ANY_TAG_STR) a->actual._str = arena_copy_str(test_arena(), a->actual._str);
        if (a->expect.tag == ANY_TAG_STR) a->expect._str = arena_copy_str(test_arena(), a->expect._str);
    }

    return eq;
}
