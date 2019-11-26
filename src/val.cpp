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
};

internal_proc Val *
val_new(Val_Kind kind, size_t size) {
    Val *result = ALLOC_STRUCT(&resolve_arena, Val);

    result->kind = kind;
    result->size = size;
    result->len  = 1;
    result->ptr  = (void *)ALLOC_SIZE(&resolve_arena, size);
    result->user_data = 0;

    return result;
}

internal_proc Val *
val_none() {
    Val *result = val_new(VAL_NONE, 0);

    result->size = 0;
    result->len  = 0;
    result->ptr  = 0;
    result->user_data = 0;

    return result;
}

internal_proc Val *
val_undefined() {
    Val *result = val_new(VAL_UNDEFINED, 0);

    result->size = 0;
    result->len  = 0;
    result->ptr  = 0;
    result->user_data = 0;

    return result;
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

    return result;
}

internal_proc float
val_float(Val *val) {
    return *(float *)val->ptr;
}

internal_proc Val *
val_str(char *val, size_t len = 0) {
    Val *result = val_new(VAL_STR, sizeof(char*));

    result->len = (len) ? len : utf8_strlen(val);
    result->ptr = intern_str(val);

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

    result->len = max - min;
    *((int *)result->ptr)   = min;
    *((int *)result->ptr+1) = max;
    *((int *)result->ptr+2) = step;

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
    Val *result = val_new(VAL_LIST, sizeof(Val)*num_vals);

    result->ptr = (Val **)AST_DUP(vals);
    result->len = num_vals;

    return result;
}

internal_proc Val *
val_dict(Scope *scope) {
    Val *result = val_new(VAL_DICT, sizeof(Map));

    result->len = (scope) ? scope->num_syms : 0;
    result->ptr = scope;

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
            char *result = strf("%.*s", (int)len, (char *)val->ptr);
            return result;
        } break;

        case VAL_INT: {
            char *result = strf("%d", val_int(val));
            return result;
        } break;

        case VAL_FLOAT: {
            char *result = strf("%.9g", val_float(val));
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
            return "none";
        } break;

        default: {
            fatal(0, 0, "datentyp kann nicht in zeichenkette umgewandelt werden.\n");
            return "";
        } break;
    }
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

