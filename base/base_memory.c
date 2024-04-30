////////////////////////////////////////////////////////////////
// rune: Heap

static void *heap_alloc(i64 size) {
    void *ret = HeapAlloc(GetProcessHeap(), 0, u64(size));
    assert(ret);
#ifdef TRACK_ALLOCATIONS
    idk_track_alloc(IDK_LOCATION, ret, size);
#endif
    return ret;
}

static void *heap_realloc(void *p, i64 size) {
    void *ret = null;
    if (p == null) {
        ret = heap_alloc(size);
    } else {
        ret = HeapReAlloc(GetProcessHeap(), 0, p, u64(size));
    }

    assert(ret);
#ifdef TRACK_ALLOCATIONS
    idk_track_realloc(IDK_LOCATION, p, ret, size);
#endif
    return ret;
}

static void heap_free(void *p) {
    if (p) {
        HeapFree(GetProcessHeap(), 0, p);
#ifdef TRACK_ALLOCATIONS
        idk_track_free(IDK_LOCATION, p);
#endif
    }
}

////////////////////////////////////////////////////////////////
// rune: Arena

static arena *arena_create(i64 cap, arena_kind kind) {
    u8 *base = (u8 *)heap_alloc(cap);
    memset(base, 0, sizeof(struct arena));

    arena *arena   = (struct arena *)base;
    arena->base    = base;
    arena->kind    = kind;
    arena->cap     = cap;
    arena->pos     = sizeof(struct arena);
    arena->curr    = arena;

    return arena;
}

static arena *arena_create_default(void) {
    return arena_create(kilobytes(64), ARENA_KIND_LINEAR);
}

static void arena_destroy(arena *arena) {
    while (arena) {
        struct arena *next = arena->next;
        heap_free(arena->base);
        arena = next;
    }
}

static void arena_reset(arena *arena) {
    arena->curr = arena;
    arena->pos = sizeof(struct arena);
}

static void *arena_push_size(arena *arena, i64 size, i64 align) {
    void *ret = arena_push_size_nozero(arena, size, align);
    memset(ret, 0, u64(size));
    return ret;
}

static void *arena_push_size_nozero(arena *arena, i64 size, i64 align) {
    // rune: Defaults to  1-byte alignment
    if (align == 0) align = 1;

    // rune: Is there any space left in the current block?
    struct arena *curr = arena->curr;
    if (i64_align_to_pow2(curr->pos, align) + size > curr->cap) {
        // rune: Find previously allocated block
        curr = arena->curr->next;
        while (curr) {
            arena->curr = curr;
            curr->pos = sizeof(struct arena);
            if (i64_align_to_pow2(curr->pos, align) + size > curr->cap) {
                curr = curr->next;
            } else {
                break;
            }
        }

        // rune: Allocate a new block if none found
        if (curr == null) {
            i64 next_cap = 0;
            switch (arena->kind) {
                case ARENA_KIND_LINEAR: {
                    next_cap = arena->curr->cap;
                } break;

                case ARENA_KIND_EXPONENTIAL: {
                    next_cap = arena->curr->cap * 2;
                } break;

                default: {
                    assert(false && "Arena overflow");
                    exit(1);
                } break;
            }

            next_cap = max(next_cap, size + i64_align_to_pow2(sizeof(struct arena), align)); // Must fit the requested align + size
            next_cap = i64_round_up_to_pow2(next_cap); // Must be a power of two.

            curr = arena_create(next_cap, ARENA_KIND_CHAINED);
            arena->curr->next = curr;
        }

        arena->curr = curr;
    }

    // rune: Push align + size
    curr->pos = i64_align_to_pow2(curr->pos, align);
    void *ptr = curr->base + curr->pos;
    curr->pos += size;
    assert(curr->pos <= curr->cap);
    return ptr;
}

static void *arena_copy_size(arena *arena, void *src, i64 size, i64 align) {
    void *dst = arena_push_size_nozero(arena, size, align);
    memcpy(dst, src, u64(size));
    return dst;
}

static str arena_copy_str(arena *arena, str s) {
    str ret = { 0 };
    ret.v = (u8 *)arena_push_size_nozero(arena, s.len + 1, 0);
    ret.len = s.len;
    memcpy(ret.v, s.v, u64(s.len));
    ret.v[ret.len] = '\0';
    return ret;
}

