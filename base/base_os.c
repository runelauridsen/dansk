////////////////////////////////////////////////////////////////
// rune: Handles

static void os_handle_destroy(os_handle handle) {
    switch (handle.kind) {
        case OS_HANDLE_KIND_MUTEX:  os_mutex_destroy(handle);  break;
        case OS_HANDLE_KIND_THREAD: os_thread_destroy(handle); break;
        case OS_HANDLE_KIND_COND:   os_cond_destroy(handle);   break;
    }
}

////////////////////////////////////////////////////////////////
// rune: Prof scope

typedef struct prof_scope prof_scope;
struct prof_scope {
    bool init;
    u64 t_begin;
    u64 t_end;
};

static inline bool prof_scope_func(prof_scope *scope, char *name) {
    if (!scope->init) {
        scope->t_begin = os_get_performance_timestamp();
        scope->init = true;
        return true;
    } else {
        scope->t_end = os_get_performance_timestamp();
        f64 ms = os_get_millis_between(scope->t_begin, scope->t_end);
        if (ms < 1) {
            println("% us \t %", ms * 1000.0f, name);
        } else {
            println("% ms \t %", ms, name);
        }
        return false;
    }
}

#ifdef NDEBUG
#define prof_scope(name) for(prof_scope _scope_ = {0}; prof_scope_func(&_scope_, name);)
#else
#define prof_scope(...)
#endif

#if _WIN32

////////////////////////////////////////////////////////////////
// rune: Object list

static void os_object_list_push(os_object_list *list, os_object *node) {
    dlist_push(list, node);
    list->count += 1;
}

static os_object *os_object_list_pop(os_object_list *list) {
    os_object *node = list->last;
    os_object_list_remove(list, node);
    return node;
}

static void os_object_list_remove(os_object_list *list, os_object *node) {
    if (node) {
        list->count -= 1;
        dlist_remove(list, node);
    }
}

////////////////////////////////////////////////////////////////
// rune: Objects and handles

static os_object *os_object_alloc(os_handle_kind kind) {
    os_object *object = null;
    os_mutex_scope(os_g_mutex) {
        object = os_object_list_pop(&os_g_object_free_list);
        if (object) {
            i32 gen = object->gen;
            memset(object, 0, sizeof(os_object));
            object->gen = gen;
        } else {
            object = arena_push_struct(os_g_arena, os_object);
            object->gen = 1;
        }

        object->kind = kind;
        os_object_list_push(&os_g_object_alive_list, object);
    }
    return object;
}

static void os_object_free(os_object *object) {
    os_mutex_scope(os_g_mutex) {
        object->gen += 1;
        os_object_list_remove(&os_g_object_alive_list, object);
        os_object_list_push(&os_g_object_free_list, object);
    }
}

static os_handle os_handle_from_object(os_object *object) {
    os_handle handle = { 0 };
    if (object) {
        handle.kind = object->kind;
        handle.gen  = object->gen;
        handle.data = object;
    }
    return handle;
}

static os_object *os_object_from_handle(os_handle handle) {
    os_object *object = handle.data;
    if (object && object->gen != handle.gen) {
        object = null;
    }
    return object;
}

////////////////////////////////////////////////////////////////
// rune: Intialization

static void os_init(void) {
    os_g_arena = arena_create_default();
    os_g_mutex = os_mutex_create();
}

static void os_deinit(void) {
    os_handle mutex = os_g_mutex;
    os_g_mutex = (os_handle) { 0 };
    os_mutex_destroy(mutex);
    arena_destroy(os_g_arena);
}

////////////////////////////////////////////////////////////////
// rune: Threads

static DWORD os_thread_start_address(void *lpParameter) {
    os_object *object = lpParameter;
    object->proc(object->param);
    return 0;
}

static os_handle os_thread_create(os_thread_proc *proc, void *param) {
    os_handle handle = { 0 };
    os_object *object = os_object_alloc(OS_HANDLE_KIND_THREAD);
    if (object) {
        object->proc = proc;
        object->param = param;
        object->handle = CreateThread(0, 0, os_thread_start_address, object, 0, &object->thread_id);
        handle = os_handle_from_object(object);
    }
    return handle;
}

static void os_thread_destroy(os_handle thread) {
    os_object *object = os_object_from_handle(thread);
    if (object) {
        CloseHandle(object->handle);
        os_object_free(object);
    }
}

static void os_thread_join(os_handle thread) {
    os_object *object = os_object_from_handle(thread);
    if (object) {
        WaitForSingleObject(object->handle, INFINITE);
        CloseHandle(object->handle);
        os_object_free(object);
    }
}

static void os_thread_sleep(i32 millis) {
    Sleep((DWORD)millis);
}

////////////////////////////////////////////////////////////////
// rune: Mutexes

static os_handle os_mutex_create(void) {
    os_handle mutex = { 0 };
    os_object *object = os_object_alloc(OS_HANDLE_KIND_MUTEX);
    if (object) {
        InitializeSRWLock(&object->srwlock);
        mutex = os_handle_from_object(object);
    }
    return mutex;
}

static void os_mutex_destroy(os_handle mutex) {
    os_object *object = os_object_from_handle(mutex);
    if (object) {
        os_object_free(object);
    }
}

static void os_mutex_acquire(os_handle mutex) {
    os_object *object = os_object_from_handle(mutex);
    if (object) {
        AcquireSRWLockExclusive(&object->srwlock);
    }
}

static void os_mutex_release(os_handle mutex) {
    os_object *object = os_object_from_handle(mutex);
    if (object) {
        ReleaseSRWLockExclusive(&object->srwlock);
    }
}

////////////////////////////////////////////////////////////////
// rune: Condition variables

