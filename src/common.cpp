#define COMMON_ALLOCATOR(name) void * name(size_t size)
typedef COMMON_ALLOCATOR(Common_Alloc);

COMMON_ALLOCATOR(common_alloc_default) {
    void *result = malloc(size);

    return result;
}

Common_Alloc *common_alloc = common_alloc_default;

internal_proc void *
xcalloc(size_t num, size_t size) {
    void *mem = calloc(num, size);

    if ( !mem ) {
        ASSERT(!"speicher konnte nicht reserviert werden!");
        exit(1);
    }

    return mem;
}

internal_proc void *
xrealloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        ASSERT(!"xrealloc failed");
        exit(1);
    }
    return ptr;
}

internal_proc void *
memdup(void *mem, size_t size) {
    void *result = common_alloc(size);

    memcpy(result, mem, size);

    return result;
}

enum Status_Kind {
    STATUS_OK,
    STATUS_WARNING,
    STATUS_ERROR,
};
struct Status {
    Status_Kind kind;
    char *filename;
    s64   line;
    char  message[1024];

    Status **errors;
    size_t num_errors;

    Status **warnings;
    size_t num_warnings;
};

global_var Status global_status = {STATUS_OK};

internal_proc Status *
status_error(char *filename, s64 line, char *message) {
    Status *result = (Status *)common_alloc(sizeof(Status));

    result->kind = STATUS_ERROR;
    result->filename = filename;
    result->line = line;
    memcpy(result->message, message, utf8_str_len(message));

    return result;
}

internal_proc Status *
status_warning(char *filename, s64 line, char *message) {
    Status *result = (Status *)common_alloc(sizeof(Status));

    result->kind = STATUS_WARNING;
    result->filename = filename;
    result->line = line;
    memcpy(result->message, message, utf8_str_len(message));

    return result;
}

user_api char *
status_message(Status *status = &global_status) {
    if ( !status ) {
        return "";
    }

    return status->message;
}

user_api char *
status_filename(Status *status = &global_status) {
    if ( !status ) {
        return "";
    }

    return status->filename;
}

user_api s64
status_line(Status *status = &global_status) {
    if ( !status ) {
        return 0;
    }

    return status->line;
}

internal_proc void
status_set(Status_Kind kind, char *filename, s64 line, char *message) {
    global_status.kind     = kind;
    global_status.filename = filename;
    global_status.line     = line;
    memcpy(global_status.message, message, utf8_str_len(message));
}

internal_proc void
status_set_error(char *file, s64 line, char *message) {
    status_set(STATUS_ERROR, file, line, message);
}

user_api b32
status_is_error() {
    b32 result = global_status.kind == STATUS_ERROR;

    return result;
}

user_api b32
status_is_not_error() {
    b32 result = global_status.kind != STATUS_ERROR;

    return result;
}

internal_proc void
status_set_warning(char *file, s64 line, char *message) {
    status_set(STATUS_WARNING, file, line, message);
}

user_api b32
status_is_warning() {
    b32 result = global_status.kind == STATUS_WARNING;

    return result;
}

user_api b32
status_is_not_warning() {
    b32 result = global_status.kind != STATUS_WARNING && status_is_not_error();

    return result;
}

internal_proc void
status_set_ok() {
    status_set(STATUS_OK, 0, 0, "");
}

user_api void
status_reset() {
    status_set_ok();
}

user_api size_t
status_num_errors() {
    return global_status.num_errors;
}

user_api Status *
status_error_get(size_t idx) {
    Status *result = 0;

    if ( idx >= global_status.num_errors ) {
        return result;
    }

    result = global_status.errors[idx];
    return result;
}

user_api size_t
status_num_warnings() {
    return global_status.num_warnings;
}

user_api Status *
status_warning_get(size_t idx) {
    Status *result = 0;

    if ( idx >= global_status.num_warnings ) {
        return result;
    }

    result = global_status.warnings[idx];
    return result;
}

internal_proc char *
strf(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int size = 1 + vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *str = (char *)common_alloc(size);

    va_start(args, fmt);
    vsnprintf(str, size, fmt, args);
    va_end(args);

    return str;
}

struct Buf_Hdr {
    size_t len;
    size_t cap;
    char buf[1];
};

