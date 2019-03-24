#define internal_proc static
#define global_var    static

typedef int32_t b32;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define CLAMP_MAX(x, max) MIN(x, max)
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

internal_proc void *
xmalloc(size_t size) {
    void *mem = malloc(size);

    if ( !mem ) {
        assert(!"speicher konnte nicht reserviert werden!");
        exit(1);
    }

    return mem;
}

internal_proc void *
xcalloc(size_t num, size_t size) {
    void *mem = calloc(num, size);

    if ( !mem ) {
        assert(!"speicher konnte nicht reserviert werden!");
        exit(1);
    }

    return mem;
}

internal_proc void *
xrealloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        assert(!"xrealloc failed");
        exit(1);
    }
    return ptr;
}

internal_proc void *
memdup(void *mem, size_t size) {
    void *result = xmalloc(size);

    memcpy(result, mem, size);

    return result;
}

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[1];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : (*((void **)&(b)) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

internal_proc void *
buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = CLAMP_MIN(2*buf_cap(buf), MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = (BufHdr *)xrealloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = (BufHdr *)xmalloc(new_size);
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
        assert(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}

struct Intern {
    s64   len;
    char *str;
};

global_var Intern *interns;
internal_proc char *
intern_str(char *start, char *end) {
    s64 len = end - start;
    for ( int i = 0; i < buf_len(interns); ++i ) {
        Intern intern = interns[i];
        if ( intern.len == len && strncmp(intern.str, start, len) == 0 ) {
            return intern.str;
        }
    }

    Intern new_intern = {};
    new_intern.len = len;
    new_intern.str = (char *)malloc(sizeof(char)*len + 1);
    memcpy(new_intern.str, start, sizeof(char)*len);
    new_intern.str[len] = 0;
    buf_push(interns, new_intern);

    return new_intern.str;
}

internal_proc char *
intern_str(char *value) {
    size_t len = strlen(value);
    return intern_str(value, value + len);
}

