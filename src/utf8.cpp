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

internal_proc size_t
utf8_strlen(char *str) {
    size_t result = 0;
    char *ptr = str;

    while ( *ptr ) {
        size_t size = utf8_char_size(ptr);
        ptr += size;
        result++;
    }

    return result;
}

internal_proc size_t
utf8_str_size(char *str, size_t len) {
    size_t result = 0;
    char *ptr = str;

    for ( int i = 0; i < len; ++i ) {
        result += utf8_char_size(ptr+result);
    }

    return result;
}

internal_proc size_t
utf8_str_size(char *str) {
    size_t len = utf8_strlen(str);
    size_t result = utf8_str_size(str, len);

    return result;
}

internal_proc void
utf8_char_write(char *dest, char *c) {
    size_t num_bytes = utf8_char_size(c);

    for ( int i = 0; i < num_bytes; ++i ) {
        dest[i] = c[i];
    }
}

internal_proc char *
utf8_char_next(char *input) {
    char *result = input + utf8_char_size(input);

    return result;
}

internal_proc size_t
utf8_char_offset(char *ptr, char *c) {
    size_t result = c - ptr;

    return result;
}

internal_proc char *
utf8_char_lastbyte(char *input) {
    char *result = input + utf8_char_size(input) - 1;

    return result;
}

internal_proc char *
utf8_char_goto(char *input, size_t count) {
    size_t len = utf8_strlen(input);
    if ( len < count ) {
        return input;
    }

    char *result = input;
    for ( int i = 0; i < count; ++i ) {
        result += utf8_char_size(result);
    }

    return result;
}

internal_proc char *
utf8_toupper(char *str) {
    /* @INFO: tabelle mit werten für die zeichen https://unicode-table.com/ */

    size_t size = utf8_char_size(str);
    char *result = str;

    u8 c0 = (u8)str[0];
    u8 c1 = ( size > 1 ) ? (u8)str[1] : 0;

    erstes_if ( size == 1 && *str >= 'a' && *str <= 'z') {
        result = (char *)malloc(sizeof(char));
        memcpy(result, str, sizeof(char));
        result[0] -= 0x20;

    } else if ( size == 2 ) {
        /* @INFO: umlaute */
        erstes_if ( c0 == 0xc3 ) {
            if ( c1 == 0x9f ) {
                result = (char *)malloc(sizeof(char)*4);
                memcpy(result, str, sizeof(char)*2);

                result[0] = (u8)0xe1;
                result[1] = (u8)0xba;
                result[2] = (u8)0x9e;
                result[3] = 0;
            } else if ( c1 >= 0xa0 && c1 <= 0xbf) {
                result = (char *)malloc(sizeof(char)*2);
                memcpy(result, str, sizeof(char)*2);

                result[1] -= 0x20;
            }

        /* @INFO: рстуфхцчшщъыьэюя */
        } else if ( c0 == 0xd0 && c1 >= 0xb0 ) {
            result = (char *)malloc(sizeof(char)*2);
            memcpy(result, str, sizeof(char)*2);

            result[1] -= 0x20;

        /* @INFO: абвгдеёжзийклмноп */
        } else if ( c0 == 0xd1 ) {
            result = (char *)malloc(sizeof(char)*3);
            memcpy(result, str, sizeof(char)*2);

            /* @INFO: абвгдежзийклмноп */
            if ( c1 <= 0x8f ) {
                result[0] -= 0x01;
                result[1] += 0x20;

            /* @INFO: ё */
            } else if ( c1 == 0x91 ) {
                result[0] -= 0x01;
                result[1] -= 0x10;
            }

            result[2] = 0;
        }

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 3 ) {
        /* @AUFGABE: implementieren */

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 4 ) {
        /* @AUFGABE: implementieren */
    }

    return result;
}

internal_proc char *
utf8_tolower(char *str) {
    /* @INFO: tabelle mit werten für die zeichen https://unicode-table.com/ */

    size_t size = utf8_char_size(str);
    char *result = str;

    u8 c0 = (u8)str[0];
    u8 c1 = ( size > 1 ) ? (u8)str[1] : 0;
    u8 c2 = ( size > 2 ) ? (u8)str[2] : 0;

    /* @INFO: ascii */
    erstes_if ( size == 1 && *str >= 'A' && *str <= 'Z') {
        result = (char *)malloc(sizeof(char)*2);
        memcpy(result, str, sizeof(char));
        result[0] += 0x20;
        result[1] = 0;
    } else if ( size == 2 ) {
        /* @INFO: umlaute */
        erstes_if ( c0 == 0xc3 ) {
            if ( c1 >= 0x80 && c1 <= 0x9e) {
                result = (char *)malloc(sizeof(char)*3);
                memcpy(result, str, sizeof(char)*2);

                result[1] += 0x20;
                result[2] = 0;
            }

        /* @INFO: АБВГДЕЖЗИЙКЛМНОП */
        } else if ( c0 == 0xd0 && c1 >= 0x90 && c1 <= 0x9f) {
            result = (char *)malloc(sizeof(char)*3);
            memcpy(result, str, sizeof(char)*2);

            result[1] += 0x20;
            result[2] = 0;

        /* @INFO: РСТУФХЦЧШЩЪЫЬЭЮЯ */
        } else if ( c0 == 0xd0 && c1 >= 0xa0 && c1 <= 0xaf) {
            result = (char *)malloc(sizeof(char)*3);
            memcpy(result, str, sizeof(char)*2);

            result[0] += 0x01;
            result[1] -= 0x20;
            result[2] = 0;

        /* @INFO: Ё */
        } else if ( c0 == 0xd0 && c1 == 0x81 ) {
            result = (char *)malloc(sizeof(char)*3);
            memcpy(result, str, sizeof(char)*2);

            result[0] += 0x01;
            result[1] += 0x10;
            result[2] = 0;
        }
    } else if ( size == 3 ) {

        /* @INFO: ẞ */
        if ( c0 == 0xe1 && c1 == 0xba && c2 == 0x9e ) {
            result = (char *)malloc(sizeof(char)*3);
            result[0] = (u8)0xc3;
            result[1] = (u8)0x9f;
            result[2] = 0;
        } else {
            /* @AUFGABE: implementieren */
        }
    } else if ( size == 4 ) {
        /* @AUFGABE: implementieren */
    }

    return result;
}

