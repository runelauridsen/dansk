static char *fmt_get_esc_seq(int c) {
    static readonly char *esc_seq_table[] = {
        [0x07] = "\\a",
        [0x08] = "\\b",
        [0x1b] = "\\e",
        [0x0c] = "\\f",
        [0x0a] = "\\n",
        [0x0d] = "\\r",
        [0x09] = "\\t",
        [0x0b] = "\\v",
        [0x5c] = "\\\\",
        [0x27] = "\\'",
        [0x22] = "\\\"",
    };

    char *esc_seq = null;
    if (c >= 0 && c < countof(esc_seq_table)) {
        esc_seq = esc_seq_table[c];
    }

    return esc_seq;
}

static void fmt_append_raw_u8(buf *buf, u8 u) {
    if (buf->v && buf->len < buf->cap) {
        buf->v[buf->len] = u;
    }
    buf->len += 1;
}

static void fmt_append_raw(buf *buf, fmt_spec spec, str s) {
    unused(spec);
    if (buf->v) {
        i64 copy_len = max(min(s.len, buf->cap - buf->len), 0);
        memcpy(buf->v + buf->len, s.v, copy_len);
        buf->len += copy_len;
    } else {
        buf->len += s.len;
    }
}

// NOTE(rune): Workaround by Paul Groke because MSVC doesn't support printf format checks for user functions.
// https://developercommunity.visualstudio.com/t/support-format-specifier-checking-on-user-defined/843652,
#define fmt_check_va(...)                   ((void)(sizeof(printf(__VA_ARGS__))))
#define fmt_append_printf(buf, spec, ...)   (fmt_check_va(__VA_ARGS__), fmt_append_printf_(buf, spec, __VA_ARGS__))

// NOTE(rune): This procedure should be the only time we interface with CRT formatting.
// Unfortunately vsnprintf always null terminates the output, even though the rest of
// the code base does not rely on null-terminated strings. To utilize fmt_buf fully
// (not wasting the last byte on a null-terminator), we just use a fixed-sized scratch buffer
// which is large enough to format 64-bit floats and integers.
static void fmt_append_printf_(buf *buf, fmt_spec spec, char *format, ...) {
    YO_PROFILE_BEGIN(fmt_append_printf);

    char scratch[512];

    va_list va;
    va_start(va, format);
    int len = vsnprintf(scratch, sizeof(scratch), format, va);
    va_end(va);

    if (len > 0) {
        str s = str_make((u8 *)scratch, len);
        fmt_append_raw(buf, spec, s);
    }

    YO_PROFILE_END(fmt_append_printf);
}

static void fmt_append_character(buf *buf, fmt_spec spec, u32 c) {
    if (spec.lit) {
        char *esc_seq = fmt_get_esc_seq(c);
        if (esc_seq) {
            fmt_append_printf(buf, spec, "%s", esc_seq);
        } else if (u32_is_printable(c)) {
            fmt_append_printf(buf, spec, "%c", c);
        } else if (c <= 0xff) {
            fmt_append_printf(buf, spec, "'\\x%02x'", c);
        } else if (c <= 0xffff) {
            fmt_append_printf(buf, spec, "'\\u%04x'", c);
        } else {
            fmt_append_printf(buf, spec, "'\\U%08x'", c);
        }
    } else {
        fmt_append_printf(buf, spec, "%c", c);
    }
}

static void fmt_append_hex(buf *buf, fmt_spec spec, u64 v) {
    if (spec.lit) {
        fmt_append_printf(buf, spec, "0x%llx", v);
    } else {
        fmt_append_printf(buf, spec, "%llx", v);
    }
}

static void fmt_append_hexpad(buf *buf, fmt_spec spec, u64 v, i32 num_bits) {
    if (spec.lit) {
        fmt_append_printf(buf, spec, "0x%0*llx", num_bits / 4, v);
    } else {
        fmt_append_printf(buf, spec, "%0*llx", num_bits / 4, v);
    }
}

