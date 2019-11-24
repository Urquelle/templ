internal_proc PROC_CALLBACK(filter_abs) {
    assert(operand->kind == VAL_INT);
    s32 i = abs(val_int(operand));

    return val_int(i);
}

internal_proc PROC_CALLBACK(filter_attr) {
    Scope *scope = (Scope *)operand->ptr;
    Scope *prev_scope = scope_set(scope);

    Sym *sym = sym_get(val_str(narg("name")->val));
    Val *result = sym->val;

    scope_set(prev_scope);

    return result;
}

internal_proc PROC_CALLBACK(filter_batch) {
    return val_str("/* @IMPLEMENT: filter batch */");
}

internal_proc PROC_CALLBACK(filter_capitalize) {
    assert(operand->kind == VAL_STR);

    char *cap = utf8_toupper((char *)operand->ptr);
    size_t cap_size = utf8_char_size(cap);
    char *remainder = (char *)operand->ptr + cap_size;

    char *result = strf("%.*s%s", cap_size, cap, remainder);

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_center) {
    int width = val_int(narg("width")->val);

    if ( operand->len >= width ) {
        return operand;
    }

    s32 padding = (s32)((width - operand->len) / 2);

    char *result = strf("%*s%s%*s", padding, "", val_print(operand), padding, "");

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_default) {
    char *default_value = val_str(narg("s")->val);
    b32 boolean = val_bool(narg("boolean")->val);

    if ( val_is_undefined(operand) || boolean && !val_is_true(operand) ) {
        return val_str(default_value);
    }

    return operand;
}

internal_proc void
quicksort(Sym **left, Sym **right, b32 case_sensitive, char *by, b32 reverse) {
    char *key = intern_str("key");

#define compneq(left, right) ((by == key) ? (utf8_strcmp((left)->name, (right)->name) != 0) : (*((left)->val) != *((right)->val)))
#define compgt(left, right)  ((by == key) ? (utf8_strcmp((left)->name, (right)->name)  > 0) : (*(b32 *)(*(left)->val > *(right)->val).ptr))
#define complt(left, right)  ((by == key) ? (utf8_strcmp((left)->name, (right)->name)  < 0) : (*(b32 *)(*(left)->val < *(right)->val).ptr))

    if ( compneq(*left, *right) ) {
        Sym **ptr0 = left;
        Sym **ptr1 = left;
        Sym **ptr2 = left;

        Sym *pivot = *left;

        do {
            ptr2 = ptr2 + 1;
            b32 check = ( reverse ) ? ( compgt(*ptr2, pivot) ) : ( complt(*ptr2, pivot) );

            if ( check ) {
                ptr0 = ptr1;
                ptr1 = ptr1 + 1;

                Sym *temp = *ptr1;
                *ptr1 = *ptr2;
                *ptr2 = temp;
            }
        } while ( compneq(*ptr2, *right) );

        Sym *temp = *left;
        *left = *ptr1;
        *ptr1 = temp;

        if ( compneq(*ptr1, *right) ) {
            ptr1 = ptr1 + 1;
        }

        quicksort(left, ptr0, case_sensitive, by, reverse);
        quicksort(ptr1, right, case_sensitive, by, reverse);
    }

#undef compneq
#undef compgt
#undef complt
}

internal_proc PROC_CALLBACK(filter_dictsort) {
    b32 case_sensitive = val_bool(narg("case_sensitive")->val);
    char *by = val_str(narg("by")->val);
    b32 reverse = val_bool(narg("reverse")->val);

    Sym **syms = 0;
    {
        Scope *scope = (Scope *)operand->ptr;
        for ( int i = 0; i < scope->num_syms; ++i ) {
            buf_push(syms, scope->sym_list[i]);
        }

        quicksort(syms, syms + buf_len(syms)-1, case_sensitive, by, reverse);
    }

    Scope *scope = scope_new(0, "dictsort_dict");
    Scope *prev_scope = scope_set(scope);
    for ( int i = 0; i < buf_len(syms); ++i ) {
        Sym *sym = syms[i];
        sym_push_var(sym->name, sym->type, sym->val);
    }
    scope_set(prev_scope);

    return val_dict(scope);
}

