// TODO(rune):
// - Remove need for () around placeholders, just use %char,literal syntax instead.
// - Some whay to specify padding, for easier table printing.

////////////////////////////////////////////////////////////////
// rune: Format append to buffer

typedef struct fmt_spec fmt_spec;
struct fmt_spec {
    bool hex : 1;
    bool hexpad : 1;
    bool lit : 1;
    bool size : 1;
    bool rgb : 1;
    bool rgba : 1;
    bool character : 1;
};

#define FMT_SPEC_DEFAULT ((fmt_spec){0})

#define fmt_spec_(prop)   .prop = true
#define fmt_spec(...)     ((fmt_spec) { VA_WRAP(fmt_spec_, __VA_ARGS__) })

static void fmt_append_i8(buf *buf, fmt_spec spec, i8 v);
static void fmt_append_i16(buf *buf, fmt_spec spec, i16 v);
static void fmt_append_i32(buf *buf, fmt_spec spec, i32 v);
static void fmt_append_i64(buf *buf, fmt_spec spec, i64 v);
static void fmt_append_u8(buf *buf, fmt_spec spec, u8 v);
static void fmt_append_u16(buf *buf, fmt_spec spec, u16 v);
static void fmt_append_u32(buf *buf, fmt_spec spec, u32 v);
static void fmt_append_u64(buf *buf, fmt_spec spec, u64 v);
static void fmt_append_f32(buf *buf, fmt_spec spec, f32 v);
static void fmt_append_f64(buf *buf, fmt_spec spec, f64 v);
static void fmt_append_bool(buf *buf, fmt_spec spec, bool v);
static void fmt_append_str(buf *buf, fmt_spec spec, str v);
static void fmt_append_vec2(buf *buf, fmt_spec spec, vec2 v);
static void fmt_append_vec3(buf *buf, fmt_spec spec, vec3 v);
static void fmt_append_vec4(buf *buf, fmt_spec spec, vec4 v);
static void fmt_append_rect(buf *buf, fmt_spec spec, rect v);
static void fmt_append_range_i8(buf *buf, fmt_spec spec, i8_range v);
static void fmt_append_range_i16(buf *buf, fmt_spec spec, i16_range v);
static void fmt_append_range_i32(buf *buf, fmt_spec spec, i32_range v);
static void fmt_append_range_i64(buf *buf, fmt_spec spec, i64_range v);
static void fmt_append_range_u8(buf *buf, fmt_spec spec, u8_range v);
static void fmt_append_range_u16(buf *buf, fmt_spec spec, u16_range v);
static void fmt_append_range_u32(buf *buf, fmt_spec spec, u32_range v);
static void fmt_append_range_u64(buf *buf, fmt_spec spec, u64_range v);
static void fmt_append_range_f32(buf *buf, fmt_spec spec, f32_range v);
static void fmt_append_range_f64(buf *buf, fmt_spec spec, f64_range v);
static void fmt_append_cstr(buf *buf, fmt_spec spec, char *v);
static void fmt_append_ptr(buf *buf, fmt_spec spec, void *v);

#define fmt_append_val(buf, spec, val) (_Generic((val),    \
        i8                  : fmt_append_i8,               \
        i16                 : fmt_append_i16,              \
        i64                 : fmt_append_i64,              \
        i32                 : fmt_append_i32,              \
        u8                  : fmt_append_u8,               \
        u16                 : fmt_append_u16,              \
        u32                 : fmt_append_u32,              \
        u64                 : fmt_append_u64,              \
        f32                 : fmt_append_f32,              \
        f64                 : fmt_append_f64,              \
        bool                : fmt_append_bool,             \
        str                 : fmt_append_str,              \
        union vec2          : fmt_append_vec2,             \
        union vec3          : fmt_append_vec3,             \
        union vec4          : fmt_append_vec4,             \
        union rect          : fmt_append_rect,             \
        struct i8_range     : fmt_append_range_i8,         \
        struct i16_range    : fmt_append_range_i16,        \
        struct i32_range    : fmt_append_range_i32,        \
        struct i64_range    : fmt_append_range_i64,        \
        struct u8_range     : fmt_append_range_u8,         \
        struct u16_range    : fmt_append_range_u16,        \
        struct u32_range    : fmt_append_range_u32,        \
        struct u64_range    : fmt_append_range_u64,        \
        struct f32_range    : fmt_append_range_f32,        \
        struct f64_range    : fmt_append_range_f64,        \
        char *              : fmt_append_cstr,             \
        void *              : fmt_append_ptr               \
))((buf), (spec), (val))

// NOTE(rune): Printf-like formatting e.g. fmt_append(buf, "This is a string: %. This is a number: %.", "abc", 123)
#define fmt_append(buf, ...) fmt_append_args((buf), argsof(__VA_ARGS__))
static void fmt_append_args(buf *buf, args args);

////////////////////////////////////////////////////////////////
// rune: Formatting

static i64 fmt_args(u8 *out, i64 cap, args args); // NOTE(rune): If out is 0, returns number if bytes that would have been written.
static void print_args(args args);
static void println_args(args args);
static str arena_print_args(arena *arena, args args);

#define fmt(out, cap, ...)            fmt_args(out, cap, argsof(__VA_ARGS__))
#define print(...)                    print_args(argsof(__VA_ARGS__))
#define println(...)                  println_args(argsof(__VA_ARGS__))
#define arena_print(arena, ...)       arena_print_args(arena, argsof(__VA_ARGS__))

#define dumpvar(expr) println("% = %(lit)", #expr, (expr))
