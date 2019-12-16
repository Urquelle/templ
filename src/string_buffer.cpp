enum { STRING_BUFFER_CAP = 128 };
struct String_Buffer {
    size_t cap;
    size_t len;
    size_t size;
    char *buf;
};

internal_proc String_Buffer *
sb_push(String_Buffer *sb, char *str, Arena *arena) {
    String_Buffer *result = sb;

    if ( !result ) {
        result = ALLOC_STRUCT(arena, String_Buffer);
        result->cap = STRING_BUFFER_CAP;
        result->len = 0;
        result->size = 0;
        result->buf = (char *)ALLOC_SIZE(arena, STRING_BUFFER_CAP);
    }

    size_t len  = utf8_str_len(str);
    size_t size = utf8_str_size(str);
    if ( result->cap < (result->size + size) ) {
        size_t new_cap = MAX(result->size + size, result->cap*2);
        char *buf = (char *)ALLOC_SIZE(arena, new_cap);
        memcpy(buf, result->buf, result->size);
        result->buf = buf;
        result->cap = new_cap;
    }

    for ( int i = 0; i < len; ++i ) {
        size_t char_size = utf8_char_size(str);

        for ( int j = 0; j < char_size; ++j ) {
            result->buf[result->size++] = str[j];
        }

        str += char_size;
        result->len++;
    }

    return result;
}