static void fmt_append_usigned(buf *buf, fmt_spec spec, u64 v, i32 num_bits) {
    if (spec.size) {
        if (v < 1024)               fmt_append_printf(buf, spec, "%llu B", v);
        else if (v < 1024 * 1024)   fmt_append_printf(buf, spec, "%llu KB", v / 1024);
        else                        fmt_append_printf(buf, spec, "%llu MB", v / (1024 * 1024));
    } else if (spec.character) {
        fmt_append_character(buf, spec, u32(v));
    } else if (spec.hex) {
        fmt_append_hex(buf, spec, v);
    } else if (spec.hexpad) {
        fmt_append_hexpad(buf, spec, v, num_bits);
    } else {
        fmt_append_printf(buf, spec, "%llu", v);
    }
}

static void fmt_append_signed(buf *buf, fmt_spec spec, i64 v, i32 num_bits) {
    if (spec.size) {
        if (v < 1024)               fmt_append_printf(buf, spec, "%lli B", v);
        else if (v < 1024 * 1024)   fmt_append_printf(buf, spec, "%lli KB", v / 1024);
        else                        fmt_append_printf(buf, spec, "%lli MB", v / (1024 * 1024));
    } else if (spec.character) {
        fmt_append_character(buf, spec, u32(v));
    } else if (spec.hex) {
        fmt_append_hex(buf, spec, u64(v));
    } else if (spec.hexpad) {
        fmt_append_hexpad(buf, spec, u64(v), num_bits);
    } else {
        fmt_append_printf(buf, spec, "%lli", v);
    }
}

static void fmt_append_u8(buf *buf, fmt_spec spec, u8 v) { fmt_append_usigned(buf, spec, u64(v), 8); }
static void fmt_append_u16(buf *buf, fmt_spec spec, u16 v) { fmt_append_usigned(buf, spec, u64(v), 16); }
static void fmt_append_u32(buf *buf, fmt_spec spec, u32 v) { fmt_append_usigned(buf, spec, u64(v), 32); }
static void fmt_append_u64(buf *buf, fmt_spec spec, u64 v) { fmt_append_usigned(buf, spec, u64(v), 64); }

static void fmt_append_i8(buf *buf, fmt_spec spec, i8 v) { fmt_append_signed(buf, spec, i64(v), 8); }
static void fmt_append_i16(buf *buf, fmt_spec spec, i16 v) { fmt_append_signed(buf, spec, i64(v), 16); }
static void fmt_append_i32(buf *buf, fmt_spec spec, i32 v) { fmt_append_signed(buf, spec, i64(v), 32); }
static void fmt_append_i64(buf *buf, fmt_spec spec, i64 v) { fmt_append_signed(buf, spec, i64(v), 64); }

static void fmt_append_f32(buf *buf, fmt_spec spec, f32 v) {
    if (spec.hex || spec.hexpad) {
        fmt_append_u32(buf, spec, u32_from_f32(v));
    } else if (spec.lit && v == F32_MIN) {
        fmt_append_raw(buf, spec, str("F32_MIN"));
    } else if (spec.lit && v == F32_MAX) {
        fmt_append_raw(buf, spec, str("F32_MAX"));
    } else if (spec.lit && v == F32_INF) {
        fmt_append_raw(buf, spec, str("F32_INF"));
    } else if (spec.lit && v == -F32_INF) {
        fmt_append_raw(buf, spec, str("-F32_INF"));
    } else if (spec.lit && f32_is_nan(v)) {
        fmt_append_raw(buf, spec, str("F32_NAV"));
    } else {
        fmt_append_printf(buf, spec, "%f", v);
    }
}

