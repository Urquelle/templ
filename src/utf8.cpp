struct Utf8_Char {
    size_t size;
    char  *bytes;
};

internal_proc size_t
utf8_char_size(char *str) {
    if ( (*str & 0x80) == 0 ) {
        return 1;
    } else if ( (*str & 0xE0) == 0xc0 ) {
        return 2;
    } else if ( (*str & 0xF0) == 0xE0 ) {
        return 3;
    } else if ( (*str & 0xF0) == 0xF0 ) {
        return 4;
    } else {
        illegal_path();
    }

    return 0;
}

internal_proc Utf8_Char
utf8_char(size_t size, char *ptr) {
    Utf8_Char result = {};

    result.size  = size;
    result.bytes = (char *)xcalloc(1, size);

    for ( int i = 0; i < size; ++i ) {
        result.bytes[i] = *(ptr+i);
    }

    return result;
}

internal_proc Utf8_Char
utf8_char(char *input) {
    size_t size = utf8_char_size(input);
    Utf8_Char result = utf8_char(size, input);

    return result;
}

internal_proc char *
utf8_char_end(char *input) {
    char *result = input + utf8_char_size(input) - 1;

    return result;
}

internal_proc wchar_t
utf8_char_to_wchar(Utf8_Char c) {
    wchar_t *result = (wchar_t *)xmalloc(sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, c.bytes, (int)c.size, result, 1);

    return *result;
}