static os_handle os_cond_create(void) {
    os_handle cond = { 0 };
    os_object *object = os_object_alloc(OS_HANDLE_KIND_COND);
    if (object) {
        InitializeConditionVariable(&object->cond);
        cond = os_handle_from_object(object);
    }
    return cond;
}

static void os_cond_destroy(os_handle cond) {
    os_object *object = os_object_from_handle(cond);
    if (object) {
        os_object_free(object);
    }
}

static void os_cond_wait(os_handle cond, os_handle mutex, u32 timeout_ms) {
    if (timeout_ms > 0) {
        os_object *cond_object = os_object_from_handle(cond);
        os_object *mutex_object = os_object_from_handle(mutex);
        if (cond_object && mutex_object) {
            SleepConditionVariableSRW(&cond_object->cond, &mutex_object->srwlock, timeout_ms, 0);
        }
    }
}

static void os_cond_signal(os_handle cond) {
    os_object *object = os_object_from_handle(cond);
    if (object) {
        WakeConditionVariable(&object->cond);
    }
}

static void os_cond_signal_all(os_handle cond) {
    os_object *object = os_object_from_handle(cond);
    if (object) {
        WakeAllConditionVariable(&object->cond);
    }
}

////////////////////////////////////////////////////////////////
// rune: File iterator

static bool os_valid_handle(HANDLE handle) {
    bool ret = (handle && handle != INVALID_HANDLE_VALUE);
    return ret;
}

static u32 os_get_logical_drives(void) {
    return GetLogicalDrives();
}

////////////////////////////////////////////////////////////////
// rune: Entire file

// TODO(rune): Larger than 4gb files.
static str os_read_entire_file(str file_name, arena *arena, bool *succeeded) {
    str ret = { 0 };

    HANDLE file = INVALID_HANDLE_VALUE;
    arena_scope(arena) {
        wstr wname = convert_utf8_to_utf16(file_name, arena);
        file = CreateFileW((LPWSTR)wname.v, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    }

    arena_mark mark = arena_mark_get(arena);

    bool local_succeeded = false;
    if (os_valid_handle(file)) {
        u32 file_size = GetFileSize(file, 0);
        ret.v = arena_push_size_nozero(arena, file_size + 1, 0);
        if (ret.v) {
            ret.v[0] = '\0';

            DWORD bytes_read = 0;
            if (ReadFile(file, ret.v, file_size, &bytes_read, 0)) {
                if (bytes_read == file_size) {
                    ret.count         = bytes_read;
                    ret.v[bytes_read] = '\0';

                    local_succeeded = true;
                }
            }
        }

        CloseHandle(file);
    }

    if (!local_succeeded) {
        arena_mark_set(arena, mark);
    }

    if (succeeded) *succeeded = local_succeeded;

    return ret;
}

// TODO(rune): Larger than 4gb files.
static void os_write_entire_file(str file_name, str data, arena *arena, bool *succeeded) {
    bool local_succeeded = false;

    HANDLE file = INVALID_HANDLE_VALUE;
    arena_scope(arena) {
        wstr wname = convert_utf8_to_utf16(file_name, arena);
        file = CreateFileW((LPWSTR)wname.v, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    }

    if (os_valid_handle(file)) {
        DWORD bytes_written = 0;
        if (WriteFile(file, data.v, (DWORD)data.len, &bytes_written, 0)) {
            if (bytes_written == (DWORD)data.len) {
                local_succeeded = true;
            }
        }

        CloseHandle(file);
    }

    if (succeeded) *succeeded = local_succeeded;
}

////////////////////////////////////////////////////////////////
// rune: File info

static os_file_info *os_file_info_list_push(os_file_info_list *list, arena *arena) {
    os_file_info *node = arena_push_struct(arena, os_file_info);
    slist_push(list, node);
    list->count += 1;
    return node;
}

static os_file_flags os_file_flags_from_dword(DWORD dword) {
    os_file_flags flags = 0;
    if (dword & FILE_ATTRIBUTE_DIRECTORY) flags |= OS_FILE_FLAG_DIRECTORY;
    return flags;
}

static os_file_info_list os_get_files_from_path(str path, i64 max, arena *arena) {
    os_file_info_list infos = { 0 };

    HANDLE find_handle = 0;
    WIN32_FIND_DATAW find_data = { 0 };

    arena_scope(arena) {
        if (path.len > 0) {
            str pattern = arena_print(arena, "%\\*", path);
            wstr wpath = convert_utf8_to_utf16(pattern, arena);
            find_handle = FindFirstFileW((LPWSTR)wpath.v, &find_data);
        }
    }

    if (os_valid_handle(find_handle)) {
        do {
            wstr wname = wstr_make(find_data.cFileName, 0);
            for_n (i64, i, countof(find_data.cFileName)) {
                wname.count = i;
                if (find_data.cFileName[i] == '\0') {
                    break;
                }
            }

            if (wstr_eq(wname, wstr(L"")))   continue;
            if (wstr_eq(wname, wstr(L".")))  continue;
            if (wstr_eq(wname, wstr(L".."))) continue;

            os_file_info *info = os_file_info_list_push(&infos, arena);
            info->name = convert_utf16_to_utf8(wname, arena);
            info->flags = os_file_flags_from_dword(find_data.dwFileAttributes);

            if (infos.count >= max) break;

        } while (FindNextFileW(find_handle, &find_data));
    }

    return infos;
}

////////////////////////////////////////////////////////////////
// rune: Performance counter

static u64 os_get_performance_timestamp(void) {
    u64 ret = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&ret);
    return ret;
}

static f64 os_get_millis_between(u64 t_begin, u64 t_end) {
    u64 diff = t_end - t_begin;
    u64 freq = 1;
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    f64 ret = 1000.0 * f64(diff) / f64(freq);
    return ret;
}

#endif