static void fmt_append_f64(buf *buf, fmt_spec spec, f64 v) {
    if (spec.hex || spec.hexpad) {
        fmt_append_u64(buf, spec, u64_from_f64(v));
    } else if (spec.lit && v == F64_MIN) {
        fmt_append_raw(buf, spec, str("F64_MIN"));
    } else if (spec.lit && v == F64_MAX) {
        fmt_append_raw(buf, spec, str("F64_MAX"));
    } else if (spec.lit && v == F64_INF) {
        fmt_append_raw(buf, spec, str("F64_INF"));
    } else if (spec.lit && v == -F64_INF) {
        fmt_append_raw(buf, spec, str("-F64_INF"));
    } else if (spec.lit && f64_is_nan(v)) {
        fmt_append_raw(buf, spec, str("F64_NAV"));
    } else {
        fmt_append_printf(buf, spec, "%f", v);
    }
}

static void fmt_append_bool(buf *buf, fmt_spec spec, bool v) {
    unused(spec);
    if (v) {
        fmt_append_raw(buf, spec, str("true"));
    } else {
        fmt_append_raw(buf, spec, str("false"));
    }
}

static void fmt_append_str(buf *buf, fmt_spec spec, str v) {
    if (spec.lit) {
        fmt_append_raw(buf, spec, str("\""));
        for_array (u8, c, v) {
            fmt_append_character(buf, (fmt_spec) { .lit = true }, *c);
        }
        fmt_append_raw(buf, spec, str("\""));
    } else {
        fmt_append_raw(buf, spec, v);
    }
}

static void fmt_append_cstr(buf *buf, fmt_spec spec, char *v) {
    str s = str_from_cstr(v);
    fmt_append_str(buf, spec, s);
}

static void fmt_append_vec2(buf *buf, fmt_spec spec, vec2 v) {
    fmt_append_raw(buf, spec, str("("));
    fmt_append_val(buf, spec, v.x);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_val(buf, spec, v.y);
    fmt_append_raw(buf, spec, str(")"));
}

static void fmt_append_vec3(buf *buf, fmt_spec spec, vec3 v) {
    fmt_append_raw(buf, spec, str("("));
    fmt_append_f32(buf, spec, v.x);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_f32(buf, spec, v.y);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_f32(buf, spec, v.z);
    fmt_append_raw(buf, spec, str(")"));
}

static void fmt_append_vec4(buf *buf, fmt_spec spec, vec4 v) {
    fmt_append_raw(buf, spec, str("("));
    fmt_append_f32(buf, spec, v.x);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_f32(buf, spec, v.y);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_f32(buf, spec, v.z);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_f32(buf, spec, v.w);
    fmt_append_raw(buf, spec, str(")"));
}

static void fmt_append_rect(buf *buf, fmt_spec spec, rect v) {
    fmt_append_vec2(buf, spec, v.p0);
    fmt_append_raw(buf, spec, str(" x "));
    fmt_append_vec2(buf, spec, v.p1);
}

static void fmt_append_any(buf *buf, fmt_spec spec, any v);

static void fmt_append_any_range(buf *buf, fmt_spec spec, any min, any max) {
    fmt_append_raw(buf, spec, str("["));
    fmt_append_any(buf, spec, min);
    fmt_append_raw(buf, spec, str(", "));
    fmt_append_any(buf, spec, max);
    fmt_append_raw(buf, spec, str("]"));
}

