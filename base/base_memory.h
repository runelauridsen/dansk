////////////////////////////////////////////////////////////////
// rune: Macros

#define kilobytes(x)             ((x) * 1024LL)
#define megabytes(x)             (kilobytes(x) * 1024LL)
#define gigabytes(x)             (megabytes(x) * 1024LL)

#define zero_size(a, size)       (memset((a), 0, (size)))
#define zero_struct(a)           (memset((a), 0, sizeof(*(a))))
#define zero_array(a,c)          (memset((a), 0, sizeof(*(a)) * c))

////////////////////////////////////////////////////////////////
// rune: Heap

// NOTE(rune): MSVC magic memory numbers: https://stackoverflow.com/a/127404
// 0xabababab : Used by Microsoft's HeapAlloc() to mark "no man's land" guard bytes after allocated heap memory
// 0xcccccccc : Used by Microsoft's C++ debugging runtime library to mark uninitialised stack memory
// 0xcdcdcdcd : Used by Microsoft's C++ debugging runtime library to mark uninitialised heap memory
// 0xdddddddd : Used by Microsoft's C++ debugging heap to mark freed heap memory
// 0xfdfdfdfd : Used by Microsoft's C++ debugging heap to mark "no man's land" guard bytes before and after allocated heap memory
// 0xfeeefeee : Used by Microsoft's HeapFree() to mark freed heap memory

static void *   heap_alloc(i64 size);
static void *   heap_realloc(void *p, i64 new_size);
static void     heap_free(void *p);

////////////////////////////////////////////////////////////////
// rune: Arena (linear allocator)

typedef enum arena_kind {
    ARENA_KIND_LINEAR,      // rune: Chained blocks are the same size as previous block
    ARENA_KIND_EXPONENTIAL, // rune: Chained blocks are double the size as previous block
    ARENA_KIND_FIXED,       // rune: Do not allow chaining
    ARENA_KIND_CHAINED,     // rune: Marks that the block is not the "head" block of the chain
} arena_kind;

typedef struct arena arena;
struct arena {
    // rune: Memory block
    u8 *base;
    i64 pos;
    i64 cap;

    // rune: Chain links
    arena *next;
    arena *curr;

    // rune: Scope stack
    struct arena_scope *scope_head;

    // rune: Configuration
    arena_kind kind;
    u32 _unused[3];
};

typedef struct arena_mark arena_mark;
struct arena_mark {
    arena *curr;
    i64 pos;
};

typedef struct arena_scope arena_scope;
struct arena_scope {
    arena_mark mark;
    arena_scope *next;
};

// rune: Lifetime
static arena *arena_create(i64 cap, arena_kind kind);
static arena *arena_create_default(void);
static void   arena_destroy(arena *arena);

// rune: Push
static void *arena_push_size(arena *arena, i64 size, i64 align);
static void *arena_push_size_nozero(arena *arena, i64 size, i64 align);
#define      arena_push_struct(arena, T)             ((T *)arena_push_size       ((arena), sizeof(T),         alignof(T)))
#define      arena_push_struct_nozero(arena, T)      ((T *)arena_push_size_nozero((arena), sizeof(T),         alignof(T)))
#define      arena_push_array(arena, T, num)         ((T *)arena_push_size       ((arena), sizeof(T) * (num), alignof(T)))
#define      arena_push_array_nozero(arena, T, num)  ((T *)arena_push_size_nozero((arena), sizeof(T) * (num), alignof(T)))
static str   arena_push_str(arena *arena, i64 len, u8 val);
static str   arena_copy_str(arena *arena, str s);
static void *arena_copy_size(arena *arena, void *src, i64 size, i64 align);

// rune: Rewind
static void       arena_reset(arena *arena);
static arena_mark arena_mark_get(arena *arena);
static void       arena_mark_set(arena *arena, arena_mark mark);

// rune: Temporary scopes
#define           arena_scope(arena)  defer(arena_scope_begin(arena), arena_scope_end(arena))
static void       arena_scope_begin(arena *arena);
static void       arena_scope_end(arena *arena);

////////////////////////////////////////////////////////////////
// rune: Generic dynamic array

// TODO(rune): Remove darray(T)

#define darray(T)                                                array_##T
#define array_size(array)                                        ((array)->count * array_elem_size(array))
#define array_size_cap(array)                                    ((array)->cap * array_elem_size(array))
#define array_is_empty(array)                                    ((array)->count == 0)
#define array_first(array)                                       (*((array).count > 0 ? &(array).v[0]               : null))
#define array_first_or_default(array, default)                   ( ((array).count > 0 ?  (array).v[0]               : default))
#define array_last(array)                                        (*((array).count > 0 ? &(array).v[(array).count - 1] : null))
#define array_last_or_default(array, default)                    ( ((array).count > 0 ?  (array).v[(array).count - 1] : default))
#define array_front(array)                                       ((array).v)
#define array_back(array)                                        ((array).v + (array).count)
#define array_elem_size(array)                                   (sizeof(*(array)->v))
#define array_get(array, idx)                                    ((array)->v + (idx))

#define darray_create(array, initial_capacity)                    darray_create_     (&(array)->_void, array_elem_size(array), initial_capacity)
#define darray_destroy(array)                                     darray_destroy_    (&(array)->_void)
#define darray_reserve(array, count)                              darray_reserve_    (&(array)->_void, array_elem_size(array), count)
#define darray_add(array, count)                                  darray_add_        (&(array)->_void, array_elem_size(array), count)
#define darray_pop(array, count)                                  darray_pop_        (&(array)->_void, array_elem_size(array), count)
#define darray_remove(array, idx, count)                          darray_remove_     (&(array)->_void, array_elem_size(array), idx, count)
#define darray_insert(array, idx, count)                          darray_insert_     (&(array)->_void, array_elem_size(array), idx, count)
#define darray_reset(array)                                       darray_reset_      (&(array)->_void, array_elem_size(array))
#define darray_put(array, val)                                    (darray_add(array, 1) ? ((array)->v[(array)->count - 1] = (val), &(array)->v[(array)->count - 1]) : null)
#define array_idx_of(array, ptr)                                  ((ptr) - (array).v)

#define darray_struct(T)      struct { T *v; i64 count, cap; }
#define typedef_darray(T)     typedef union { darray_struct(T); array _void; } darray(T)

typedef darray_struct(void) array;

typedef_darray(u8);
typedef_darray(u16);
typedef_darray(u32);
typedef_darray(u64);
typedef_darray(i8);
typedef_darray(i16);
typedef_darray(i32);
typedef_darray(i64);

static bool darray_create_(array *array, i64 elem_size, i64 initial_cap);
static void darray_destroy_(array *array);
static bool darray_reserve_(array *array, i64 elem_size, i64 reserve_count);
static void *darray_add_(array *array, i64 elem_size, i64 add_count);
static void *darray_pop_(array *array, i64 elem_size, i64 pop_count);
static bool darray_remove_(array *array, i64 elem_size, i64 idx, i64 remove_count);
static void *darray_insert_(array *array, i64 elem_size, i64 idx, i64 insert_count);
static void *array_push_(array *array, i64 elem_size);
static void darray_reset_(array *array, i64 elem_size);