internal_proc Val
operator*(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        result.size = sizeof(s32);
        result.ptr = ALLOC_SIZE(&resolve_arena, result.size);
        val_set(&result, val_int(&left) * val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_float(&left) * val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        val_set(&result, val_int(&left) * val_range0(&right), val_int(&left) * val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_float(&left) * val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        val_set(&result, val_range0(&left) * val_range0(&right), val_range1(&left) * val_range1(&right));
    } else if (( left.kind == VAL_STR && right.kind == VAL_INT ) ||
               ( left.kind == VAL_INT && right.kind == VAL_STR ))
    {
        result.kind = VAL_STR;
        result.ptr = "";

        char  *str   = (right.kind == VAL_STR) ? val_str(&right) : val_str(&left);
        int    count = (right.kind == VAL_INT) ? val_int(&right) : val_int(&left);
        size_t len   = count * utf8_strlen(str);

        for ( int i = 0; i < len; ++i ) {
            result.ptr = strf("%s%s", result.ptr, str);
        }

        result.size = len*sizeof(char);
        result.len  = len;
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator^(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        result.size = sizeof(s32);
        result.ptr = ALLOC_SIZE(&resolve_arena, result.size);

        int a = val_int(&left);
        int b = val_int(&right);
        double c = pow(a, b);

        val_set(&result, (int)c);
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator/(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        assert(val_int(&right) != 0);
        result.size = sizeof(s32);
        result.ptr = ALLOC_SIZE(&resolve_arena, result.size);

        int a = val_int(&left);
        int b = val_int(&right);
        auto c = (float)a / b;

        val_set(&result, c);
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_int(&left) / val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        assert(val_range0(&right) != 0 && val_range1(&right) != 0);
        val_set(&result, val_int(&left) / val_range0(&right), val_int(&left) / val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_float(&left) / val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        assert(val_range0(&right) != 0 && val_range1(&right) != 0);
        val_set(&result, val_range0(&left) * val_range0(&right), val_range1(&left) * val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator%(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        result.size = sizeof(s32);
        result.ptr = ALLOC_SIZE(&resolve_arena, result.size);
        val_set(&result, val_int(&left) % val_int(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator+(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        val_set(&result, val_int(&left) + val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_int(&left) + val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        val_set(&result, val_int(&left) + val_range0(&right), val_int(&left) + val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_float(&left) + val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        val_set(&result, val_range0(&left) + val_range0(&right), val_range1(&left) + val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator-(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        val_set(&result, val_int(&left) - val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_int(&left) - val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        val_set(&result, val_int(&left) - val_range0(&right), val_int(&left) - val_range1(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        val_set(&result, val_float(&left) - val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        val_set(&result, val_range0(&left) - val_range0(&right), val_range1(&left) - val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator<(Val left, Val right) {
    Val result = {};
    result.kind = VAL_BOOL;

    /* @AUFGABE: an dieser stelle die exec_arena verwenden */
    result.ptr = ALLOC_SIZE(&resolve_arena, sizeof(b32));

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        val_set(&result, val_int(&left) < val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_int(&left) < val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        val_set(&result, val_int(&left) < val_range0(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_float(&left) < val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        val_set(&result, val_range0(&left) < val_range0(&right) && val_range1(&left) < val_range1(&right));
    } else if ( left.kind == VAL_STR && right.kind == VAL_STR ) {
        char *lval = val_str(&left);
        char *rval = val_str(&right);
        s32 t = utf8_strcmp(lval, rval);
        val_set(&result, (t < 0) ? true : false );
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator<=(Val left, Val right) {
    Val result = {};
    result.kind = VAL_BOOL;

    /* @AUFGABE: an dieser stelle die exec_arena verwenden */
    result.ptr = ALLOC_SIZE(&resolve_arena, sizeof(b32));

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        val_set(&result, val_int(&left) <= val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_int(&left) <= val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        val_set(&result, val_int(&left) <= val_range0(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_float(&left) <= val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        val_set(&result, val_range0(&left) <= val_range0(&right) && val_range1(&left) <= val_range1(&right));
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator>(Val &left, Val &right) {
    Val result = {};
    result.kind = VAL_BOOL;

    /* @AUFGABE: an dieser stelle die exec_arena verwenden */
    result.ptr = ALLOC_SIZE(&resolve_arena, sizeof(b32));

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        val_set(&result, val_int(&left) > val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_int(&left) > val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        val_set(&result, val_int(&left) > val_range0(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_float(&left) > val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        val_set(&result, val_range0(&left) > val_range0(&right) && val_range1(&left) > val_range1(&right));
    } else if ( left.kind == VAL_STR && right.kind == VAL_STR ) {
        char *lval = val_str(&left);
        char *rval = val_str(&right);
        s32 t = utf8_strcmp(lval, rval);
        val_set(&result, (t > 0) ? true : false );
    } else {
        illegal_path();
    }

    return result;
}

internal_proc Val
operator>=(Val &left, Val &right) {
    Val result = {};
    result.kind = VAL_BOOL;

    /* @AUFGABE: an dieser stelle die exec_arena verwenden */
    result.ptr = ALLOC_SIZE(&resolve_arena, sizeof(b32));

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        val_set(&result, val_int(&left) >= val_int(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_int(&left) >= val_float(&right));
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        val_set(&result, val_int(&left) >= val_range0(&right));
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        val_set(&result, val_float(&left) >= val_float(&right));
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        val_set(&result, val_range0(&left) >= val_range0(&right) && val_range1(&left) >= val_range1(&right));
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
                Sym *sym = scope_elem((Scope *)val->ptr, idx);

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