static str arena_push_str(arena *arena, i64 len, u8 val) {
    str ret = { 0 };
    ret.v = (u8 *)arena_push_size_nozero(arena, len + 1, 0);
    ret.len = len;
    memset(ret.v, val, u64(ret.len));
    ret.v[len] = '\0';
    return ret;
}

static arena_mark arena_mark_get(arena *arena) {
    arena_mark mark = { 0 };
    mark.curr = arena->curr;
    mark.pos = arena->pos;
    return mark;
}

static void arena_mark_set(arena *arena, arena_mark mark) {
    assert(mark.curr != 0);
    assert(mark.pos <= mark.curr->cap);

    arena->curr = mark.curr;
    arena->pos  = mark.pos;
}

static void arena_scope_begin(arena *arena) {
    arena_mark mark = arena_mark_get(arena);
    arena_scope *scope = arena_push_struct_nozero(arena, arena_scope);
    scope->mark = mark;
    slstack_push(&arena->scope_head, scope);
}

static void arena_scope_end(arena *arena) {
    assert(arena->scope_head && "Arena scope stack underflow.");
    arena_mark mark = arena->scope_head->mark;
    arena_mark_set(arena, mark);
    slstack_pop(&arena->scope_head);
}

////////////////////////////////////////////////////////////////
// rune: Dynamic array

static bool darray_create_(array *array, i64 elem_size, i64 initial_cap) {
    assert(array);
    assert(initial_cap);
    assert(elem_size);

    array->v        = heap_alloc(initial_cap * elem_size);
    array->cap      = initial_cap;
    array->count    = 0;
    return true;
}

static void darray_destroy_(array *array) {
    if (array) {
        heap_free(array->v);
        zero_struct(array);
    }
}

static bool darray_reserve_(array *array, i64 elem_size, i64 reserve_count) {
    bool ret = false;

    if (array->cap < reserve_count) {
        i64 new_cap = 0;
        void *new_elems;
        if (array->v == null) {
            new_cap = reserve_count;
            new_elems = heap_alloc(new_cap * elem_size);
        } else {
            new_cap = array->cap;
            while (new_cap < reserve_count) {
                new_cap *= 2;
            }

            new_elems = heap_realloc(array->v, new_cap * elem_size);
        }
        if (new_elems) {
            array->v   = new_elems;
            array->cap = new_cap;

            ret = true;
        }
    } else {
        ret = true;
    }

    return ret;
}

static void *darray_add_(array *array, i64 elem_size, i64 add_count) {
    void *ret = null;

    if (darray_reserve_(array, elem_size, array->count + add_count)) {
        ret = (u8 *)array->v + array->count * elem_size;
        array->count += add_count;
    }

    return ret;
}

static void *darray_pop_(array *array, i64 elem_size, i64 pop_count) {
    void *ret = null;

    if (array->count >= pop_count) {
        array->count  -= pop_count;
        ret = (u8 *)array->v + array->count * elem_size;
    }

    return ret;
}

static bool darray_remove_(array *array, i64 elem_size, i64 idx, i64 remove_count) {
    assert(remove_count > 0);

    bool ret = false;

    if (array->count >= idx + remove_count) {
        memmove((u8 *)array->v + elem_size * idx,
                (u8 *)array->v + elem_size * (idx + remove_count),
                (u64)(elem_size * (array->count - idx - remove_count)));

        array->count -= remove_count;
        ret = true;
    }

    return ret;
}

static void *darray_insert_(array *array, i64 elem_size, i64 idx, i64 insert_count) {
    assert(insert_count > 0);

    void *ret = null;

    if (darray_reserve_(array, elem_size, array->count + insert_count)) {
        memmove((u8 *)array->v + elem_size * (idx + insert_count),
                (u8 *)array->v + elem_size * idx,
                (u64)((array->count - idx) * elem_size));

        array->count += insert_count;

        ret = (u8 *)array->v + elem_size * idx;
    }

    return ret;
}


static void *array_push_(array *array, i64 elem_size) {
    void *ret = darray_add_(array, elem_size, 1);
    return ret;
}

static void darray_reset_(array *array, i64 elem_size) {
    unused(elem_size);
    array->count = 0;
}
