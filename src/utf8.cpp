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
    size_t size = utf8_char_size(str);

    /* @AUFGABE: bei der konvertierung die anzahl bytes ermitteln und
     *           für das ergebnis reservieren.
     */
    erstes_if ( size == 1 && *str >= 'a' && *str <= 'z') {
        str[0] -= 0x20;

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 2 ) {
        /* @INFO: umlaute */
        erstes_if ( (u8)str[0] == 0xc3 ) {
            if ( (u8)str[1] == 0x9f ) {
                /* @ACHTUNG: ß umwandeln. von 0xc39f nach 0xe1ba9e. aus 2 bytes,
                 *           werden 3 bytes 😭
                 */
            } else {
                str[1] -= 0x20;
            }

        /* @INFO: kyrillisch teil 1 */
        } else if ( (u8)str[0] == 0xd0 && (u8)str[1] >= 0xb0 ) {
            str[1] -= 0x20;

        /* @INFO: kyrillisch teil 2 */
        } else if ( (u8)str[0] == 0xd1 ) {
            /* @INFO: абвгдежзийклмноп */
            if ( (u8)str[1] <= 0x8f ) {
                str[0] -= 0x01;
                str[1] += 0x20;

            /* @INFO: ё */
            } else if ( (u8)str[1] == 0x91 ) {
                str[0] -= 0x01;
                str[1] -= 0x10;
            }
        }

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 3 ) {
        /* @AUFGABE: implementieren */

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 4 ) {
        /* @AUFGABE: implementieren */
    }

    return str;
}

