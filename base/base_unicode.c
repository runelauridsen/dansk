////////////////////////////////////////////////////////////////
// rune: Decode/encode


// NOTE(rune): Table generated with:
#if 0
#include <stdio.h>
int main() {
    printf("static const u8 decode_table[256] =\n{");

    for (int i = 0; i < 256; i++) {
        if (i % 32 == 0) printf("\n    ");

        if ((i >> 3) == 0b11110)      printf("4, "); // 11110xxx
        else if ((i >> 4) == 0b1110)  printf("3, "); // 1110xxxx
        else if ((i >> 5) == 0b110)   printf("2, "); // 111xxxxx
        else if ((i >> 7) == 0)       printf("1, "); // 0xxxxxxx
        else                         printf("0, "); // continuation
    }

    printf("\n};\n");
}
#endif

static readonly u8 utf8_table[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0-1f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 20-3f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 40-5f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 60-7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80-9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // a0-bf
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // c0-df
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, // e0-ff
};

static i32 utf8_class_from_u8(u8 cu) {
    static_assert(countof(utf8_table) == 256, "Size of utf8_table should be 256");
    i32 ret = (i32)utf8_table[cu];
    return ret;
}

static i32 utf8_class_from_u32(u32 cp) {
    if (0)                   return 0;
    else if (cp <= 0x7F)     return 1;
    else if (cp <= 0x7FF)    return 2;
    else if (cp <= 0xFFFF)   return 3;
    else if (cp <= 0x10FFFF) return 4;
    else                     return 0;
}

static i32 utf16_class_from_u32(u32 codepoint) {
    unused(codepoint);
    return 1; // @Implement
}

static bool utf8_is_continuation(u8 codeunit) {
    bool ret = utf8_class_from_u8(codeunit) == 0;
    return ret;
}

static unicode_codepoint decode_single_utf8_codepoint(str s) {
    // https://en.wikipedia.org/wiki/UTF-8#Encoding

    unicode_codepoint ret = { 0 };
    switch (utf8_class_from_u8(s.v[0])) {
        case 1: {
            if (s.len >= 1) {
                ret.len = 1;
                ret.codepoint = s.v[0];
            }
        } break;

        case 2: {
            if (s.len >= 2) {
                ret.len = 2;
                ret.codepoint = (((s.v[0] & 0b00011111) << (1*6)) |
                                 ((s.v[1] & 0b00111111) << (0*0)));
            }
        } break;

        case 3: {
            if (s.len >= 3) {
                ret.len = 3;
                ret.codepoint = (((s.v[0] & 0b00001111) << (2*6)) |
                                 ((s.v[1] & 0b00111111) << (1*6)) |
                                 ((s.v[2] & 0b00111111) << (0*6)));
            }
        } break;

        case 4: {
            if (s.len >= 4) {
                ret.len = 4;
                ret.codepoint = (((s.v[0] & 0b00000111) << (3*6)) |
                                 ((s.v[1] & 0b00111111) << (2*6)) |
                                 ((s.v[2] & 0b00111111) << (1*6)) |
                                 ((s.v[3] & 0b00111111) << (0*6)));
            }
        }
    }

    return ret;
}

static i32 encode_single_utf8_codepoint(u32 c, u8 *out) {
    i32 ret = 0;
    switch (utf8_class_from_u32(c)) {
        case 1: {
            out[0] = (u8)c;
            ret = 1;
        } break;

        case 2: {
            out[0] = 0b11000000 | ((c >> 1*6) & 0b00011111);
            out[1] = 0b10000000 | ((c >> 0*6) & 0b00111111);
            ret = 2;
        } break;

        case 3: {
            out[0] = 0b11100000 | ((c >> 2*6) & 0b00001111);
            out[1] = 0b10000000 | ((c >> 1*6) & 0b00111111);
            out[2] = 0b10000000 | ((c >> 0*6) & 0b00111111);
            ret = 3;
        } break;

        case 4: {
            out[0] = 0b11110000 | ((c >> 3*6) & 0b00000111);
            out[1] = 0b10000000 | ((c >> 2*6) & 0b00111111);
            out[2] = 0b10000000 | ((c >> 1*6) & 0b00111111);
            out[3] = 0b10000000 | ((c >> 0*6) & 0b00111111);
            ret = 4;
        } break;

        default: {
            out[0] = ' ';
            ret = 1;
        } break;
    }
    return ret;
}

static i32 encode_single_utf16_codepoint(u32 codepoint, u16 *out) {
    // @Implement
    out[0] = u16(codepoint);
    return 1;
}

static bool advance_single_utf8_codepoint(str *s, unicode_codepoint *decoded) {
    bool ret = false;
    if (s->len) {
        *decoded = decode_single_utf8_codepoint(*s);

        s->v     += decoded->len;
        s->count -= decoded->len;

        ret = decoded->len > 0;
    }
    return ret;
}

