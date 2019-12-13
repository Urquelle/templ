internal_proc char *
utf8_str_escape(char *ptr) {
    char *result = "";
    size_t len = utf8_str_len(ptr);

    for ( int i = 0; i < len; ++i ) {
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

    return result;
}
/* scope {{{ */
struct Scope {
    char*  name;
    Scope* parent;
    Map    syms;
    Sym  **sym_list;
    size_t num_syms;
};

global_var Scope filter_scope;
global_var Scope global_scope;
global_var Scope system_scope;
global_var Scope tester_scope;

global_var Scope type_any_scope;
global_var Scope type_sequence_scope;
global_var Scope type_numeric_scope;
global_var Scope type_bool_scope;
global_var Scope type_dict_scope;
global_var Scope type_float_scope;
global_var Scope type_int_scope;
global_var Scope type_range_scope;
global_var Scope type_list_scope;
global_var Scope type_string_scope;

global_var Scope *current_scope = &global_scope;

internal_proc Scope *
scope_new(Scope *parent, char *name = NULL) {
    Scope *result = ALLOC_STRUCT(&resolve_arena, Scope);

    result->sym_list = 0;
    result->name   = name;
    result->parent = parent;
    result->syms   = {};

    return result;
}

internal_proc Scope *
scope_enter(char *name = NULL) {
    Scope *scope = scope_new(current_scope, name);
    current_scope = scope;

    return scope;
}

internal_proc void
scope_leave() {
    assert(current_scope->parent);
    current_scope = current_scope->parent;
}

internal_proc Scope *
scope_set(Scope *scope) {
    Scope *result = current_scope;

    if ( scope ) {
        current_scope = scope;
    }

    return result;
}

internal_proc Sym *
scope_attr(Scope *scope, char *attribute) {
    Scope *prev_scope = scope_set(scope);
    Sym *result = sym_get(attribute);
    scope_set(prev_scope);

    return result;
}

internal_proc size_t
scope_num_elems(Scope *scope) {
    size_t result = scope->num_syms;

    return result;
}

internal_proc Sym *
scope_elem(Scope *scope, size_t idx) {
    Sym *result = scope->sym_list[idx];

    return result;
}
/* }}} */
/* val {{{ */
enum Val_Kind {
    VAL_UNDEFINED,
    VAL_NONE,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_CHAR,
    VAL_PAIR,
    VAL_STR,
    VAL_ITERABLE_START = VAL_STR,
    VAL_RANGE,
    VAL_TUPLE,
    VAL_LIST,
    VAL_DICT,
    VAL_ITERABLE_END = VAL_DICT,
    VAL_PROC,
};

struct Val_Proc {
    Type_Field **params;
    size_t num_params;
    Type *ret;
    Proc_Callback *callback;
    Resolved_Stmt **stmts;
    size_t num_stmts;
};

struct Val {
    Val_Kind kind;
    size_t size;
    size_t len;
    void  *ptr;
    void *user_data;
    Scope *scope;
    b32 safe;
};

internal_proc Val *
val_new(Val_Kind kind, size_t size) {
    Val *result = ALLOC_STRUCT(&resolve_arena, Val);

    result->kind = kind;
    result->size = size;
    result->len  = 1;
    result->ptr  = (void *)ALLOC_SIZE(&resolve_arena, size);
    result->user_data = 0;
    result->scope = 0;

    return result;
}

internal_proc Val *
val_none() {
    Val *result = val_new(VAL_NONE, 0);

    result->size = 0;
    result->len  = 0;
    result->ptr  = 0;

    return result;
}

internal_proc Val *
val_undefined() {
    Val *result = val_new(VAL_UNDEFINED, 0);

    result->size = 0;
    result->len  = 0;
    result->ptr  = 0;

    return result;
}

internal_proc Val *
val_safe(Val *val) {
    val->safe = true;

    return val;
}

internal_proc Val *
val_unsafe(Val *val) {
    val->safe = false;

    return val;
}

internal_proc Val *
val_copy(Val *val) {
    if ( !val ) {
        return 0;
    }

    if ( val->kind == VAL_UNDEFINED ) {
        return val;
    }

    Val *result = val_new(val->kind, val->size);

    result->len = val->len;
    result->size = val->size;
    result->scope = val->scope;
    result->user_data = val->user_data;

    if ( val->kind == VAL_STR ) {
        result->ptr = val->ptr;
    } else if ( val->kind == VAL_BOOL ) {
        *(b32 *)result->ptr = *(b32 *)val->ptr;
    } else {
        memcpy(result->ptr, val->ptr, val->size);
    }

    return result;
}

internal_proc Val *
val_bool(b32 val) {
    Val *result = val_new(VAL_BOOL, sizeof(b32));

    *((b32 *)result->ptr) = val;
    result->scope = &type_bool_scope;

    return result;
}

internal_proc b32
val_bool(Val *val) {
    return *(b32 *)val->ptr;
}

internal_proc Val *
val_neg(Val *val) {
    assert(val->kind == VAL_BOOL);

    Val *result = val_new(VAL_BOOL, sizeof(s32));

    *((b32 *)result->ptr) = !val_bool(val);

    return result;
}

internal_proc char
val_char(Val *val) {
    return *(char *)val->ptr;
}

internal_proc Val *
val_int(int val) {
    Val *result = val_new(VAL_INT, sizeof(int));

    *((int *)result->ptr) = val;
    result->scope = &type_int_scope;

    return result;
}

internal_proc int
val_int(Val *val) {
    return *(int *)val->ptr;
}

internal_proc Val *
val_float(float val) {
    Val *result = val_new(VAL_FLOAT, sizeof(float));

    *((float *)result->ptr) = val;
    result->scope = &type_float_scope;

    return result;
}

internal_proc float
val_float(Val *val) {
    return *(float *)val->ptr;
}

internal_proc Val *
val_str(char *val, size_t len = 0) {
    Val *result = val_new(VAL_STR, sizeof(char*));

    result->len = (len) ? len : utf8_str_len(val);
    result->ptr = intern_str(val);
    result->scope = &type_string_scope;

    return result;
}

internal_proc Val *
val_char(Val *val, int idx) {
    Val *result = val_new(VAL_CHAR, sizeof(char *));

    result->len = idx;
    result->ptr = val;

    return result;
}

internal_proc char *
val_str(Val *val) {
    return (char *)val->ptr;
}

struct Resolved_Pair {
    Val *key;
    Val *value;
};

internal_proc Resolved_Pair *
resolved_pair(Val *key, Val *value) {
    Resolved_Pair *result = ALLOC_STRUCT(&resolve_arena, Resolved_Pair);

    result->key = key;
    result->value = value;

    return result;
}

internal_proc Val *
val_pair(Val *key, Val *value) {
    Val *result = val_new(VAL_PAIR, sizeof(Val *)*2);

    result->len = 1;
    result->ptr = resolved_pair(key, value);

    return result;
}

internal_proc Val *
val_range(int min, int max, int step = 1) {
    Val *result = val_new(VAL_RANGE, sizeof(int)*3);

    result->len = (max - min) + 1;
    *((int *)result->ptr)   = min;
    *((int *)result->ptr+1) = max;
    *((int *)result->ptr+2) = step;
    result->scope = &type_range_scope;

    return result;
}

internal_proc void
val_range0(Val *val, s32 value) {
    *((s32 *)val->ptr) = value;
}

internal_proc void
val_range1(Val *val, s32 value) {
    *((s32 *)val->ptr+1) = value;
}

internal_proc int
val_range0(Val *val) {
    return *((int *)val->ptr+0);
}

internal_proc int
val_range1(Val *val) {
    return *((int *)val->ptr+1);
}

internal_proc int
val_range2(Val *val) {
    return *((int *)val->ptr+2);
}

internal_proc Val *
val_tuple(Val **vals, size_t num_vals) {
    Val *result = val_new(VAL_TUPLE, sizeof(Val)*num_vals);

    result->ptr = (Val **)AST_DUP(vals);
    result->len = num_vals;

    return result;
}

internal_proc Val *
val_list(Val **vals, size_t num_vals) {
    Val *result = val_new(VAL_LIST, sizeof(Val *)*num_vals);

    result->ptr = (Val **)AST_DUP(vals);
    result->len = num_vals;
    result->scope = &type_list_scope;

    return result;
}

internal_proc Val *
val_dict(Scope *scope) {
    Val *result = val_new(VAL_DICT, sizeof(Map));

    result->len   = (scope) ? scope->num_syms : 0;
    result->scope = scope;

    if ( scope ) {
        result->scope->parent = &type_dict_scope;
    }

    return result;
}

internal_proc Val *
val_proc(Type_Field **params, size_t num_params, Type *ret,
        Proc_Callback *callback)
{
    Val *result = val_new(VAL_PROC, sizeof(Type *));

    Val_Proc *ptr = ALLOC_STRUCT(&resolve_arena, Val_Proc);

    ptr->params = (Type_Field **)AST_DUP(params);
    ptr->num_params = num_params;
    ptr->ret = ret;
    ptr->callback = callback;

    result->ptr = ptr;
    result->scope = &type_any_scope;

    return result;
}

internal_proc void
val_set(Val *val, s32 value) {
    assert(val->kind == VAL_INT);
    *((s32 *)val->ptr) = value;
}

internal_proc void
val_set(Val *val, b32 value) {
    assert(val->kind == VAL_BOOL);
    *((b32 *)val->ptr) = value;
}

internal_proc void
val_set(Val *val, f32 value) {
    if ( val->kind == VAL_INT ) {
        val->kind = VAL_FLOAT;
        *((f32 *)val->ptr) = value;
    } else {
        assert(val->kind == VAL_FLOAT);
        *((f32 *)val->ptr) = value;
    }
}

internal_proc void
val_set(Val *val, char *value) {
    assert(val->kind == VAL_STR);
    *((char **)val->ptr) = value;
}

internal_proc void
val_set(Val *val, s32 min, s32 max) {
    *((s32 *)val->ptr+0) = min;
    *((s32 *)val->ptr+1) = max;
}

internal_proc void
val_set(Val *dest, Val *source, size_t index) {
    assert(dest->kind == VAL_LIST || dest->kind == VAL_TUPLE);

    *((Val **)dest->ptr + index) = source;
}

internal_proc void
val_set(Val *dest, Val *source) {
    erstes_if ( source->kind == VAL_STR && dest->kind == VAL_CHAR ) {
        Val *orig = (Val *)dest->ptr;

        char * old_char_loc  = utf8_char_goto((char *)orig->ptr, dest->len);
        size_t size_new_char = utf8_char_size((char *)source->ptr);
        size_t size_old_char = utf8_char_size(old_char_loc);
        size_t old_size      = utf8_str_size((char *)orig->ptr);
        size_t new_size      = old_size - size_old_char + size_new_char;

        if ( new_size == old_size && size_new_char == size_old_char ) {
            size_t offset = utf8_char_offset((char *)orig->ptr, old_char_loc);
            utf8_char_write((char *)orig->ptr + offset, (char *)source->ptr);
            dest->ptr = intern_str((char *)dest->ptr);
        } else {
            size_t len = utf8_str_len((char *)orig->ptr);
            char *new_mem = (char *)xcalloc(1, new_size+1);

            for ( int i = 0; i < dest->len; ++i ) {
                utf8_char_write(utf8_char_goto(new_mem, i), utf8_char_goto((char *)orig->ptr, i));
            }

            utf8_char_write(utf8_char_goto(new_mem, dest->len), (char *)source->ptr);

            for ( size_t i = dest->len+1; i < len; ++i ) {
                utf8_char_write(utf8_char_goto(new_mem, i), utf8_char_goto((char *)orig->ptr, i));
            }

            new_mem[new_size] = 0;
            orig->ptr = intern_str(new_mem);
        }
    } else {
        dest->kind  = source->kind;
        dest->size  = source->size;
        dest->len   = source->len;
        dest->scope = source->scope;
        dest->ptr   = source->ptr;
        dest->user_data = source->user_data;
    }
}

internal_proc void
val_inc(Val *val) {
    assert(val->kind == VAL_INT);

    val_set(val, val_int(val) + 1);
}

internal_proc void
val_dec(Val *val) {
    assert(val->kind == VAL_INT);

    val_set(val, val_int(val) - 1);
}

internal_proc Val *
val_op(Token_Kind op, Val *val) {
    if ( op == T_MINUS ) {
        if ( val->kind == VAL_INT ) {
            return val_int( val_int(val)*-1 );
        }
    }

    return val;
}

internal_proc char *
val_print(Val *val) {
    switch ( val->kind ) {
        case VAL_STR: {
            size_t len = utf8_str_size((char *)val->ptr, val->len);
            char *result = "";
            char *ptr = (char *)val->ptr;

            for ( int i = 0; i < len; ++i ) {
                if ( *(ptr + i) == '\\' ) {
                    continue;
                }

                result = strf("%s%c", result, *(ptr + i));
            }

            return result;
        } break;

        case VAL_INT: {
            char *result = strf("%d", val_int(val));
            return result;
        } break;

        case VAL_FLOAT: {
            char *result = strf("%f", val_float(val));
            return result;
        } break;

        case VAL_UNDEFINED: {
            return "<undefined>";
        } break;

        case VAL_BOOL: {
            erstes_if ( val_bool(val) == true ) {
                return "true";
            } else {
                return "false";
            }
        } break;

        case VAL_CHAR: {
            Val *orig = (Val *)val->ptr;
            char *ptr = utf8_char_goto((char *)orig->ptr, val->len);
            int size = (int)utf8_char_size(ptr);

            char *result = strf("%.*s", size, ptr);
            return result;
        } break;

        case VAL_PAIR: {
            Resolved_Pair *pair = (Resolved_Pair *)val->ptr;
            char *result = strf("%s", val_print(pair->value));

            return result;
        } break;

        case VAL_NONE: {
            return "<none>";
        } break;

        default: {
            fatal(0, 0, "datentyp kann nicht in zeichenkette umgewandelt werden.\n");
            return "";
        } break;
    }
}

internal_proc char *
val_pprint(Val *val, b32 verbose = false, s32 depth = 0) {
    char *result = "";

    if ( val->kind == VAL_DICT ) {
        result = strf("dict(\n");
        Scope *scope = val->scope;
        for ( int i = 0; i < scope_num_elems(scope); ++i ) {
            Sym *sym = scope_elem(scope, i);
            result = strf("%s%*s%s = %s\n", result, (depth+1)*4, "", sym_name(sym), val_pprint(sym_val(sym), verbose, depth+1));
        }
        result = strf("%s%*s)", result, depth*4, "");
    } else if ( val->kind == VAL_LIST ) {
        result = strf("%*slist(", depth*4, "");
        for ( int i = 0; i < val->len; ++i ) {
            Val *v = ((Val **)val->ptr)[i];
            result = strf("%s\n%*s%s", result, (depth+1)*4, "", val_pprint(v, verbose, depth+1));
        }
        result = strf("%s\n)", result);
    } else if ( val->kind == VAL_RANGE ) {
        result = strf("range(%d..%d)", val_range0(val), val_range1(val));
    } else if ( val->kind == VAL_TUPLE ) {
        result = strf("tuple(");
        for ( int i = 0; i < val->len; ++i ) {
            Val *v = ((Val **)val->ptr)[i];
            result = strf("%s%s\n", result, val_pprint(v, verbose, depth+1));
        }
        result = strf("%s)", result);
    } else if ( val->kind == VAL_PAIR ) {
        Resolved_Pair *pair = (Resolved_Pair *)val->ptr;
        result = strf("pair(%s = %s)", val_pprint(pair->key), val_pprint(pair->value));
    } else if ( val->kind == VAL_INT ) {
        result = strf("int(%d)", val_int(val));
    } else if ( val->kind == VAL_FLOAT ) {
        result = strf("float(%f)", val_float(val));
    } else if ( val->kind == VAL_BOOL ) {
        result = strf("bool(%s)", val_bool(val) ? "true" : "false");
    } else if ( val->kind == VAL_STR ) {
        result = strf("str(\"%s\")", val_str(val));
    } else if ( val->kind == VAL_NONE ) {
        result = strf("none()");
    } else if ( val->kind == VAL_PROC ) {
        Val_Proc *proc = (Val_Proc *)val->ptr;

        result = strf("proc(%s%s", (proc->ret) ? " -> " : "", type_pprint(proc->ret));
        for ( int i = 0; i < proc->num_params; ++i ) {
            Type_Field *param = proc->params[i];
            result = strf("%s\n%*sparam = %s(%s)", result,
                    (depth+1)*4, "",
                    type_field_name(param),
                    val_pprint(type_field_value(param), verbose, depth));
        }
        result = strf("%s\n%*s)", result, depth*4, "");
    } else if ( val->kind == VAL_UNDEFINED ) {
        result = strf("<undefined>");
    } else {
        result = strf("<unknown>");
    }

    return result;
}

internal_proc char *
val_tojson(Val *val, s32 indent, s32 depth = 0) {
    char *result = "";

    switch ( val->kind ) {
        case VAL_TUPLE:
        case VAL_LIST: {
            result = strf("%*s[", depth*indent, "");
            for ( int i = 0; i < val->len; ++i ) {
                Val *v = val_elem(val, i);
                result = strf("%s%s\n%s", result, (i > 0) ? "," : "", val_tojson(v, indent, depth + 1));
            }
            result = strf("%*s%s\n]", depth*indent, "", result);
        } break;

        case VAL_DICT: {
            result = strf("%*s{", depth*indent, "");
            for ( int i = 0; i < val->scope->num_syms; ++i ) {
                Sym *sym = val->scope->sym_list[i];
                result = strf("%s%s\n%*s\"%s\": %s", result, (i > 0) ? "," : "", (depth+1)*indent, "", sym_name(sym), val_tojson(sym_val(sym), indent, depth + 1));
            }
            result = strf("%s\n%*s}", result, depth*indent, "");
        } break;

        case VAL_STR: {
            result = strf("\"%s\"", utf8_str_escape(val_print(val)));
        } break;

        case VAL_UNDEFINED:
        case VAL_NONE: {
            result = "null";
        } break;

        default: {
            result = strf("%s", val_print(val));
        } break;
    }

    return result;
}

internal_proc Val *
val_subscript(Val *val, int idx) {
    switch ( val->kind ) {
        case VAL_RANGE: {
            return val_int(val_range0(val) + idx);
        } break;

        case VAL_LIST: {
            return *((Val **)val->ptr + idx);
        } break;

        case VAL_TUPLE: {
            return *((Val **)val->ptr + idx);
        } break;

        case VAL_STR: {
            return val_char(val, idx);
        } break;

        default: {
            return val;
        } break;
    }
}

internal_proc b32
val_is_undefined(Val *val) {
    b32 result = ( !val || val->kind == VAL_UNDEFINED );

    return result;
}

internal_proc b32
val_is_true(Val *val) {
    b32 result = val && ( val->kind == VAL_BOOL && val_bool(val) ||
                          val->kind == VAL_INT && val_int(val) != 0 );

    return result;
}

internal_proc Val *
operator*(Val left, Val right) {
    Val *result = 0;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(val_int(&left) * val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_float(val_float(&left) * val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_range(val_int(&left) * val_range0(&right), val_int(&left) * val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(val_float(&left) * val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range(val_range0(&left) * val_range0(&right), val_range1(&left) * val_range1(&right));
    } else if (( left.kind == VAL_STR && right.kind == VAL_INT ) ||
               ( left.kind == VAL_INT && right.kind == VAL_STR ))
    {
        char *ptr = "";
        char  *str   = (right.kind == VAL_STR) ? val_str(&right) : val_str(&left);
        int    count = (right.kind == VAL_INT) ? val_int(&right) : val_int(&left);
        size_t len   = count * utf8_str_len(str);

        for ( int i = 0; i < len; ++i ) {
            ptr = strf("%s%s", ptr, str);
        }

        result = val_str(ptr);
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val *
operator^(Val left, Val right) {
    Val *result = 0;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        int a = val_int(&left);
        int b = val_int(&right);
        double c = pow(a, b);

        result = val_int((int)c);
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val *
operator/(Val left, Val right) {
    Val *result = 0;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        assert(val_int(&right) != 0);

        int a = val_int(&left);
        int b = val_int(&right);
        double c = (float)a / b;

        result = val_int((int)c);
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_float(val_int(&left) / val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        assert(val_range0(&right) != 0 && val_range1(&right) != 0);
        result = val_range(val_int(&left) / val_range0(&right), val_int(&left) / val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(val_float(&left) / val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        assert(val_range0(&right) != 0 && val_range1(&right) != 0);
        result = val_range(val_range0(&left) * val_range0(&right), val_range1(&left) * val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val *
operator%(Val left, Val right) {
    Val *result = 0;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(val_int(&left) % val_int(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val *
operator+(Val left, Val right) {
    Val *result = 0;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(val_int(&left) + val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_float(val_int(&left) + val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_range(val_int(&left) + val_range0(&right), val_int(&left) + val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(val_float(&left) + val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range(val_range0(&left) + val_range0(&right), val_range1(&left) + val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val *
operator-(Val left, Val right) {
    Val *result = 0;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(val_int(&left) - val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_float(val_int(&left) - val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_range(val_int(&left) - val_range0(&right), val_int(&left) - val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(val_float(&left) - val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range(val_range0(&left) - val_range0(&right), val_range1(&left) - val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc b32
operator<(Val left, Val right) {
    b32 result = false;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(&left) < val_int(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_int(&left) < val_float(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_int(&left) < val_range0(&right);
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(&left) < val_float(&right);
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range0(&left) < val_range0(&right) && val_range1(&left) < val_range1(&right);
    } else if ( left.kind == VAL_STR && right.kind == VAL_STR ) {
        char *lval = val_str(&left);
        char *rval = val_str(&right);
        s32 t = utf8_str_cmp(lval, rval);

        result = (t < 0) ? true : false;
    } else if ( left.kind == VAL_DICT || right.kind == VAL_DICT ) {
        /* @AUFGABE: was ist an dieser stelle sinnvoll? */
        result = false;
    } else {
        illegal_path();
    }

    return result;
}

internal_proc b32
operator<=(Val left, Val right) {
    b32 result = false;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(&left) <= val_int(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_int(&left) <= val_float(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_int(&left) <= val_range0(&right);
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(&left) <= val_float(&right);
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range0(&left) <= val_range0(&right) && val_range1(&left) <= val_range1(&right);
    } else {
        illegal_path();
    }

    return result;
}

internal_proc b32
operator>(Val &left, Val &right) {
    b32 result = false;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(&left) > val_int(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_int(&left) > val_float(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_int(&left) > val_range0(&right);
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(&left) > val_float(&right);
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range0(&left) > val_range0(&right) && val_range1(&left) > val_range1(&right);
    } else if ( left.kind == VAL_STR && right.kind == VAL_STR ) {
        char *lval = val_str(&left);
        char *rval = val_str(&right);
        s32 t = utf8_str_cmp(lval, rval);

        result = (t > 0) ? true : false;
    } else if ( left.kind == VAL_DICT || right.kind == VAL_DICT ) {
        /* @AUFGABE: was ist an dieser stelle sinnvoll? */
        result = true;
    } else {
        illegal_path();
    }

    return result;
}

internal_proc b32
operator>=(Val &left, Val &right) {
    b32 result = false;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result = val_int(&left) >= val_int(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result = val_int(&left) >= val_float(&right);
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result = val_int(&left) >= val_range0(&right);
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result = val_float(&left) >= val_float(&right);
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result = val_range0(&left) >= val_range0(&right) && val_range1(&left) >= val_range1(&right);
    } else {
        illegal_path();
    }

    return result;
}

internal_proc b32
operator==(Val &left, Val &right) {
    if ( left.kind != right.kind ) {
        return false;
    }

    if ( left.kind == VAL_INT ) {
        return val_int(&left) == val_int(&right);
    }

    if ( left.kind == VAL_FLOAT ) {
        f32 eps = 0.00001f;

        if ( (val_float(&left) - val_float(&right) < eps) || (val_float(&left) - val_float(&right) > eps ) ) {
            return false;
        } else {
            return true;
        }
    }

    if ( left.kind == VAL_RANGE ) {
        return val_range0(&left) == val_range0(&right) && val_range1(&left) == val_range1(&right);
    }

    if ( left.kind == VAL_STR ) {
        return val_str(&left) == val_str(&right);
    }

    if ( left.kind == VAL_BOOL ) {
        return val_bool(&left) == val_bool(&right);
    }

    if ( left.kind == VAL_DICT && right.kind == VAL_DICT ) {
        return &left == &right;
    }

    return false;
}

internal_proc b32
operator!=(Val &left, Val &right) {
    b32 result = !(left == right);

    return result;
}

internal_proc b32
operator&&(Val left, Val right) {
    b32 left_result = false;

    if ( left.kind == VAL_INT && val_int(&left) != 0 ) {
        left_result = true;
    } else if ( left.kind == VAL_FLOAT && val_float(&left) > 0.0001 && val_float(&left) < -0.0001 ) {
        left_result = true;
    } else if ( left.kind == VAL_STR && left.len > 0 ) {
        left_result = true;
    } else if ( left.kind == VAL_BOOL ) {
        left_result = val_bool(&left);
    }

    b32 right_result = false;

    if ( right.kind == VAL_INT && val_int(&right) != 0 ) {
        right_result = true;
    } else if ( right.kind == VAL_FLOAT && val_float(&right) > 0.0001 && val_float(&right) < -0.0001 ) {
        right_result = true;
    } else if ( right.kind == VAL_STR && right.len > 0 ) {
        right_result = true;
    } else if ( right.kind == VAL_BOOL ) {
        right_result = val_bool(&right);
    }

    return left_result && right_result;
}

internal_proc b32
operator||(Val left, Val right) {
    b32 left_result = false;

    if ( left.kind == VAL_INT && val_int(&left) != 0 ) {
        left_result = true;
    } else if ( left.kind == VAL_FLOAT && val_float(&left) > 0.0001 && val_float(&left) < -0.0001 ) {
        left_result = true;
    } else if ( left.kind == VAL_STR && left.len > 0 ) {
        left_result = true;
    } else if ( left.kind == VAL_BOOL ) {
        left_result = val_bool(&left);
    }

    b32 right_result = false;

    if ( right.kind == VAL_INT && val_int(&right) != 0 ) {
        right_result = true;
    } else if ( right.kind == VAL_FLOAT && val_float(&right) > 0.0001 && val_float(&right) < -0.0001 ) {
        right_result = true;
    } else if ( right.kind == VAL_STR && right.len > 0 ) {
        right_result = true;
    } else if ( right.kind == VAL_BOOL ) {
        right_result = val_bool(&right);
    }

    return left_result || right_result;
}

internal_proc Val *
val_elem(Val *val, int idx) {
    Val *result = val;

    switch ( val->kind ) {
        case VAL_RANGE: {
            result = val_int(val_range0(val) + idx);
        } break;

        case VAL_LIST: {
            result = *((Val **)val->ptr + idx);
        } break;

        case VAL_TUPLE: {
            result = *((Val **)val->ptr + idx);
        } break;

        case VAL_PAIR: {
            Resolved_Pair *pair = (Resolved_Pair *)val->ptr;
            result = (idx == 0) ? pair->key : pair->value;
        } break;

        case VAL_STR: {
            result = val_str(utf8_char_goto((char *)val->ptr, idx), 1);
        } break;

        case VAL_DICT: {
            if ( idx < val->len ) {
                Sym *sym = scope_elem(val->scope, idx);

                /* @AUFGABE: die val_pairs bei der erstellung
                 *           der dict anlegen
                 */
                result = val_pair(val_str(sym_name(sym)), sym_val(sym));
            }
        } break;

        default: {
            warn(0, 0, "nicht unterstützter datentyp übergeben");
        }
    }

    return result;
}
/* }}} */
/* type {{{ */
struct Type_Field {
    char *name;
    Sym *sym;
    Type *type;
    Val  *default_value;
};

internal_proc Type_Field *
type_field(char *name, Type *type, Val *default_value = val_undefined()) {
    Type_Field *result = ALLOC_STRUCT(&resolve_arena, Type_Field);

    result->name = intern_str(name);
    result->type = type;
    result->default_value = default_value;

    return result;
}

internal_proc char *
type_field_name(Type_Field *field) {
    char *result = field->name;

    return result;
}

internal_proc Val *
type_field_value(Type_Field *field) {
    Val *result = field->default_value;

    return result;
}

enum Type_Kind {
    TYPE_NONE,
    TYPE_ANY,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STR,
    TYPE_RANGE,
    TYPE_LIST,
    TYPE_DICT,
    TYPE_TUPLE,
    TYPE_PROC,
    TYPE_MODULE,

    TYPE_COUNT,
};
enum Type_Flags {
    TYPE_FLAGS_NONE     = 0x0,
    TYPE_FLAGS_CALLABLE = 0x1,
    TYPE_FLAGS_ITERABLE = 0x2,
    TYPE_FLAGS_CONST    = 0x4,
};
struct Type {
    Type_Kind kind;
    s64 size;
    u32 flags;
    Scope *scope;

    union {
        struct {
            size_t num_elems;
        } type_tuple;

        struct {
            Type *base;
        } type_list;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
        } type_proc;

        struct {
            char *name;
        } type_module;
    };
};

enum { PTR_SIZE = 8 };


global_var Type type_none = { TYPE_NONE };
global_var Type type_undefined;
global_var Type *type_bool;
global_var Type *type_int;
global_var Type *type_float;
global_var Type *type_str;
global_var Type *type_range;
global_var Type *type_any;

internal_proc Type *
type_base(Type *type) {
    Type *result = 0;

    if ( !type ) {
        return result;
    }

    switch ( type->kind ) {
        case TYPE_LIST: {
            result = type->type_list.base;
        } break;

        default: {
            result = type;
        } break;
    }

    return result;
}

internal_proc Type *
type_new(Type_Kind kind) {
    Type *result = ALLOC_STRUCT(&resolve_arena, Type);

    result->kind  = kind;
    result->scope = 0;

    return result;
}

internal_proc Type *
type_tuple(size_t num_elems) {
    Type *result = type_new(TYPE_TUPLE);

    result->flags = TYPE_FLAGS_CONST|TYPE_FLAGS_ITERABLE;

    result->type_tuple.num_elems = num_elems;

    return result;
}

internal_proc Type *
type_proc(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_PROC);

    result->flags = TYPE_FLAGS_CALLABLE;

    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;
    result->scope = &type_any_scope;

    return result;
}

internal_proc Type *
type_list(Type *type) {
    Type *result = type_new(TYPE_LIST);

    result->type_list.base = type;
    result->scope = &type_list_scope;

    return result;
}

internal_proc Type *
type_dict(Scope *scope) {
    Type *result = type_new(TYPE_DICT);

    result->scope = scope;

    if ( scope ) {
        result->scope->parent = &type_dict_scope;
    }

    return result;
}

internal_proc b32
type_is_int(Type *type) {
    b32 result = (type->kind == TYPE_INT);

    return result;
}

internal_proc b32
type_is_arithmetic(Type *type) {
    b32 result = TYPE_INT <= type->kind && type->kind <= TYPE_FLOAT;

    return result;
}

internal_proc b32
type_is_scalar(Type *type) {
    b32 result = TYPE_BOOL <= type->kind && type->kind <= TYPE_FLOAT;

    return result;
}

internal_proc b32
type_is_callable(Type *type) {
    b32 result = type->flags & TYPE_FLAGS_CALLABLE;

    return result;
}

internal_proc b32
type_is_iterable(Type *type) {
    b32 result = type->flags & TYPE_FLAGS_ITERABLE;

    return result;
}

internal_proc char *
type_pprint(Type *type) {
    char *result = "";

    if ( !type ) {
        return "<null>";
    }

    switch ( type->kind ) {
        case TYPE_INT: {
            result = "int()";
        } break;

        case TYPE_FLOAT: {
            result = "float()";
        } break;

        case TYPE_STR: {
            result = "string()";
        } break;

        case TYPE_BOOL: {
            result = "bool()";
        } break;

        case TYPE_RANGE: {
            result = "range()";
        } break;

        case TYPE_LIST: {
            result = "list()";
        } break;

        case TYPE_TUPLE: {
            result = "tuple()";
        } break;

        case TYPE_DICT: {
            result = "dict()";
        } break;

        case TYPE_PROC: {
            result = "proc()";
        } break;
    }

    return result;
}
/* }}} */

