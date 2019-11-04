internal_proc FILTER_CALLBACK(filter_abs) {
    assert(operand->kind == VAL_INT);
    s32 i = abs(val_int(operand));

    return val_int(i);
}

internal_proc FILTER_CALLBACK(filter_capitalize) {
    assert(operand->kind == VAL_STR);

    char first_letter = ((char *)operand->ptr)[0];
    ((char *)operand->ptr)[0] = std::toupper(first_letter, std::locale());

    return operand;
}

internal_proc FILTER_CALLBACK(filter_default) {
    assert(operand->kind == VAL_STR);

    Resolved_Arg *arg = (Resolved_Arg *)map_get(nargs, intern_str("s"));
    if ( !operand->len ) {
        return arg->val;
    }

    return operand;
}

internal_proc FILTER_CALLBACK(filter_upper) {
    assert(operand->kind == VAL_STR);
    char *str = val_str(operand);
    char *result = "";

    size_t offset = 0;
    for ( int i = 0; i < utf8_strlen(str); ++i ) {
        char *c = utf8_toupper(str + offset);
        size_t len = utf8_char_size(c);

        result = strf("%s%.*s", result, len, c);
        offset += len;
    }

    return val_str(result);
}

internal_proc FILTER_CALLBACK(filter_escape) {
    char *result = "";

    for ( int i = 0; i < operand->len; ++i ) {
        char c = ((char *)operand->ptr)[i];

        erstes_if ( c == '<' ) {
            result = strf("%s%s", result, "&lt;");
        } else if ( c == '>' ) {
            result = strf("%s%s", result, "&gt;");
        } else if ( c == '&' ) {
            result = strf("%s%s", result, "&amp;");
        } else if ( c == ' ' ) {
            result = strf("%s%s", result, "&nbsp;");
        } else {
            result = strf("%s%c", result, c);
        }
    }

    return val_str(result);
}

internal_proc FILTER_CALLBACK(filter_format) {
    assert(operand->kind == VAL_STR);

    char *format = val_str(operand);
    int num_arg = 0;
    char *result = "";

    while ( format[0] ) {
        if ( format[0] == '%' ) {
            int size = 6;
            format = utf8_char_next(format);

            if ( format[0] == '.' ) {
                size = 0;
                format = utf8_char_next(format);

                while ( is_numeric(format[0]) ) {
                    size *= 10;
                    size += format[0] - '0';

                    format = utf8_char_next(format);
                }
            }

            if ( format[0] == 's' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg->val->kind != VAL_STR ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss string sein", arg->name);
                    }

                    result = strf("%s%s", result, val_to_char(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'd' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg->val->kind != VAL_INT ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss int sein", arg->name);
                    }

                    result = strf("%s%s", result, val_to_char(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'f' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg->val->kind != VAL_FLOAT ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss float sein", arg->name);
                    }

                    result = strf("%s%.*f", result, size, val_float(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else {
                illegal_path();
            }
        } else {
            result = strf("%s%.*s", result, utf8_char_size(format), format);
        }

        format = utf8_char_next(format);
    }

    return val_str(result);
}

internal_proc FILTER_CALLBACK(filter_truncate) {
    assert(operand->kind == VAL_STR);

    return val_str("<!-- @AUFGABE: truncate implementieren -->");
}