static void fmt_append_range_i8(buf *buf, fmt_spec spec, i8_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_i16(buf *buf, fmt_spec spec, i16_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_i32(buf *buf, fmt_spec spec, i32_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_i64(buf *buf, fmt_spec spec, i64_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_u8(buf *buf, fmt_spec spec, u8_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_u16(buf *buf, fmt_spec spec, u16_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_u32(buf *buf, fmt_spec spec, u32_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_u64(buf *buf, fmt_spec spec, u64_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_f32(buf *buf, fmt_spec spec, f32_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }
static void fmt_append_range_f64(buf *buf, fmt_spec spec, f64_range v) { fmt_append_any_range(buf, spec, anyof(v.min), anyof(v.max)); }

static void fmt_append_ptr(buf *buf, fmt_spec spec, void *ptr) {
    unused(spec);
    fmt_append_printf(buf, spec, "%p", ptr);
}

static void fmt_append_any(buf *buf, fmt_spec spec, any v) {
    switch (v.tag) {
        case ANY_TAG_I8: fmt_append_i8(buf, spec, v._i8);   break;
        case ANY_TAG_I16: fmt_append_i16(buf, spec, v._i16); break;
        case ANY_TAG_I32: fmt_append_i32(buf, spec, v._i32); break;
        case ANY_TAG_I64: fmt_append_i64(buf, spec, v._i64); break;
        case ANY_TAG_U8: fmt_append_u8(buf, spec, v._u8); break;
        case ANY_TAG_U16: fmt_append_u16(buf, spec, v._u16); break;
        case ANY_TAG_U32: fmt_append_u32(buf, spec, v._u32); break;
        case ANY_TAG_U64: fmt_append_u64(buf, spec, v._u64); break;
        case ANY_TAG_F32: fmt_append_f32(buf, spec, v._f32); break;
        case ANY_TAG_F64: fmt_append_f64(buf, spec, v._f64); break;
        case ANY_TAG_BOOL: fmt_append_bool(buf, spec, v._bool); break;
        case ANY_TAG_STR: fmt_append_str(buf, spec, v._str); break;
        case ANY_TAG_VEC2: fmt_append_vec2(buf, spec, v._vec2); break;
        case ANY_TAG_VEC3: fmt_append_vec3(buf, spec, v._vec3); break;
        case ANY_TAG_VEC4: fmt_append_vec4(buf, spec, v._vec4); break;
        case ANY_TAG_RECT: fmt_append_rect(buf, spec, v._rect); break;
        case ANY_TAG_RANGE_I8: fmt_append_any_range(buf, spec, anyof(v._range_i8.min), anyof(v._range_i8.max)); break;
        case ANY_TAG_RANGE_I16: fmt_append_any_range(buf, spec, anyof(v._range_i16.min), anyof(v._range_i16.max)); break;
        case ANY_TAG_RANGE_I32: fmt_append_any_range(buf, spec, anyof(v._range_i32.min), anyof(v._range_i32.max)); break;
        case ANY_TAG_RANGE_I64: fmt_append_any_range(buf, spec, anyof(v._range_i64.min), anyof(v._range_i64.max)); break;
        case ANY_TAG_RANGE_U8: fmt_append_any_range(buf, spec, anyof(v._range_u8.min), anyof(v._range_u8.max)); break;
        case ANY_TAG_RANGE_U16: fmt_append_any_range(buf, spec, anyof(v._range_u16.min), anyof(v._range_u16.max)); break;
        case ANY_TAG_RANGE_U32: fmt_append_any_range(buf, spec, anyof(v._range_u32.min), anyof(v._range_u32.max)); break;
        case ANY_TAG_RANGE_U64: fmt_append_any_range(buf, spec, anyof(v._range_u64.min), anyof(v._range_u64.max)); break;
        case ANY_TAG_RANGE_F32: fmt_append_any_range(buf, spec, anyof(v._range_f32.min), anyof(v._range_f32.max)); break;
        case ANY_TAG_RANGE_F64: fmt_append_any_range(buf, spec, anyof(v._range_f64.min), anyof(v._range_f64.max)); break;
        case ANY_TAG_PTR: fmt_append_ptr(buf, spec, v._ptr); break;
        default: fmt_append_raw(buf, spec, str("[unknown any_tag]")); break;
    }
}

// @Feature Format specifier for u32 -> hex rgb/rgba.
// @Feature Format specifier for string -> Properly escaped C string literal.
// @Feature Format specifier for char   -> Properly escaped C char literal.
// @Feature Color speicifers that are translated to ANSI escape codes.

static fmt_spec fmt_parse_spec(str s) {
    fmt_spec ret = { 0 };
    while (s.len) {
        str word = str_chop_by_delim(&s, str(","));
        if (0);
        else if (str_eq(word, str("hex")))       ret.hex       = true;
        else if (str_eq(word, str("hexpad")))    ret.hexpad    = true;
        else if (str_eq(word, str("lit")))       ret.lit       = true;
        else if (str_eq(word, str("literal")))   ret.lit       = true;
        else if (str_eq(word, str("size")))      ret.size      = true;
        else if (str_eq(word, str("rgb")))       ret.rgb       = true;
        else if (str_eq(word, str("rgba")))      ret.rgba      = true;
        else if (str_eq(word, str("c")))         ret.character = true;
        else if (str_eq(word, str("char")))      ret.character = true;
        else if (str_eq(word, str("character"))) ret.character = true;
        else if (str_eq(word, str("size")))      ret.size      = true;
    }

    return ret;
}

static any pop_first_arg(args *args) {
    any ret = { 0 };
    if (args->count > 0) {
        ret = args->v[0];
        args->count--;
        args->v++;
    }
    return ret;
}

static void fmt_append_args(buf *buf, args args) {
    YO_PROFILE_BEGIN(fmt_args);

    if (args.count == 1) {
        fmt_append_any(buf, FMT_SPEC_DEFAULT, args.v[0]);
    } else {
        str fmt = str("");
        any fmt_arg = pop_first_arg(&args);
        switch (fmt_arg.tag) {
            case ANY_TAG_STR: {
                fmt = fmt_arg._str;
            } break;

            default: {
                assert(!"First argument is not a format string.");
            } break;
        }

        YO_PROFILE_BEGIN(fmt_args_lex_loop);
        enum { NORMAL, PERCENT, SPECIFIER } state = NORMAL;

        i64 arg_idx = 0;
        str spec_str = str("");
        for (u8 *at = fmt.v; at < fmt.v + fmt.len; at++) {
            if (buf->v && buf->len >= buf->cap) {
                break;
            }

            u8 c = *at;
            bool dont_copy_to_out = false;
            bool ready_to_format_next_arg = false;

            switch (state) {
                case PERCENT: {
                    if (c == '(') {
                        state = SPECIFIER;
                        spec_str.v = at + 1;
                        spec_str.count = 0;
                    } else {
                        ready_to_format_next_arg = true;
                        if (c == '%') {
                            state = PERCENT;
                        } else {
                            state = NORMAL;
                        }
                    }
                } break;

                case SPECIFIER: {
                    if (c == ')') {
                        ready_to_format_next_arg = true;
                        state = NORMAL;
                        dont_copy_to_out = true;
                    } else {
                        spec_str.count++;
                    }
                } break;

                case NORMAL: {
                    if (c == '%') {
                        state = PERCENT;
                    }
                } break;
            }

            if (ready_to_format_next_arg) {
                fmt_spec spec = fmt_parse_spec(spec_str);
                spec_str = str("");

                any arg;
                if (arg_idx < args.count) {
                    arg = args.v[arg_idx++];
                } else {
                    arg = anyof("[not enough arguments]");
                }

                fmt_append_any(buf, spec, arg);
            }

            if (c == '\0') {
                break;
            }

            if (state == NORMAL && !dont_copy_to_out) {
                fmt_append_raw_u8(buf, c);
            }
        }

        if (state == PERCENT) {
            any arg;
            if (arg_idx < args.count) {
                arg = args.v[arg_idx++];
            } else {
                arg = anyof("[not enough arguments]");
            }

            fmt_spec spec = fmt_parse_spec(spec_str);
            fmt_append_any(buf, spec, arg);
            spec_str = str("");
        }

        YO_PROFILE_END(fmt_args_lex_loop);
    }

    YO_PROFILE_END(fmt_args);
}

static i64 fmt_args(u8 *out, i64 cap, args args) {
    buf buf = make_buf(out, cap, 0);
    fmt_append_args(&buf, args);
    return buf.len;
}

static void print_args(args args) {
    u8 scratch[4096];
    i64 len = fmt_args(scratch, sizeof(scratch) - 1, args);
    scratch[len] = '\0';
#if 0
    OutputDebugStringA((char *)scratch);
#else
    printf("%s", (char *)scratch);
#endif
}

static void println_args(args args) {
    print_args(args);
    print_args(argsof("\n"));
}

static str arena_print_args(arena *arena, args args) {
    i64 len = fmt_args(0, 0, args);
    str ret = { 0 };
    ret.v = arena_push_size_nozero(arena, len + 1, 0);
    if (ret.v) {
        fmt_args(ret.v, len, args);
        ret.len = len;
        ret.v[len] = '\0';
    }
    return ret;
}

static void fmt_pad_until(buf *buf, i64 pad_len) {
    if (pad_len > buf->len) {
        i64 add_chars = pad_len - buf->len;
        for_n (i64, i, add_chars) {
            fmt_append_raw_u8(buf, ' ');
        }
    }
}

// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste
// TODO(rune): Lots of @Copypaste

static bool fmt_parse_u32(str s, u32 *out) {
    YO_PROFILE_BEGIN(fmt_parse_u32);

    u32 parsed = 0;
    bool ret = false;
    s = str_trim(s);
    if (s.len > 0) {
        ret = true;
        for_array (u8, c, s) {
            if (u32_is_digit(*c)) {
                // @Todo Check for overflow.
                parsed *= 10;
                parsed += *c - '0';
            } else {
                ret = false;
                break;
            }
        }
    }

    if (ret) {
        *out = parsed;
    }

    YO_PROFILE_END(fmt_parse_u32);
    return ret;
}

static bool fmt_parse_u64(str s, u64 *out) {
    YO_PROFILE_BEGIN(fmt_parse_u64);

    u64 parsed = 0;
    bool ret = false;
    s = str_trim(s);
    if (s.len > 0) {
        ret = true;
        for_array (u8, c, s) {
            if (u32_is_digit(*c)) {
                // @Todo Check for overflow.
                parsed *= 10;
                parsed += *c - '0';
            } else {
                ret = false;
                break;
            }
        }
    }

    if (ret) {
        *out = parsed;
    }

    YO_PROFILE_END(fmt_parse_u64);
    return ret;
}

static bool fmt_parse_u32_hex(str s, u32 *out) {
    YO_PROFILE_BEGIN(ui_fmt_parse_u32_hex);

    u32 parsed = 0;
    bool ret = false;
    s = str_trim(s);
    if (s.len > 0) {
        ret = true;
        for_array (u8, c, s) {
            *c = u8_to_lower(*c);
            if (u8_is_digit(*c)) {
                // @Todo Check for overflow.
                parsed *= 16;
                parsed += *c - '0';
            } else if (*c >= 'a' && *c <= 'f') {
                parsed *= 16;
                parsed += *c - 'a' + 10;
            } else {
                ret = false;
                break;
            }
        }
    }

    if (ret) {
        *out = parsed;
    }

    YO_PROFILE_END(ui_fmt_parse_u32_hex);
    return ret;
}

static bool fmt_parse_u64_hex(str s, u64 *out) {
    YO_PROFILE_BEGIN(ui_fmt_parse_u64_hex);

    u64 parsed = 0;
    bool ret = false;
    s = str_trim(s);
    if (s.len > 0) {
        ret = true;
        for_array (u8, c, s) {
            *c = u8_to_lower(*c);
            if (u8_is_digit(*c)) {
                // @Todo Check for overflow.
                parsed *= 16;
                parsed += *c - '0';
            } else if (*c >= 'a' && *c <= 'f') {
                parsed *= 16;
                parsed += *c - 'a' + 10;
            } else {
                ret = false;
                break;
            }
        }
    }

    if (ret) {
        *out = parsed;
    }

    YO_PROFILE_END(ui_fmt_parse_u64_hex);
    return ret;
}

static bool fmt_parse_f64(str s, f64 *out) {
    bool ret = false;
    char temp[256];
    if (s.len + 1 < sizeof(temp)) {
        memcpy(temp, s.v, s.len);
        temp[s.len] = '\0';

        char *end_ptr = temp;
        *out = strtod(temp, &end_ptr);

        if (end_ptr == temp + s.len) {
            ret = true;
        }
    }

    return ret;
}
