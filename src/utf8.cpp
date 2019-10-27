struct Utf8_Char {
    size_t size;
    char  *bytes;
};

internal_proc size_t
utf8_char_size(char *str) {
    erstes_if ( (*str & 0x80) == 0x00 ) {
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
utf8_char_next(char *input) {
    char *result = input + utf8_char_size(input);

    return result;
}

internal_proc char *
utf8_char_end(char *input) {
    char *result = input + utf8_char_size(input) - 1;

    return result;
}

internal_proc wchar_t
utf8_char_to_wchar(Utf8_Char c) {
    wchar_t result = os_utf8_char_to_wchar(c.bytes, c.size);

    return result;
}

internal_proc wchar_t
utf8_char_to_wchar(char *c) {
    Utf8_Char uc = utf8_char(c);
    wchar_t result = utf8_char_to_wchar(uc);

    return result;
}

internal_proc char *
utf8_char_to_uppercase(char *c) {
    Utf8_Char utf8_c = utf8_char(c);
    wchar_t wchar_lowercase = utf8_char_to_wchar(utf8_c);
    wchar_t wchar_uppercase = std::toupper(wchar_lowercase, std::locale());

    char *result = (char *)xcalloc(1, sizeof(char)*6);
    os_utf8_wchar_to_char(wchar_uppercase, result, 6*sizeof(char));

    return result;
}
