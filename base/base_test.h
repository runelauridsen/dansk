////////////////////////////////////////////////////////////////
// rune: Structures

typedef struct test_assertion test_assertion;
struct test_assertion {
    str loc;
    str msg;
    str expr;
    any actual;
    any expect;
    test_assertion *next;
};

typedef struct test_assertion_list test_assertion_list;
struct test_assertion_list {
    test_assertion *first;
    test_assertion *last;
    i64 count;
};

typedef struct test_ctx test_ctx;
struct test_ctx {
    // rune: Cross-test data.
    str name;
    arena *arena;
    void *param;
    void (*before)(void *param);
    void (*after)(void *param);

    // rune: Results.
    i64 passed_count;
    i64 failed_count;
    f64 duration_total;

    // rune: Per-test data.
    struct {
        str name;
        str loc;
        u64 timestamp_begin;
        u64 timestamp_end;
        bool in_progress;
        test_assertion_list failed_assertions;
    } curr_test;
};

////////////////////////////////////////////////////////////////
// ruen: Test context

static test_ctx *test_g_ctx;

#define       test_ctx(ctx) defer(test_ctx_begin(ctx), test_ctx_end(ctx))
static void   test_ctx_begin(test_ctx *ctx);
static void   test_ctx_end(test_ctx *ctx);

////////////////////////////////////////////////////////////////
// rune: Test scope

#define     test_scope(...) defer(test_scope_begin(loc(), argsof(__VA_ARGS__)), test_scope_end())
static void test_scope_begin(str loc, args name_args);
static void test_scope_end(void);

// NOTE(rune): Allocations within a test_scope are only valid until the end of the test_scope.
static arena *test_arena(void);

////////////////////////////////////////////////////////////////
// rune: Assertions

#define     test_assert(loc, expr) (test_assert_((loc), (expr), str("test_assert(" #expr ");")))
static bool test_assert_(str loc, bool condition, str expr);

#define     test_assert_eq(loc, actual, expect) (test_assert_eq_any(loc, anyof(actual), anyof(expect)))
static bool test_assert_eq_any(str loc, any actual, any axpect);