internal_proc void *
buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    ASSERT(buf_cap(buf) <= (SIZE_MAX - 1)/2);

    size_t new_cap = CLAMP_MIN(2*buf_cap(buf), MAX(new_len, 16));
    ASSERT(new_len <= new_cap);
    ASSERT(new_cap <= (SIZE_MAX - offsetof(Buf_Hdr, buf))/elem_size);
    size_t new_size = offsetof(Buf_Hdr, buf) + new_cap*elem_size;

    Buf_Hdr *new_hdr;
    if (buf) {
        new_hdr = (Buf_Hdr *)xrealloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = (Buf_Hdr *)common_alloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

internal_proc char *
buf__printf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t cap = buf_cap(buf) - buf_len(buf);
    size_t n = 1 + vsnprintf(buf_end(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        buf_fit(buf, n + buf_len(buf));
        va_start(args, fmt);
        size_t new_cap = buf_cap(buf) - buf_len(buf);
        n = 1 + vsnprintf(buf_end(buf), new_cap, fmt, args);
        ASSERT(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}

internal_proc void
fatal(char *file, s64 line, char *msg, ...) {
    va_list args;
    va_start(args, msg);

    char buf[255];
    vsnprintf(buf, 255, msg, args);

    va_end(args);

    buf_push(global_status.errors, status_error(file, line, buf));
    global_status.num_errors = buf_len(global_status.errors);

    if ( status_is_not_error() ) {
        status_set_error(file, line, buf);
    }
}

internal_proc void
warn(char *file, s64 line, char *msg, ...) {
    va_list args;
    va_start(args, msg);

    char buf[255];
    vsnprintf(buf, 255, msg, args);

    va_end(args);

    buf_push(global_status.warnings, status_warning(file, line, buf));
    global_status.num_warnings = buf_len(global_status.warnings);

    if ( status_is_not_warning() ) {
        status_set_warning(file, line, buf);
    }
}

internal_proc uint64_t
uint64_hash(uint64_t x) {
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;

    return x;
}

internal_proc uint64_t
ptr_hash(void *ptr) {
    return uint64_hash((uintptr_t)ptr);
}

internal_proc uint64_t
mix_hash(uint64_t x, uint64_t y) {
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;

    return x;
}

internal_proc uint64_t
bytes_hash(void *ptr, size_t len) {
    uint64_t x = 0xcbf29ce484222325;
    char *buf = (char *)ptr;

    for (size_t i = 0; i < len; i++) {
        x ^= buf[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }

    return x;
}

struct Map {
    void **vals;
    void **keys;
    size_t len;
    size_t cap;
};

internal_proc void *
map_get(Map *map, void *key) {
    if (map->len == 0) {
        return NULL;
    }

    ASSERT(IS_POW2(map->cap));
    size_t i = (size_t)ptr_hash(key);
    ASSERT(map->len < map->cap);

    for (;;) {
        i &= map->cap - 1;

        if ( map->keys[i] == key ) {
            return map->vals[i];
        } else if ( !map->keys[i] ) {
            return NULL;
        }
        i++;
    }

    return NULL;
}

internal_proc void map_put(Map *map, void *key, void *val);

internal_proc void
map_grow(Map *map, size_t new_cap) {
    new_cap = MAX(16, new_cap);
    Map new_map = {};

    new_map.keys = (void **)xcalloc(new_cap, sizeof(void *));
    new_map.vals = (void **)common_alloc(new_cap * sizeof(void *));
    new_map.cap  = new_cap;

    for ( size_t i = 0; i < map->cap; i++ ) {
        if ( map->keys[i] ) {
            map_put(&new_map, map->keys[i], map->vals[i]);
        }
    }

    free(map->keys);
    free(map->vals);
    *map = new_map;
}

internal_proc void
map_put(Map *map, void *key, void *val) {
    ASSERT(key);
    ASSERT(val);

    if (2*map->len >= map->cap) {
        map_grow(map, 2*map->cap);
    }

    ASSERT(2*map->len < map->cap);
    ASSERT(IS_POW2(map->cap));

    size_t i = (size_t)ptr_hash(key);
    for (;;) {
        i &= map->cap - 1;

        if ( !map->keys[i] ) {
            map->len++;
            map->keys[i] = key;
            map->vals[i] = val;

            return;
        } else if ( map->keys[i] == key ) {
            map->vals[i] = val;

            return;
        }

        i++;
    }
}

internal_proc void
map_reset(Map *map) {
    for ( int i = 0; i < map->cap; ++i ) {
        if ( map->keys[i] ) {
            ((char **)map->keys)[i] = 0;
        }
    }
}

enum { ARENA_SIZE = 1024*1024, ARENA_ALIGNMENT = 8 };
struct Arena {
    char *base;
    char *ptr;
    size_t size;
    char **buckets;
};

internal_proc void
arena_init(Arena *arena, size_t size) {
    arena->base = (char *)os_mem_alloc(size);
    arena->ptr  = arena->base;
    arena->size = 0;
}

internal_proc void
arena_grow(Arena *arena, size_t size) {
    char *buf = (char *)os_mem_alloc(MAX(size, ARENA_SIZE));
    arena->base = buf;
    arena->ptr = buf;
    arena->size = 0;
    buf_push(arena->buckets, buf);
}

internal_proc void *
arena_alloc(Arena *arena, size_t size) {
    if (!arena->base || arena->ptr + size > arena->base + ARENA_SIZE) {
        arena_grow(arena, size);
    }

    void *result = (void *)arena->ptr;
    arena->ptr = (char *)ALIGN_UP_PTR(arena->ptr + size, ARENA_ALIGNMENT);

    return result;
}

internal_proc void
arena_free(Arena *arena) {
    for ( int i = 0; i < buf_len(arena->buckets); ++i ) {
        buf_free(arena->buckets[i]);
    }
    free(arena->base);
}

internal_proc void
arena_reset(Arena *arena) {
    for ( int i = 0; i < buf_len(arena->buckets); ++i ) {
        buf__hdr(arena->buckets[i])->len = 0;
    }
    arena->ptr = arena->base;
}

struct Intern {
    size_t   len;
    Intern*  next;
    char     str[1];
};

global_var Map interns;
global_var Arena intern_arena;

internal_proc char *
intern_str(char *start, char *end) {
    size_t len = end - start;
    uint64_t hash = bytes_hash(start, len);
    void *key = (void *)(uintptr_t)(hash ? hash : 1);

    Intern *intern = (Intern *)map_get(&interns, key);
    for (Intern *it = intern; it; it = it->next) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }

    Intern *new_intern = (Intern *)arena_alloc(&intern_arena, offsetof(Intern, str) + len + 1);

    new_intern->len = len;
    new_intern->next   = intern;
    memcpy(new_intern->str, start, len);
    new_intern->str[len] = 0;
    map_put(&interns, key, new_intern);

    return new_intern->str;
}

internal_proc char *
intern_str(char *value) {
    size_t len = utf8_str_size(value);
    return intern_str(value, value + len);
}

#define QUEUE_ALLOCATOR(name) void * name(size_t size)
typedef QUEUE_ALLOCATOR(Queue_Alloc);

QUEUE_ALLOCATOR(queue_alloc_default) {
    void *result = malloc(size);

    return result;
}

Queue_Alloc *queue_alloc = queue_alloc_default;

struct Queue_Entry {
    Queue_Entry * next;
    Queue_Entry * prev;
    void        * data;
};

struct Queue {
    Queue_Entry   root;
    Queue_Entry * curr;
    size_t        num_elems;
};

void *
queue_entry(Queue *q, size_t index) {
    if ( index >= q->num_elems ) {
        return NULL;
    }

    Queue_Entry *elem = q->root.next;
    for ( int i = 0; i < index; ++i ) {
        elem = elem->next;
    }

    return elem->data;
}

void
queue_push(Queue *q, void *data) {
    Queue_Entry *entry = (Queue_Entry *)queue_alloc(sizeof(Queue_Entry));

    if ( !q->curr ) {
        q->curr = &q->root;
    }

    entry->data = data;
    entry->next = NULL;
    entry->prev = q->curr;

    q->curr->next = entry;
    q->curr = entry;
    q->num_elems++;
}

void *
queue_pop(Queue *q) {
    void *result = q->curr->data;

    if ( q->curr != &q->root) {
        q->curr = q->curr->prev;
    }

    q->num_elems--;

    return result;
}

void *
queue_shift(Queue *q) {
    void *result = 0;

    if ( !q->root.next ) {
        return result;
    }

    result = q->root.next->data;
    q->root.next = q->root.next->next;

    if ( q->root.next ) {
        q->root.next->prev = &q->root;
    }

    q->num_elems--;

    return result;
}

void
queue_unshift(Queue *q, void *data) {
    Queue_Entry *entry = (Queue_Entry *)queue_alloc(sizeof(Queue_Entry));

    entry->data = data;
    entry->next = q->root.next;
    entry->prev = &q->root;

    q->root.next = entry;
    q->num_elems++;
}

