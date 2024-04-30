////////////////////////////////////////////////////////////////
// rune: Fixed sized buffer

static buf make_buf(void *mem, i64 cap, i64 len) {
    buf ret = {
        .v = mem,
        .cap = cap,
        .len = len,
    };
    return ret;
}

static buf arena_push_buf(arena *arena, i64 cap) {
    buf ret = {
        .v   = arena_push_array(arena, u8, cap),
        .cap = cap,
        .len = 0,
    };
    return ret;
}

static void buf_reset(buf *buf) {
    buf->count = 0;
}

static str buf_as_str(buf buf) {
    str ret = str_make(buf.v, buf.count);
    return ret;
}

static wstr buf_as_wstr(buf buf) {
    assert(buf.count % 2 == 0);
    wstr ret = wstr_make((u16 *)buf.v, buf.count / 2);
    return ret;
}

static void buf_append_u8(buf *buf, u8 append) {
    if (buf->count < buf->cap) {
        buf->v[buf->count++] = append;
    }
}

static void buf_append_str(buf *buf, str append) {
    clamp_assign(&append.len, 0, buf->cap - buf->count);
    memcpy(buf->v + buf->count, append.v, append.len);
    buf->count += append.len;
}

static void *buf_push_size(buf *buf, i64 size) {
    void *ret = null;
    if (buf->len + size <= buf->cap) {
        ret = buf->v + buf->len;
        buf->len += size;
    }
    return ret;
}

static void buf_replace(buf *buf, i64_range replace, str replace_with) {
    clamp_assign(&replace.min, 0, buf->len);
    clamp_assign(&replace.max, 0, buf->len);

    i64 replace_len = replace.max - replace.min;
    clamp_assign(&replace_with.len, 0, buf->cap - buf->count + replace_len);

    u8 *move_dst    = buf->v + replace.min + replace_with.len;
    u8 *move_src    = buf->v + replace.max;
    i64  move_size   = buf->count - replace.max;

    memmove(move_dst, move_src, move_size);
    memcpy(buf->v + replace.min, replace_with.v, replace_with.len);

    buf->count -= replace_len;
    buf->count += replace_with.len;
}

static void buf_delete(buf *buf, i64_range range) {
    buf_replace(buf, range, str(""));
}

static void buf_insert(buf *buf, i64 insert_at, str insert) {
    buf_replace(buf, i64_range(insert_at, insert_at), insert);
}

static void buf_null_terminate_u8(buf *buf) {
    if (buf->count < buf->cap) {
        buf->v[buf->count] = '\0';
    } else if (buf->cap > 0) {
        buf->v[buf->cap - 1] = '\0';
    }
}

static void buf_null_terminate_u16(buf *buf) {
    if (buf->count + 1 < buf->cap) {
        buf->v[buf->count] = '\0';
        buf->v[buf->count + 1] = '\0';
    } else if (buf->cap > 0) {
        buf->v[buf->cap - 2] = '\0';
        buf->v[buf->cap - 1] = '\0';
    }
}

static i64 buf_remaining_size(buf buf) {
    return buf.cap - buf.len;
}