static unicode_codepoint decode_single_utf16_codepoint(wstr w) {
    // TODO(rune): @Todo Actual utf16 decoder.

    unicode_codepoint ret = { 0 };

    if (w.count) {
        assert(w.v[0] < 0xd7ff);
        ret.codepoint = w.v[0];
        ret.len = 1;
    }

    return ret;
}

static bool advance_single_utf16_codepoint(wstr *w, unicode_codepoint *decoded) {
    *decoded = decode_single_utf16_codepoint(*w);

    w->v     += decoded->len;
    w->count -= decoded->len;

    bool ret = decoded->len > 0;
    return ret;
}

////////////////////////////////////////////////////////////////
// rune: Conversions

static str convert_utf16_to_utf8(wstr w, arena *out) {
    i64 max_len_as_utf8 = (w.count + 1) * 3;
    buf buf = arena_push_buf(out, max_len_as_utf8);
    unicode_codepoint decoded = { 0 };
    while (advance_single_utf16_codepoint(&w, &decoded)) {
        buf_append_utf8_codepoint(&buf, decoded.codepoint);
    }

    buf_null_terminate_u8(&buf);
    str ret = buf_as_str(buf);
    return ret;
}

static wstr convert_utf8_to_utf16(str s, arena *out) {
    i64 max_len_as_utf16 = (s.count + 1) * 2;
    buf buf = arena_push_buf(out, max_len_as_utf16);

    unicode_codepoint decoded = { 0 };
    while (advance_single_utf8_codepoint(&s, &decoded)) {
        buf_append_utf16_codepoint(&buf, decoded.codepoint);
    }

    buf_null_terminate_u16(&buf);
    wstr ret = buf_as_wstr(buf);
    return ret;
}

static bool is_well_formed_utf8(str s) {
    i64 i = 0;
    str remaining = s;
    unicode_codepoint decoded = { 0 };
    while (advance_single_utf8_codepoint(&remaining, &decoded)) {
        i += decoded.len;
    }

    bool ret = s.count == i;
    return ret;
}

////////////////////////////////////////////////////////////////
// rune: Buffer

static void buf_append_utf8_codepoint(buf *buf, u32 append) {
    i32 num_units = utf8_class_from_u32(append);
    if (buf->count + num_units <= buf->cap) {
        encode_single_utf8_codepoint(append, buf->v + buf->count);
        buf->count += num_units;
    }
}

static void buf_append_utf16_codepoint(buf *buf, u32 append) {
    i32 num_units = utf16_class_from_u32(append);
    if (buf->count + num_units * 2 <= buf->cap) {
        encode_single_utf16_codepoint(append, (u16 *)(buf->v + buf->count));
        buf->count += num_units * 2;
    }
}

////////////////////////////////////////////////////////////////
// rune: Unicode iterator

static utf8_iter utf8_iter_begin(str s, i64 idx) {
    utf8_iter ret = { 0 };
    ret.s = s;
    ret.idx = idx;
    unicode_codepoint decoded = decode_single_utf8_codepoint(substr_idx(s, idx));
    ret.codepoint = decoded.codepoint;
    ret.valid = idx < s.count;
    return ret;
}

// TODO(rune): @Simplify.
static utf8_iter utf8_iter_next(utf8_iter iter, i32 dir) {
    iter.valid = false;
    switch (dir) {
        case 1: {
            if (iter.idx < iter.s.count) {
                i32 utf8_class = utf8_class_from_u8(iter.s.v[iter.idx]);
                iter.idx += utf8_class;
                unicode_codepoint decoded = decode_single_utf8_codepoint(substr_idx(iter.s, iter.idx));
                if (decoded.codepoint) {
                    iter.valid = true;
                    iter.codepoint = decoded.codepoint;
                } else {
                    iter.valid = false;
                    iter.codepoint = 0;
                }
            } else {
                iter.idx++;
            }
        } break;

        case -1: {
            if (iter.idx > 0) {
                while (1) {
                    iter.idx--;
                    if (iter.idx < 0) {
                        break;
                    }

                    i32 utf8_class = utf8_class_from_u8(iter.s.v[iter.idx]);
                    if (utf8_class != 0) {
                        unicode_codepoint decoded = decode_single_utf8_codepoint(substr_idx(iter.s, iter.idx));
                        if (decoded.codepoint) {
                            iter.codepoint = decoded.codepoint;
                            break;
                        }
                    }
                }

                iter.valid = iter.idx >= 0;
            } else {
                iter.idx--;
            }
        } break;
    }

    clamp_assign(&iter.idx, 0, iter.s.len);
    return iter;
}