internal_proc PROC_CALLBACK(filter_escape) {
    char *result = "";
    char *ptr = (char *)operand->ptr;

    for ( int i = 0; i < operand->len; ++i ) {
        erstes_if ( *ptr == '<' ) {
            result = strf("%s&lt;", result);
        } else if ( *ptr == '>' ) {
            result = strf("%s&gt;", result);
        } else if ( *ptr == '&' ) {
            result = strf("%s&amp;", result);
        } else if ( *ptr == '\'' ) {
            result = strf("%s&#39;", result);
        } else if ( *ptr == '"' ) {
            result = strf("%s&#34;", result);
        } else {
            result = strf("%s%.*s", result, utf8_char_size(ptr), ptr);
        }

        ptr += utf8_char_size(ptr);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_filesizeformat) {
    char *suff_bin[] = {
        { "Bytes" }, { "kiB" }, { "MiB" }, { "GiB" }, { "TiB" }, { "PiB" },
        { "EiB" }, { "ZiB" }, { "YiB" }
    };

    char *suff_dec[] = {
        { "Bytes" }, { "kB" }, { "MB" }, { "GB" }, { "TB" }, { "PB" },
        { "EB" }, { "ZB" }, { "YB" }
    };

    b32 binary = val_bool(narg("binary")->val);
    int c = (binary) ? 1024 : 1000;
    char **suff = (binary) ? suff_bin : suff_dec;

    s64 size = val_int(operand);
    s32 mag = (s32)(log(size)/log(c));
    f32 calc_size = ( mag ) ? ((f32)size/(mag*c)) : (f32)size;

    char *result = strf("%.3f %s", calc_size, suff[mag]);

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_first) {
    Val *result = val_elem(operand, 0);

    return result;
}

internal_proc PROC_CALLBACK(filter_float) {
    char *end = 0;
    f32 f = strtof(val_str(operand), &end);
    Val *result = (end == operand->ptr) ? narg("default")->val : val_float(f);

    return result;
}

internal_proc PROC_CALLBACK(filter_format) {
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

                    result = strf("%s%s", result, val_print(arg->val));
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

                    result = strf("%s%s", result, val_print(arg->val));
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

internal_proc PROC_CALLBACK(filter_int) {
    char *end = 0;
    s32 i = strtol(val_str(operand), &end, val_int(narg("base")->val));
    Val *result = (end == operand->ptr) ? narg("default")->val : val_int(i);

    return result;
}

internal_proc PROC_CALLBACK(filter_join) {
    char *sep = val_str(narg("d")->val);
    Val *attr = narg("attribute")->val;
    char *result = "";

    if (operand->len < 2) {
        return operand;
    }

    if ( attr->kind == VAL_NONE ) {
        result = strf("%s", val_print(val_elem(operand, 0)));
        for ( int i = 1; i < operand->len; ++i ) {
            result = strf("%s%s%s", result, sep, val_print(val_elem(operand, i)));
        }
    } else {
        Val *v = val_elem(operand, 0);
        Scope *scope = (Scope *)v->ptr;
        Scope *prev_scope = scope_set(scope);

        Sym *sym = sym_get(val_str(attr));
        result = strf("%s", val_print(sym->val));

        for ( int i = 1; i < operand->len; ++i ) {
            v = val_elem(operand, i);
            scope_set((Scope *)v->ptr);
            sym = sym_get(val_str(attr));

            result = strf("%s%s%s", result, sep, val_print(sym->val));
        }

        scope_set(prev_scope);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_last) {
    Val *result = val_elem(operand, (int)operand->len-1);

    return result;
}

internal_proc PROC_CALLBACK(filter_length) {
    Val *result = val_int((int)operand->len);

    return result;
}

internal_proc PROC_CALLBACK(filter_lower) {
    assert(operand->kind == VAL_STR);
    char *str = val_str(operand);
    char *result = "";

    size_t offset = 0;
    for ( int i = 0; i < utf8_strlen(str); ++i ) {
        size_t old_len = utf8_char_size(str + offset);
        char *c = utf8_tolower(str + offset);
        size_t len = utf8_char_size(c);

        result = strf("%s%.*s", result, len, c);
        offset += old_len;
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_truncate) {
    size_t len = MIN(operand->len, val_int(narg("length")->val));
    s32 leeway = val_int(narg("leeway")->val);

    u64 diff = abs((s64)(len - operand->len));
    if ( diff <= leeway ) {
        return operand;
    }

    char *result = "";
    char *ptr = val_str(operand);

    b32 killwords = val_bool(narg("killwords")->val);
    if ( !killwords ) {
        char *ptrend = utf8_char_goto(ptr, operand->len-1);
        u32 char_count = 0;
        for ( size_t i = operand->len-1; i >= len; --i ) {
            if ( *ptrend == ' ' ) {
                char_count = 0;
            } else {
                char_count += 1;
            }
            ptrend = utf8_char_goto(ptr, i-1);
        }

        if ( char_count && ptrend != ptr ) {
            size_t pos = len;
            while ( ptrend != ptr && *ptrend != ' ' ) {
                ptrend = utf8_char_goto(ptr, --pos);
            }
            len = pos;
        }
    }

    for ( int i = 0; i < len; ++i ) {
        result = strf("%s%.*s", result, utf8_char_size(ptr), ptr);
        ptr += utf8_char_size(ptr);
    }

    char *end = val_str(narg("end")->val);
    size_t end_len = utf8_strlen(end);
    for ( int i = 0; i < end_len; ++i ) {
        result = strf("%s%.*s", result, utf8_char_size(end), end);
        end += utf8_char_size(end);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_upper) {
    assert(operand->kind == VAL_STR);
    char *str = val_str(operand);
    char *result = "";

    size_t offset = 0;
    for ( int i = 0; i < utf8_strlen(str); ++i ) {
        size_t old_len = utf8_char_size(str + offset);
        char *c = utf8_toupper(str + offset);
        size_t len = utf8_char_size(c);

        result = strf("%s%.*s", result, len, c);
        offset += old_len;
    }

    return val_str(result);
}

