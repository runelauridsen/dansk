////////////////////////////////////////////////////////////////
// rune: Lifetime

static void os_init(void);
static void os_deinit(void);

////////////////////////////////////////////////////////////////
// rune: Handles

typedef enum os_handle_kind {
    OS_HANDLE_KIND_NONE,
    OS_HANDLE_KIND_FILE,
    OS_HANDLE_KIND_THREAD,
    OS_HANDLE_KIND_MUTEX,
    OS_HANDLE_KIND_COND,

    OS_HANDLE_KIND_COUNT,
} os_handle_kind;

typedef struct os_handle os_handle;
struct os_handle {
    os_handle_kind kind;
    i32 gen;
    void *data;
};

static void os_handle_destroy(os_handle handle);

////////////////////////////////////////////////////////////////
// rune: Threads

typedef void os_thread_proc(void *param);

static os_handle os_thread_create(os_thread_proc *proc, void *param);
static void      os_thread_destroy(os_handle thread);
static void      os_thread_join(os_handle thread);
static void      os_thread_sleep(i32 millis);

////////////////////////////////////////////////////////////////
// rune: Mutexes

static os_handle os_mutex_create(void);
static void      os_mutex_destroy(os_handle mutex);
static void      os_mutex_acquire(os_handle mutex);
static void      os_mutex_release(os_handle mutex);
#define          os_mutex_scope(mutex) defer(os_mutex_acquire(mutex), os_mutex_release(mutex))

////////////////////////////////////////////////////////////////
// rune: Condition variables

static os_handle os_cond_create(void);
static void      os_cond_destroy(os_handle cond);
static void      os_cond_wait(os_handle cond, os_handle mutex, u32 timeout_ms);
static void      os_cond_signal(os_handle cond);
static void      os_cond_signal_all(os_handle cond);

////////////////////////////////////////////////////////////////
// rune: Entire file

static str  os_read_entire_file(str file_name, arena *arena, bool *succeeded);
static void os_write_entire_file(str file_name, str data, arena *arena, bool *succeeded);

////////////////////////////////////////////////////////////////
// rune: File info

typedef enum os_file_flags {
    OS_FILE_FLAG_DIRECTORY = 0x1,
} os_file_flags;

typedef struct os_file_info os_file_info;
struct os_file_info {
    str name;
    os_file_flags flags;
    os_file_info *next;
};

typedef struct os_file_info_list os_file_info_list;
struct os_file_info_list {
    os_file_info *first;
    os_file_info *last;
    i64 count;
};

static os_file_info_list os_get_files_from_path(str path, i64 max, arena *arena);

////////////////////////////////////////////////////////////////
// rune: Performance counter

static u64 os_get_performance_timestamp(void);
static f64 os_get_millis_between(u64 t_begin, u64 t_end);

#if _WIN32

////////////////////////////////////////////////////////////////
// rune: Objects

typedef struct os_object os_object;
struct os_object {
    os_handle_kind kind;
    i32 gen;

    void *param;
    os_thread_proc *proc;
    DWORD thread_id;

    HANDLE handle;
    SRWLOCK srwlock;
    CONDITION_VARIABLE cond;

    os_object *next;
    os_object *prev;
};

typedef struct os_object_list os_object_list;
struct os_object_list {
    os_object *first;
    os_object *last;
    i64 count;
};

////////////////////////////////////////////////////////////////
// rune: Globals

static arena *                os_g_arena = 0;
static os_handle              os_g_mutex = { 0 };
static os_object_list         os_g_object_alive_list = { 0 };
static os_object_list         os_g_object_free_list = { 0 };

////////////////////////////////////////////////////////////////
// rune: Object list

static void             os_object_list_push(os_object_list *list, os_object *node);
static os_object *      os_object_list_pop(os_object_list *list);
static void             os_object_list_remove(os_object_list *list, os_object *node);

////////////////////////////////////////////////////////////////
// rune: Objects and handles

static os_object *os_object_alloc(os_handle_kind kind);
static void       os_object_free(os_object *object);
static os_handle  os_handle_from_object(os_object *object);
static os_object *os_object_from_handle(os_handle handle);

////////////////////////////////////////////////////////////////
// rune: Threads

static DWORD os_thread_start_address(void *lpParameter);

#endif



