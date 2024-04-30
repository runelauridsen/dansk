////////////////////////////////////////////////////////////////
// rune: Decode/encode

typedef struct unicode_codepoint unicode_codepoint;
struct unicode_codepoint {
    u32 codepoint;
    u32 len;
};

static i32  utf8_class_from_u8(u8 codeunit);
static i32  utf8_class_from_u32(u32 codepoint);
static i32  utf16_class_from_u16(u16 codeunit);
static i32  utf16_class_from_u32(u32 codepoint);
static bool utf8_is_continuation(u8 codeunit);
static bool utf16_is_continuation(u8 codeunit);
static unicode_codepoint decode_single_utf8_codepoint(str s);
static unicode_codepoint decode_single_utf16_codepoint(wstr w);
static i32 encode_single_utf8_codepoint(u32 codepoint, u8 *out);
static i32 encode_single_utf16_codepoint(u32 codepoint, u16 *out);
static bool advance_single_utf8_codepoint(str *s, unicode_codepoint *decoded);
static bool advance_single_utf16_codepoint(wstr *w, unicode_codepoint *decoded);
static bool is_well_formed_utf8(str s);
static bool is_well_formed_utf16(wstr s);

////////////////////////////////////////////////////////////////
// rune: Buffer

static void buf_append_utf8_codepoint(buf *buffer, u32 append);
static void buf_append_utf16_codepoint(buf *buffer, u32 append);

////////////////////////////////////////////////////////////////
// rune: Conversions

// NOTE(rune): Conversion results are zero-terminated.
static str convert_utf16_to_utf8(wstr w, arena *out);
static wstr convert_utf8_to_utf16(str s, arena *out);

////////////////////////////////////////////////////////////////
// rune: Iterator

typedef struct utf8_iter utf8_iter;
struct utf8_iter {
    str s;
    i64 idx;
    u32 codepoint;
    bool valid;
};

static utf8_iter utf8_iter_begin(str s, i64 idx);
static utf8_iter utf8_iter_next(utf8_iter iter, i32 dir);
