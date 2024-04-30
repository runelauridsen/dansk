////////////////////////////////////////////////////////////////
// rune: Fixed sized buffer

typedef struct buf buf;
struct buf {
    u8 *v;
    union { i64 len, count; };
    i64 cap;
};

static buf make_buf(void *mem, i64 cap, i64 len);
static buf arena_push_buf(arena *arena, i64 cap);
static str buf_as_str(buf buf);
static void buf_reset(buf *buf);
static str buf_as_str(buf buf);
static wstr buf_as_wstr(buf buf);
static void buf_append_u8(buf *buf, u8 append);
static void buf_append_str(buf *buf, str append);
static void *buf_push_size(buf *buf, i64 size);
static void buf_replace(buf *buf, i64_range replace, str replace_with);
static void buf_delete(buf *buf, i64_range range);
static void buf_insert(buf *buf, i64 insert_at, str insert);
static void buf_null_terminate_u8(buf *buf);
static void buf_null_terminate_u16(buf *buf);

#define buf_push_struct(buf, T) ((T *)buf_push_size((buf), sizeof(T)))
