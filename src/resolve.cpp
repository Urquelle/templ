/* scope {{{ */
struct Scope {
    char*  name;
    Scope* parent;
    Map    syms;
    Sym  **sym_list;
    size_t num_syms;
};

global_var Scope filter_scope;
global_var Scope system_scope;
global_var Scope tester_scope;
global_var Scope global_scope;

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
/* }}} */
/* val {{{ */
enum Val_Kind {
    VAL_UNDEFINED,
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
};

struct Val {
    Val_Kind kind;
    size_t size;
    size_t len;
    void  *ptr;
};

internal_proc Val *
val_new(Val_Kind kind, size_t size) {
    Val *result = ALLOC_STRUCT(&resolve_arena, Val);

    result->kind = kind;
    result->size = size;
    result->len  = 0;
    result->ptr  = (void *)ALLOC_SIZE(&resolve_arena, size);

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
val_bool(s32 val) {
    Val *result = val_new(VAL_BOOL, sizeof(b32));

    *((s32 *)result->ptr) = val;

    return result;
}

internal_proc s32
val_bool(Val *val) {
    return *(s32 *)val->ptr;
}

internal_proc Val *
val_neg(Val *val) {
    assert(val->kind == VAL_BOOL);

    Val *result = val_new(VAL_BOOL, sizeof(s32));

    s32 value = (s32)val_bool(val);
    *((s32 *)result->ptr) = ~(value & 0x1);

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
    result->ptr = val;

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

internal_proc Val *
val_pair(Resolved_Pair *pair) {
    Val *result = val_new(VAL_PAIR, sizeof(Resolved_Pair*));

    result->len = 1;
    result->ptr = pair;

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
val_dict(Val **pairs, size_t num_pairs) {
    Val *result = val_new(VAL_DICT, sizeof(Map));

    result->len = num_pairs;
    result->ptr = pairs;

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
    *((s32 *)val->ptr) = value;
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

internal_proc Val *
val_op(Token_Kind op, Val *val) {
    if ( op == T_MINUS ) {
        if ( val->kind == VAL_INT ) {
            return val_int( val_int(val)*-1 );
        }
    }

    return val;
}

global_var char val_to_char_buf[1000];
internal_proc char *
val_to_char(Val *val) {
    switch ( val->kind ) {
        case VAL_STR: {
            size_t len = utf8_str_size((char *)val->ptr, val->len);
            sprintf(val_to_char_buf, "%.*s", (int)len, (char *)val->ptr);
            return val_to_char_buf;
        } break;

        case VAL_INT: {
            sprintf(val_to_char_buf, "%d", val_int(val));
            return val_to_char_buf;
        } break;

        case VAL_FLOAT: {
            sprintf(val_to_char_buf, "%.9g", val_float(val));
            return val_to_char_buf;
        } break;

        case VAL_UNDEFINED: {
            return "<undefined>";
        } break;

        case VAL_BOOL: {
            erstes_if ( val_bool(val) > 0 ) {
                return "true";
            } else if ( val_bool(val) == 0 ) {
                return "false";
            } else {
                return "none";
            }
        } break;

        case VAL_CHAR: {
            Val *orig = (Val *)val->ptr;
            char *ptr = utf8_char_goto((char *)orig->ptr, val->len);
            int size = (int)utf8_char_size(ptr);

            sprintf(val_to_char_buf, "%.*s", size, ptr);
            return val_to_char_buf;
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

enum Type_Kind {
    TYPE_NONE,
    TYPE_ANY,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STR,
    TYPE_LIST,
    TYPE_DICT,
    TYPE_TUPLE,
    TYPE_PROC,
    TYPE_MACRO,
    TYPE_FILTER,
    TYPE_TEST,
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

    union {
        struct {
            size_t num_elems;
        } type_tuple;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
            Proc_Callback *callback;
            b32 variadic;
        } type_proc;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } type_macro;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
            Filter_Callback *callback;
            b32 variadic;
        } type_filter;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
            Test_Callback *callback;
        } type_test;

        struct {
            char *name;
            Scope *scope;
        } type_module;

        struct {
            Scope *scope;
        } type_dict;
    };
};

enum { PTR_SIZE = 8 };


global_var Type type_none = { TYPE_NONE };
global_var Type type_undefined;
global_var Type *type_void;
global_var Type *type_bool;
global_var Type *type_int;
global_var Type *type_float;
global_var Type *type_str;
global_var Type *type_list;
global_var Type *type_any;

global_var Type *arithmetic_result_type_table[TYPE_COUNT][TYPE_COUNT];
global_var Type *scalar_result_type_table[TYPE_COUNT][TYPE_COUNT];

internal_proc Type *
type_new(Type_Kind kind) {
    Type *result = ALLOC_STRUCT(&resolve_arena, Type);

    result->kind = kind;

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
type_proc(Type_Field **params, size_t num_params, Type *ret,
        Proc_Callback *callback = 0, b32 variadic = false)
{
    Type *result = type_new(TYPE_PROC);

    result->flags = TYPE_FLAGS_CALLABLE|TYPE_FLAGS_CONST;

    result->size = PTR_SIZE;
    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;
    result->type_proc.callback = callback;
    result->type_proc.variadic = variadic;

    return result;
}

internal_proc Type *
type_macro(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_MACRO);

    result->flags = TYPE_FLAGS_CALLABLE|TYPE_FLAGS_CONST;

    result->size = PTR_SIZE;
    result->type_macro.params = (Type_Field **)AST_DUP(params);
    result->type_macro.num_params = num_params;
    result->type_macro.ret = ret;

    return result;
}

internal_proc Type *
type_filter(Type_Field **params, size_t num_params, Type *ret,
        Filter_Callback *callback, b32 variadic = false)
{
    Type *result = type_new(TYPE_FILTER);

    result->flags = TYPE_FLAGS_CALLABLE|TYPE_FLAGS_CONST;

    result->size = PTR_SIZE;
    result->type_filter.params = (Type_Field **)AST_DUP(params);
    result->type_filter.num_params = num_params;
    result->type_filter.ret = ret;
    result->type_filter.callback = callback;
    result->type_filter.variadic = variadic;

    return result;
}

internal_proc Type *
type_test(Type_Field **params, size_t num_params, Test_Callback *callback) {
    Type *result = type_new(TYPE_TEST);

    result->flags = TYPE_FLAGS_CALLABLE|TYPE_FLAGS_CONST;

    result->size = PTR_SIZE;
    result->type_test.params = (Type_Field **)AST_DUP(params);
    result->type_test.num_params = num_params;
    result->type_test.ret = type_bool;
    result->type_test.callback = callback;

    return result;
}

internal_proc Type *
type_module(char *name, Scope *scope) {
    Type *result = type_new(TYPE_MODULE);

    result->flags = TYPE_FLAGS_CONST;

    result->type_module.name = name;
    result->type_module.scope = scope;

    return result;
}

internal_proc Type *
type_dict(Scope *scope, u32 flags = TYPE_FLAGS_NONE) {
    Type *result = type_new(TYPE_DICT);

    result->flags = flags;

    result->type_dict.scope = scope;

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

internal_proc size_t
type_num_elems(Type *type) {
    size_t result = 1;

    switch ( type->kind ) {
        case TYPE_TUPLE: {
            result = type->type_tuple.num_elems;
        } break;

        case TYPE_LIST: {
        } break;

        case TYPE_DICT: {
            result = type->type_dict.scope->num_syms;
        } break;
    }

    return result;
}
/* }}} */
/* sym {{{ */
enum Sym_Kind {
    SYM_NONE,
    SYM_VAR,
    SYM_PROC,
    SYM_MODULE,
};

struct Sym {
    Sym_Kind kind;
    Scope *scope;
    char *name;
    Type *type;
    Val  *val;
};

global_var Sym sym_undefined = { SYM_NONE, 0, intern_str("undefined"), &type_undefined, val_undefined() };

internal_proc b32
sym_valid(Sym *sym) {
    b32 result = sym != &sym_undefined;

    return result;
}

internal_proc b32
sym_invalid(Sym *sym) {
    b32 result = sym == &sym_undefined;

    return result;
}

internal_proc Sym *
sym_new(Sym_Kind kind, char *name, Type *type, Val *val = val_undefined()) {
    Sym *result = ALLOC_STRUCT(&resolve_arena, Sym);

    result->kind  = kind;
    result->scope = current_scope;
    result->name  = name;
    result->type  = type;
    result->val   = val;

    return result;
}

internal_proc Sym *
sym_get(char *name) {
    /* @INFO: symbole werden in einer map ohne external chaining gespeichert! */
    name = intern_str(name);
    for ( Scope *it = current_scope; it; it = it->parent ) {
        Sym *sym = (Sym *)map_get(&it->syms, name);
        if ( sym ) {
            return sym;
        }
    }

    return &sym_undefined;
}

internal_proc void
sym_clear() {
    for ( Scope *scope = current_scope; scope; scope = scope->parent ) {
        if ( scope == &system_scope ) {
            break;
        }
        map_reset(&scope->syms);
    }
}

internal_proc Sym *
sym_push(Sym_Kind kind, char *name, Type *type, Val *val = val_undefined()) {
    name = intern_str(name);

    Sym *sym = (Sym *)map_get(&current_scope->syms, name);
    if ( sym && sym->scope == current_scope ) {
        /* @AUFGABE: datei und zeile */
        fatal(0, 0, "symbol %s existiert bereits", name);
    } else if ( sym ) {
        /* @AUFGABE: datei und zeile */
        warn(0, 0, "symbol %s wird überschattet", name);
    }

    Sym *result = sym_new(kind, name, type, val);
    map_put(&current_scope->syms, name, result);

    buf_push(current_scope->sym_list, result);
    current_scope->num_syms = buf_len(current_scope->sym_list);

    return result;
}

internal_proc Sym *
sym_push_filter(char *name, Type *type) {
    Scope *prev_scope = scope_set(&filter_scope);
    Sym *result = sym_push(SYM_PROC, name, type);
    scope_set(prev_scope);

    return result;
}

internal_proc Sym *
sym_push_test(char *name, Type *type) {
    Scope *prev_scope = scope_set(&tester_scope);
    Sym *result = sym_push(SYM_PROC, name, type);
    scope_set(prev_scope);

    return result;
}

internal_proc Sym *
sym_push_sysproc(char *name, Type *type) {
    Scope *prev_scope = scope_set(&system_scope);
    Sym *result = sym_push(SYM_PROC, name, type);
    scope_set(prev_scope);

    return result;
}

internal_proc Sym *
sym_push_var(char *name, Type *type, Val *val) {
    return sym_push(SYM_VAR, name, type, val);
}

internal_proc Sym *
sym_push_proc(char *name, Type *type) {
    return sym_push(SYM_PROC, name, type);
}

internal_proc Sym *
sym_push_module(char *name, Type *type) {
    return sym_push(SYM_MODULE, name, type);
}
/* }}} */
/* resolved_arg {{{ */
struct Resolved_Arg {
    Pos pos;
    char *name;
    Type *type;
    Val *val;
};

internal_proc Resolved_Arg *
resolved_arg(Pos pos, char *name, Type *type, Val *val) {
    Resolved_Arg *result = ALLOC_STRUCT(&resolve_arena, Resolved_Arg);

    result->pos = pos;
    result->name = name;
    result->type = type;
    result->val = val;

    return result;
}
/* }}} */
/* resolved_pair {{{ */
struct Resolved_Pair {
    Pos pos;
    Val *key;
    Val *value;
};

internal_proc Resolved_Pair *
resolved_pair(Pos pos, Val *key, Val *value) {
    Resolved_Pair *result = ALLOC_STRUCT(&resolve_arena, Resolved_Pair);

    result->pos = pos;
    result->key = key;
    result->value = value;

    return result;
}
/* }}} */
/* resolved_expr {{{ */
struct Resolved_Expr {
    Expr_Kind kind;
    Pos pos;
    Type *type;
    Sym *sym;
    Val *val;

    Resolved_Stmt *stmt;

    Resolved_Expr **filters;
    size_t num_filters;

    b32 is_const;
    b32 is_lvalue;

    union {
        struct {
        } expr_bool;

        struct {
        } expr_int;

        struct {
        } expr_float;

        struct {
        } expr_str;

        struct {
        } expr_name;

        struct {
            Resolved_Expr *expr;
        } expr_paren;

        struct {
            int min;
            int max;
        } expr_range;

        struct {
            Resolved_Expr *base;
        } expr_field;

        struct {
            Resolved_Expr *expr;
            Resolved_Expr *index;
        } expr_subscript;

        struct {
            Token_Kind op;
            Resolved_Expr *expr;
        } expr_unary;

        struct {
            Token_Kind op;
            Resolved_Expr *left;
            Resolved_Expr *right;
        } expr_binary;

        struct {
            Resolved_Expr *operand;
            Resolved_Expr *tester;
        } expr_is;

        struct {
            Resolved_Expr *expr;
        } expr_not;

        struct {
            Resolved_Expr *set;
        } expr_in;

        struct {
            Resolved_Expr *expr;
            Map *nargs;
            Resolved_Arg **kwargs;
            size_t num_kwargs;
            Resolved_Arg **varargs;
            size_t num_varargs;
        } expr_call;

        struct {
            Resolved_Expr *cond;
            Resolved_Expr *else_expr;
        } expr_if;

        struct {
            Resolved_Expr **exprs;
            size_t num_exprs;
        } expr_tuple;

        struct {
            Resolved_Expr **expr;
            size_t num_expr;
        } expr_list;

        struct {
            char **keys;
            size_t num_keys;
        } expr_dict;
    };
};

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

        case VAL_DICT: {
            if ( idx < val->len ) {
                result = ((Val **)val->ptr)[idx];
            }
        } break;

        case VAL_PAIR: {
            idx %= 2;
            if ( idx == 0 ) {
                result = ((Resolved_Pair *)val->ptr)->key;
            } else {
                result = ((Resolved_Pair *)val->ptr)->value;
            }
        } break;

        default: {
            warn(0, 0, "nicht unterstützter datentyp übergeben");
        }
    }

    return result;
}

global_var Resolved_Expr resolved_expr_undefined = { EXPR_NONE, {}, &type_undefined, &sym_undefined, val_undefined() };

internal_proc Resolved_Expr *
resolved_expr_new(Expr_Kind kind, Type *type = 0) {
    Resolved_Expr *result = ALLOC_STRUCT(&resolve_arena, Resolved_Expr);

    result->is_lvalue = false;
    result->is_const = false;
    result->type = type;
    result->kind = kind;
    result->sym = &sym_undefined;
    result->val = val_undefined();

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_bool(Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_BOOL, type);

    result->val = val;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_int(Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_INT, type);

    result->val = val;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_float(Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_FLOAT, type);

    result->val = val;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_str(Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_STR, type);

    result->val = val;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_name(Sym *sym, Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_NAME, type);

    result->sym = sym;
    result->val = val;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_field(Resolved_Expr *base, Sym *sym, Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_FIELD, type);

    result->is_lvalue = true;
    result->sym = sym;
    result->val = val;
    result->expr_field.base = base;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_paren(Resolved_Expr *expr) {
    Resolved_Expr *result = resolved_expr_new(EXPR_PAREN);

    result->expr_paren.expr = expr;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_range(int min, int max) {
    Resolved_Expr *result = resolved_expr_new(EXPR_RANGE, type_int);

    result->expr_range.min = min;
    result->expr_range.max = max;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_unary(Token_Kind op, Resolved_Expr *expr) {
    Resolved_Expr *result = resolved_expr_new(EXPR_UNARY, expr->type);

    result->val = expr->val;
    result->expr_unary.op = op;
    result->expr_unary.expr = expr;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_binary(Token_Kind op, Type *type, Resolved_Expr *left, Resolved_Expr *right) {
    Resolved_Expr *result = resolved_expr_new(EXPR_BINARY, type);

    result->expr_binary.op = op;
    result->expr_binary.left = left;
    result->expr_binary.right = right;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_is(Resolved_Expr *operand, Resolved_Expr *tester) {
    Resolved_Expr *result = resolved_expr_new(EXPR_IS, type_bool);

    result->expr_is.operand = operand;
    result->expr_is.tester  = tester;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_not(Resolved_Expr *expr) {
    Resolved_Expr *result = resolved_expr_new(EXPR_NOT, type_bool);

    result->expr_not.expr = expr;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_call(Resolved_Expr *expr, Map *nargs, Resolved_Arg **kwargs,
        size_t num_kwargs, Resolved_Arg **varargs, size_t num_varargs,
        Type *type)
{
    Resolved_Expr *result = resolved_expr_new(EXPR_CALL, type);

    result->expr_call.expr        = expr;
    result->expr_call.nargs       = nargs;
    result->expr_call.kwargs      = (Resolved_Arg **)AST_DUP(kwargs);
    result->expr_call.num_kwargs  = num_kwargs;
    result->expr_call.varargs     = (Resolved_Arg **)AST_DUP(varargs);
    result->expr_call.num_varargs = num_varargs;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_if(Resolved_Expr *cond, Resolved_Expr *else_expr) {
    Resolved_Expr *result = resolved_expr_new(EXPR_IF, type_bool);

    result->expr_if.cond = cond;
    result->expr_if.else_expr = else_expr;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_subscript(Resolved_Expr *expr, Resolved_Expr *index, Type *type) {
    Resolved_Expr *result = resolved_expr_new(EXPR_SUBSCRIPT, type);

    result->expr_subscript.expr  = expr;
    result->expr_subscript.index = index;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_tuple(Resolved_Expr **exprs, size_t num_exprs, Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_TUPLE, type);

    result->val = val;
    result->expr_tuple.exprs     = (Resolved_Expr **)AST_DUP(exprs);
    result->expr_tuple.num_exprs = num_exprs;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_list(Resolved_Expr **expr, size_t num_expr, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_LIST);

    result->val = val;
    result->type = type_list;
    result->expr_list.expr = expr;
    result->expr_list.num_expr = num_expr;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_dict(char **keys, size_t num_keys, Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_DICT, type);

    result->val = val;
    result->expr_dict.keys = keys;
    result->expr_dict.num_keys = num_keys;

    return result;
}
/* }}} */
/* resolved_stmt {{{ */
struct Resolved_Stmt {
    Stmt_Kind kind;
    Pos pos;

    union {
        struct {
            Resolved_Expr *expr;
            Resolved_Expr *if_expr;
        } stmt_var;

        struct {
            Sym **vars;
            size_t num_vars;
            Resolved_Expr *set;

            Resolved_Stmt **stmts;
            size_t num_stmts;
            Resolved_Stmt **else_stmts;
            size_t num_else_stmts;

            Sym *loop_index;
            Sym *loop_index0;
            Sym *loop_revindex;
            Sym *loop_revindex0;
            Sym *loop_first;
            Sym *loop_last;
            Sym *loop_length;
            Sym *loop_cycle;
        } stmt_for;

        struct {
            Resolved_Expr *expr;
            Resolved_Stmt **stmts;
            size_t num_stmts;
            Resolved_Stmt **elseifs;
            size_t num_elseifs;
            Resolved_Stmt *else_stmt;
        } stmt_if;

        struct {
            Resolved_Stmt *parent_block;
            Resolved_Stmt *child_block;
            char *name;
            Resolved_Stmt **stmts;
            size_t num_stmts;
            b32 executed;
        } stmt_block;

        struct {
            char *lit;
        } stmt_lit;

        struct {
            char *value;
        } stmt_raw;

        struct {
            Resolved_Expr **names;
            size_t num_names;
            Resolved_Expr *expr;
        } stmt_set;

        struct {
            Resolved_Expr **filter;
            size_t num_filter;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_filter;

        struct {
            Resolved_Templ **templ;
            size_t num_templ;
        } stmt_include;

        struct {
            char *name;
            Resolved_Templ *tmpl;
            Resolved_Templ *else_tmpl;
            Resolved_Expr *if_expr;
        } stmt_extends;

        struct {
            Sym *sym;
            Type *type;
            Type_Field **params;
            size_t num_params;
        } stmt_macro;

        struct {
            Sym *sym;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_module;

        struct {
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_with;
    };
};

internal_proc Resolved_Stmt *
resolved_stmt_new(Stmt_Kind kind) {
    Resolved_Stmt *result = ALLOC_STRUCT(&resolve_arena, Resolved_Stmt);

    result->kind = kind;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_var(Resolved_Expr *expr, Resolved_Expr *if_expr) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_VAR);

    result->stmt_var.expr    = expr;
    result->stmt_var.if_expr = if_expr;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_if(Resolved_Expr *expr, Resolved_Stmt **stmts, size_t num_stmts,
        Resolved_Stmt **elseifs, size_t num_elseifs, Resolved_Stmt *else_stmt)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_IF);

    result->stmt_if.expr = expr;
    result->stmt_if.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;
    result->stmt_if.elseifs = elseifs;
    result->stmt_if.num_elseifs = num_elseifs;
    result->stmt_if.else_stmt = else_stmt;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_elseif(Resolved_Expr *expr, Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_ELSEIF);

    result->stmt_if.expr = expr;
    result->stmt_if.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_else(Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_ELSE);

    result->stmt_if.expr = 0;
    result->stmt_if.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_for(Sym **vars, size_t num_vars, Resolved_Expr *set, Resolved_Stmt **stmts,
        size_t num_stmts, Resolved_Stmt **else_stmts, size_t num_else_stmts,
        Sym *loop_index, Sym *loop_index0, Sym *loop_revindex, Sym *loop_revindex0,
        Sym *loop_first, Sym *loop_last, Sym *loop_length, Sym *loop_cycle)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_FOR);

    result->stmt_for.vars = (Sym **)AST_DUP(vars);
    result->stmt_for.num_vars = num_vars;
    result->stmt_for.set = set;

    result->stmt_for.stmts = stmts;
    result->stmt_for.num_stmts = num_stmts;

    result->stmt_for.else_stmts = else_stmts;
    result->stmt_for.num_else_stmts = num_else_stmts;

    result->stmt_for.loop_index     = loop_index;
    result->stmt_for.loop_index0    = loop_index0;
    result->stmt_for.loop_revindex  = loop_revindex;
    result->stmt_for.loop_revindex0 = loop_revindex0;
    result->stmt_for.loop_first     = loop_first;
    result->stmt_for.loop_last      = loop_last;
    result->stmt_for.loop_length    = loop_length;
    result->stmt_for.loop_cycle     = loop_cycle;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_lit(char *val) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_LIT);

    result->stmt_lit.lit = val;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_raw(char *value) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_RAW);

    result->stmt_raw.value = value;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_set(Resolved_Expr **names, size_t num_names, Resolved_Expr *expr) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_SET);

    result->stmt_set.names = names;
    result->stmt_set.num_names = num_names;
    result->stmt_set.expr = expr;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_block(char *name, Resolved_Stmt **stmts, size_t num_stmts,
        Resolved_Stmt *parent_block)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_BLOCK);

    result->stmt_block.name = name;
    result->stmt_block.stmts = stmts;
    result->stmt_block.num_stmts = num_stmts;
    result->stmt_block.parent_block = parent_block;
    result->stmt_block.child_block = 0;
    result->stmt_block.executed = false;

    if ( !parent_block ) {
    }

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_filter(Resolved_Expr **filter, size_t num_filter,
        Resolved_Stmt **stmts, size_t num_stmts)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_FILTER);

    result->stmt_filter.filter = filter;
    result->stmt_filter.num_filter = num_filter;
    result->stmt_filter.stmts = stmts;
    result->stmt_filter.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_include(Resolved_Templ **templ, size_t num_templ) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_INCLUDE);

    result->stmt_include.templ = templ;
    result->stmt_include.num_templ = num_templ;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_extends(char *name, Resolved_Templ *tmpl, Resolved_Templ *else_tmpl,
        Resolved_Expr *if_expr)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_EXTENDS);

    result->stmt_extends.name = name;
    result->stmt_extends.tmpl = tmpl;
    result->stmt_extends.else_tmpl = else_tmpl;
    result->stmt_extends.if_expr = if_expr;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_macro(Sym *sym, Type *type) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_MACRO);

    result->stmt_macro.sym = sym;
    result->stmt_macro.type = type;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_import(Sym *sym, Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_IMPORT);

    result->stmt_module.sym = sym;
    result->stmt_module.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_module.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_from_import(Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_FROM_IMPORT);

    result->stmt_module.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_module.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_with(Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_WITH);

    result->stmt_with.stmts = stmts;
    result->stmt_with.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_break() {
    Resolved_Stmt *result = resolved_stmt_new(STMT_BREAK);

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_continue() {
    Resolved_Stmt *result = resolved_stmt_new(STMT_CONTINUE);

    return result;
}
/* }}} */
/* resolved_templ {{{ */
struct Resolved_Templ {
    char *name;
    Resolved_Stmt **stmts;
    size_t num_stmts;
    Map blocks;
};

internal_proc Resolved_Templ *
resolved_templ(char *name) {
    Resolved_Templ *result = ALLOC_STRUCT(&resolve_arena, Resolved_Templ);

    result->name = name;
    result->stmts = 0;
    result->num_stmts = 0;
    result->blocks = {};

    return result;
}
/* }}} */
/* aufräumen block {{{ */
/* @AUFGABE: aufräumen */
internal_proc Resolved_Expr *
operand_new(Type *type, Val *val) {
    Resolved_Expr *result = ALLOC_STRUCT(&resolve_arena, Resolved_Expr);

    result->type      = type;
    result->val       = val;
    result->is_const  = false;
    result->is_lvalue = false;

    return result;
}

/* @AUFGABE: aufräumen */
internal_proc Resolved_Expr *
operand_rvalue(Type *type, Val *val = 0) {
    Resolved_Expr *result = operand_new(type, val);

    return result;
}

/* @AUFGABE: aufräumen */
internal_proc b32
unify_arithmetic_operands(Resolved_Expr *left, Resolved_Expr *right) {
    if ( type_is_arithmetic(left->type) && type_is_arithmetic(right->type) ) {
        Type *type  = arithmetic_result_type_table[left->type->kind][right->type->kind];
        left->type  = type;
        right->type = type;

        return true;
    }

    return false;
}

/* @AUFGABE: aufräumen */
internal_proc b32
unify_scalar_operands(Resolved_Expr *left, Resolved_Expr *right) {
    if ( type_is_scalar(left->type) && type_is_scalar(right->type) ) {
        Type *type  = scalar_result_type_table[left->type->kind][right->type->kind];

        if ( type == type_void ) {
            return false;
        }

        left->type  = type;
        right->type = type;

        return true;
    }

    return false;
}

/* @AUFGABE: aufräumen */
internal_proc Resolved_Expr *
eval_binary_op(Token_Kind op, Resolved_Expr *left, Resolved_Expr *right) {
    switch (op) {
        case T_PLUS: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(val_int(left->val) + val_int(right->val));
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(val_float(left->val) + val_float(right->val));
                    return left;
                } break;

                default: {
                    fatal(left->pos.name, left->pos.row, "nicht unterstützer datentyp");
                } break;
            }
        } break;

        case T_MINUS: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(val_int(left->val) - val_int(right->val));
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(val_float(left->val) - val_float(right->val));
                    return left;
                } break;

                default: {
                    fatal(left->pos.name, left->pos.row, "nicht unterstützer datentyp");
                } break;
            }
        } break;

        case T_MUL: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(val_int(left->val) * val_int(right->val));
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(val_float(left->val) * val_float(right->val));
                    return left;
                } break;

                default: {
                    fatal(left->pos.name, left->pos.row, "nicht unterstützer datentyp");
                } break;
            }
        } break;

        case T_DIV: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(val_int(left->val) / val_int(right->val));
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(val_float(left->val) / val_float(right->val));
                    return left;
                } break;

                default: {
                    fatal(left->pos.name, left->pos.row, "nicht unterstützer datentyp");
                } break;
            }
        } break;
    }

    return left;
}
/* }}} */
internal_proc Sym *
resolve_name(char *name) {
    Sym *result = sym_get(name);

    return result;
}

internal_proc Resolved_Stmt *
resolve_stmt(Stmt *stmt, Resolved_Templ *templ) {
    Resolved_Stmt *result = 0;
    Pos pos = stmt->pos;

    switch (stmt->kind) {
        case STMT_VAR: {
            Resolved_Expr *expr = resolve_expr(stmt->stmt_var.expr);
            Resolved_Expr *if_expr = 0;

            if ( stmt->stmt_var.if_expr ) {
                if_expr = resolve_expr(stmt->stmt_var.if_expr);
            }

            result = resolved_stmt_var(expr, if_expr);
        } break;

        case STMT_FOR: {
            scope_enter("for");

            Sym **vars = 0;
            size_t num_vars = 0;
            for ( int i = 0; i < stmt->stmt_for.num_vars; ++i ) {
                assert(stmt->stmt_for.vars[i]->kind == EXPR_NAME);
                Sym *sym = sym_push_var(stmt->stmt_for.vars[i]->expr_name.value, type_any, val_undefined());
                buf_push(vars, sym);
            }

            num_vars = buf_len(vars);
            Resolved_Expr *set = resolve_expr(stmt->stmt_for.set);

            /* loop variablen {{{ */
            Sym *loop = sym_push_var("loop", 0);
            Scope *scope = scope_enter();
            Type *type = type_dict(scope, TYPE_FLAGS_CONST|TYPE_FLAGS_CALLABLE);

            loop->scope = scope;
            loop->type  = type;

            Type_Field *any_type[] = { type_field("s", type_any) };

            Sym *loop_index     = sym_push_var("index",     type_int,  val_int(1));
            Sym *loop_index0    = sym_push_var("index0",    type_int,  val_int(0));
            Sym *loop_revindex  = sym_push_var("revindex",  type_int,  val_int(0));
            Sym *loop_revindex0 = sym_push_var("revindex0", type_int,  val_int(0));
            Sym *loop_first     = sym_push_var("first",     type_bool, val_bool(true));
            Sym *loop_last      = sym_push_var("last",      type_bool, val_bool(false));
            Sym *loop_length    = sym_push_var("length",    type_int,  val_int(0));
            Sym *loop_cycle     = sym_push_proc("cycle",    type_proc(any_type, 0, 0, proc_cycle));

            scope_leave();
            /* }}} */

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_for.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_for.stmts[i], templ));
            }

            Resolved_Stmt **else_stmts = 0;
            for ( int i = 0; i < stmt->stmt_for.num_else_stmts; ++i ) {
                buf_push(else_stmts, resolve_stmt(stmt->stmt_for.else_stmts[i], templ));
            }

            scope_leave();

            result = resolved_stmt_for(vars, num_vars, set, stmts, buf_len(stmts), else_stmts, buf_len(else_stmts),
                    loop_index, loop_index0, loop_revindex, loop_revindex0, loop_first, loop_last,
                    loop_length, loop_cycle);
        } break;

        case STMT_IF: {
            scope_enter();
            Resolved_Expr *expr = resolve_expr_cond(stmt->stmt_if.cond);

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_if.stmts[i], templ));
            }
            scope_leave();

            Resolved_Stmt **elseifs = 0;
            if ( stmt->stmt_if.num_elseifs ) {
                for ( int i = 0; i < stmt->stmt_if.num_elseifs; ++i ) {
                    scope_enter();
                    Stmt *elseif = stmt->stmt_if.elseifs[i];
                    Resolved_Expr *elseif_expr = resolve_expr_cond(elseif->stmt_if.cond);

                    Resolved_Stmt **elseif_stmts = 0;
                    for ( int j = 0; j < elseif->stmt_if.num_stmts; ++j ) {
                        Resolved_Stmt *resolved_elseif = resolve_stmt(elseif->stmt_if.stmts[j], templ);
                        buf_push(elseif_stmts, resolved_elseif);
                    }
                    scope_leave();

                    buf_push(elseifs, resolved_stmt_elseif(elseif_expr, elseif_stmts, buf_len(elseif_stmts)));
                }
            }

            Resolved_Stmt *else_resolved_stmt = 0;
            if ( stmt->stmt_if.else_stmt ) {
                scope_enter();
                Stmt *else_stmt = stmt->stmt_if.else_stmt;

                Resolved_Stmt **else_stmts = 0;
                for ( int i = 0; i < else_stmt->stmt_if.num_stmts; ++i ) {
                    buf_push(else_stmts, resolve_stmt(else_stmt->stmt_if.stmts[i], templ));
                }
                scope_leave();

                else_resolved_stmt = resolved_stmt_else(else_stmts, buf_len(else_stmts));
            }

            result = resolved_stmt_if(expr, stmts, buf_len(stmts), elseifs, buf_len(elseifs), else_resolved_stmt);
        } break;

        case STMT_BLOCK: {
            Scope *prev_scope = scope_set(&global_scope);

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_block.num_stmts; ++i ) {
                Resolved_Stmt *resolved_stmt = resolve_stmt(stmt->stmt_block.stmts[i], templ);
                if ( resolved_stmt ) {
                    buf_push(stmts, resolved_stmt);
                }
            }

            scope_set(prev_scope);
            Resolved_Stmt *parent_block = (Resolved_Stmt *)map_get(&global_blocks, stmt->stmt_block.name);
            result = resolved_stmt_block(stmt->stmt_block.name, stmts, buf_len(stmts), parent_block);

            if ( !parent_block ) {
                map_put(&global_blocks, result->stmt_block.name, result);
            } else {
                parent_block->stmt_block.child_block = result;
            }
        } break;

        case STMT_LIT: {
            result = resolved_stmt_lit(stmt->stmt_lit.value);
        } break;

        case STMT_RAW: {
            result = resolved_stmt_raw(stmt->stmt_raw.value);
        } break;

        case STMT_EXTENDS: {
            char *name = "super";
            if ( sym_invalid(sym_get(name)) ) {
                sym_push_proc(name, type_proc(0, 0, 0, proc_super));
            }

            Resolved_Expr *if_expr = 0;
            if ( stmt->stmt_extends.if_expr ) {
                if_expr = resolve_expr(stmt->stmt_extends.if_expr);
            }

            Resolved_Templ *prev_templ = current_templ;

            Resolved_Templ *t = resolve(stmt->stmt_extends.templ);
            Resolved_Templ *else_templ = 0;
            if ( stmt->stmt_extends.else_templ ) {
                else_templ = resolve(stmt->stmt_extends.else_templ);
            }

            current_templ = prev_templ;

            result = resolved_stmt_extends(stmt->stmt_extends.name, t, else_templ, if_expr);
        } break;

        case STMT_SET: {
            Resolved_Expr *expr = resolve_expr(stmt->stmt_set.expr);

            Resolved_Expr **names = 0;
            if ( stmt->stmt_set.num_names == 1 ) {
                Expr *name = stmt->stmt_set.names[0];
                Resolved_Expr *resolved_name = resolve_expr(name);

                if ( sym_invalid(resolved_name->sym) && name->kind == EXPR_NAME ) {
                    resolved_name->sym = sym_push_var(name->expr_name.value, expr->type, val_undefined());
                    resolved_name->val = resolved_name->sym->val;
                } else if ( !sym_invalid(resolved_name->sym) ) {
                    resolved_name->sym->type = expr->type;
                    resolved_name->type = expr->type;
                }

                buf_push(names, resolved_name);
            } else {
                for ( int i = 0; i < stmt->stmt_set.num_names; ++i ) {
                    Expr *name = stmt->stmt_set.names[i];
                    Resolved_Expr *resolved_name = resolve_expr(name);

                    if ( sym_invalid(resolved_name->sym) && name->kind == EXPR_NAME ) {
                        resolved_name->sym = sym_push_var(name->expr_name.value, expr->type, val_undefined());
                        resolved_name->val = resolved_name->sym->val;
                    } else if ( !sym_invalid(resolved_name->sym) ) {
                        resolved_name->sym->type = expr->type;
                        resolved_name->type = expr->type;
                    }

                    buf_push(names, resolved_name);
                }
            }

            result = resolved_stmt_set(names, buf_len(names), expr);
        } break;

        case STMT_FILTER: {
            Resolved_Expr **filter = 0;
            for ( int i = 0; i < stmt->stmt_filter.num_filter; ++i ) {
                buf_push(filter, resolve_filter(stmt->stmt_filter.filter[i]));
            }

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_filter.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_filter.stmts[i], templ));
            }

            result = resolved_stmt_filter(filter, buf_len(filter), stmts, buf_len(stmts));
        } break;

        case STMT_INCLUDE: {
            Resolved_Templ **t = 0;
            Resolved_Templ *prev_templ = current_templ;

            for ( int i = 0; i < stmt->stmt_include.num_templ; ++i ) {
                buf_push(t, resolve(stmt->stmt_include.templ[i], stmt->stmt_include.with_context));
            }

            current_templ = prev_templ;
            result = resolved_stmt_include(t, buf_len(t));
        } break;

        case STMT_MACRO: {
            Type_Field **params = 0;
            for ( int i = 0; i < stmt->stmt_macro.num_params; ++i ) {
                Param *param = stmt->stmt_macro.params[i];

                Val *default_value = 0;
                if ( param->default_value ) {
                    Resolved_Expr *t = resolve_expr(param->default_value);
                    default_value = t->val;
                }

                buf_push(params, type_field(param->name, type_any, default_value));
            }

            Type *type = type_macro(params, buf_len(params), 0);

            char *macro_name = ( stmt->stmt_macro.alias ) ? stmt->stmt_macro.alias : stmt->stmt_macro.name;
            Sym *sym = sym_push_proc(macro_name, type);

            sym->scope = scope_enter(macro_name);

            Val **param_names = 0;
            for ( int i = 0; i < buf_len(params); ++i ) {
                params[i]->sym = sym_push_var(params[i]->name, params[i]->type);
                buf_push(param_names, val_str(params[i]->name));
            }
            size_t num_param_names = buf_len(param_names);

            /* macro variablen {{{ */
            sym_push_var("name",      type_str, val_str(macro_name));
            sym_push_var("arguments", type_tuple(num_param_names), val_tuple(param_names, num_param_names));
            sym_push_var("varargs",   type_list, val_undefined());
            /* }}} */

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_macro.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_macro.stmts[i], templ));
            }

            type->type_macro.stmts = stmts;
            type->type_macro.num_stmts = buf_len(stmts);

            scope_leave();

            result = resolved_stmt_macro(sym, type);
        } break;

        case STMT_IMPORT: {
            Sym *sym = sym_push_module(stmt->stmt_import.name, type_module(stmt->stmt_import.name, 0));

            Resolved_Templ *prev_templ = current_templ;
            Scope *scope = scope_enter(stmt->stmt_import.name);
            Resolved_Templ *t = resolve(stmt->stmt_import.templ);
            scope_leave();
            current_templ = prev_templ;

            sym->type->type_module.scope = scope;
            result = resolved_stmt_import(sym, t->stmts, t->num_stmts);
        } break;

        case STMT_FROM_IMPORT: {
            Parsed_Templ *t = stmt->stmt_from_import.templ;
            Resolved_Stmt **stmts = 0;

            for ( int i = 0; i < t->num_stmts; ++i ) {
                Stmt *parsed_stmt = t->stmts[i];

                for ( int j = 0; j < stmt->stmt_from_import.num_syms; ++j ) {
                    Imported_Sym *import_sym = stmt->stmt_from_import.syms[j];

                    if ( parsed_stmt->kind == STMT_MACRO ) {
                        if ( import_sym->name == parsed_stmt->stmt_macro.name ) {
                            parsed_stmt->stmt_macro.alias = import_sym->alias;
                            resolve_stmt(parsed_stmt, templ);
                        }
                    } else if ( parsed_stmt->kind == STMT_LIT ) {
                        /* nichts tun */
                    } else {
                        assert(parsed_stmt->kind == STMT_SET);
                        for ( int k = 0; k < parsed_stmt->stmt_set.num_names; ++k ) {
                            Expr *name = parsed_stmt->stmt_set.names[k];
                            assert(name->kind == EXPR_NAME);

                            if ( import_sym->name == name->expr_name.value ) {
                                parsed_stmt->stmt_macro.alias = import_sym->alias;
                                buf_push(stmts, resolve_stmt(parsed_stmt, templ));
                            }
                        }
                    }
                }
            }

            result = resolved_stmt_from_import(stmts, buf_len(stmts));
        } break;

        case STMT_WITH: {
            scope_enter();

            /* @INFO: zuerst müssen die ausdrücke der argumente aufgelöst werden
             *        BEVOR die argumente als symbole in den scope gepusht werden,
             *        da in diesem bereich die argumente noch keine gültigkeit haben.
             *
             *        https://jinja.palletsprojects.com/en/2.10.x/templates/#with-statement
             */
            Resolved_Arg **args = 0;
            for ( int i = 0; i < stmt->stmt_with.num_args; ++i ) {
                Arg *arg = stmt->stmt_with.args[i];
                Resolved_Expr *expr = resolve_expr(arg->expr);
                buf_push(args, resolved_arg(arg->pos, arg->name, expr->type, expr->val));
            }

            /* @INFO: hier werden die symbole der argumente tatsächlich veröffentlicht */
            for ( int i = 0; i < buf_len(args); ++i ) {
                Resolved_Arg *arg = args[i];
                sym_push_var(arg->name, arg->type, arg->val);
            }

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_with.num_stmts; ++i ) {
                Resolved_Stmt *resolved_stmt = resolve_stmt(stmt->stmt_with.stmts[i], templ);
                buf_push(stmts, resolved_stmt);
            }

            scope_leave();

            result = resolved_stmt_with(stmts, buf_len(stmts));
        } break;

        case STMT_BREAK: {
            result = resolved_stmt_break();
        } break;

        case STMT_CONTINUE: {
            result = resolved_stmt_continue();
        } break;

        case STMT_ENDWITH:
        case STMT_ENDIF:
        case STMT_ENDBLOCK:
        case STMT_ENDFILTER:
        case STMT_ENDFOR: {
        } break;

        default: {
            illegal_path();
            return result;
        } break;
    }

    if ( result ) {
        result->pos = pos;
    }

    return result;
}

internal_proc Resolved_Expr *
resolve_expr_cond(Expr *expr) {
    Resolved_Expr *result = resolve_expr(expr);

    /* @AUFGABE: int, str akzeptieren */
    if ( result->type != type_bool ) {
        fatal(expr->pos.name, expr->pos.row, "boolischen ausdruck erwartet");
    }

    return result;
}

internal_proc Resolved_Expr *
resolve_expr_call(Expr *expr, Scope *name_scope = current_scope) {
    /* @INFO: das symbol für die expr könnte in anderen scopes liegen,
     *        wie z. b. bei filtern und testern der fall ist. deshalb kann
     *        und wird ein scope übergeben um nach dem symbol in anderen
     *        scopes zu suchen
     */
    Scope *prev_scope = scope_set(name_scope);
    Resolved_Expr *call_expr = resolve_expr(expr->expr_call.expr);
    scope_set(prev_scope);

    Type *type = call_expr->type;

    if ( !type_is_callable(type) ) {
        fatal(call_expr->pos.name, call_expr->pos.row, "aufruf einer nicht-prozedur");
    }

    /* benamte argumente */
    Map *nargs = (Map *)xcalloc(1, sizeof(Map));
    /* benamte argumente, die nicht im typen definiert wurden */
    Resolved_Arg **kwargs = 0;
    /* mehr übergebener argumente als vom typ vorgegeben */
    Resolved_Arg **varargs = 0;
    b32 must_be_named = false;

    /* @INFO: alle übergebenen argumente durchgehen */
    for ( int i = 0; i < expr->expr_call.num_args; ++i ) {
        Arg *arg = expr->expr_call.args[i];

        if ( must_be_named && !arg->name ) {
            fatal(expr->pos.name, expr->pos.row, "nach benamten parameter müssen alle folgende parameter benamt sein");
        }

        char *name = 0;
        if ( arg->name ) {
            name = arg->name;
        }

        Resolved_Expr *arg_expr = resolve_expr(arg->expr);

        /* @INFO: wenn benamtes argument */
        if ( name ) {
            must_be_named = true;

            Type_Field *matching_param = 0;
            for ( int j = 0; j < type->type_proc.num_params; ++j ) {
                Type_Field *param = type->type_proc.params[j];
                if ( name == param->name ) {
                    matching_param = param;
                    break;
                }
            }

            if ( matching_param ) {
                map_put(nargs, name, resolved_arg(expr->pos, name, arg_expr->type, arg_expr->val));
            } else {
                Resolved_Arg *kwarg = resolved_arg(expr->pos, name, arg_expr->type, arg_expr->val);
                buf_push(kwargs, kwarg);
            }

        /* @INFO: wenn unbenamtes argument */
        } else {
            /* @INFO: mehr positionsargumente als im typ angegeben kommen
                *        in die varargs sammlung
                */
            if ( i >= type->type_proc.num_params ) {
                buf_push(varargs, resolved_arg(expr->pos, 0, arg_expr->type, arg_expr->val));

            /* @INFO: namen für positionsargument aus dem typ ermitteln */
            } else {
                Type_Field *param = 0;
                size_t num_non_default_params = 0;
                for ( int j = 0; j < type->type_proc.num_params; ++j ) {
                    if ( val_is_undefined(type->type_proc.params[j]->default_value) ) {
                        ++num_non_default_params;
                    }
                }

                for ( int j = 0; j < type->type_proc.num_params; ++j ) {
                    /* @INFO: ersten parameter ohne standardwert ermitteln und diesen nehmen, falls
                        *        er nicht bereits in der nargs map eingetragen ist.
                        */
                    size_t num_remaining_args = expr->expr_call.num_args - i;
                    if ( num_remaining_args > num_non_default_params ||
                            val_is_undefined(type->type_proc.params[j]->default_value) )
                    {
                        if ( !map_get(nargs, type->type_proc.params[j]->name) ) {
                            param = type->type_proc.params[j];
                            break;
                        }
                    }
                }

                if ( !param ) {
                    param = type->type_proc.params[i];
                }

                assert(param);
                Resolved_Arg *r_arg = resolved_arg(expr->pos, param->name, arg_expr->type, arg_expr->val);
                map_put(nargs, param->name, r_arg);
            }
        }
    }

    /* @INFO: alle typparameter durchgehen und für nicht gesetzte
        *        parameter standardwerte setzen
        */
    for ( int i = 0; i < type->type_proc.num_params; ++i ) {
        Type_Field *param = type->type_proc.params[i];

        if ( !map_get(nargs, param->name) ) {
            map_put(nargs, param->name, resolved_arg(expr->pos, param->name, param->type, param->default_value));
        }
    }

    return resolved_expr_call(call_expr, nargs, kwargs, buf_len(kwargs), varargs, buf_len(varargs), type);
}

internal_proc Resolved_Expr *
resolve_expr(Expr *expr) {
    Resolved_Expr *result = &resolved_expr_undefined;

    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
            result = resolved_expr_name(sym, sym->type, (sym != &sym_undefined) ? sym->val : val_undefined());
        } break;

        case EXPR_STR: {
            result = resolved_expr_str(type_str, val_str(expr->expr_str.value));
        } break;

        case EXPR_INT: {
            result = resolved_expr_int(type_int, val_int(expr->expr_int.value));
        } break;

        case EXPR_FLOAT: {
            result = resolved_expr_float(type_float, val_float(expr->expr_float.value));
        } break;

        case EXPR_BOOL: {
            result = resolved_expr_bool(type_bool, val_bool(expr->expr_bool.value));
        } break;

        case EXPR_PAREN: {
            result = resolved_expr_paren(resolve_expr(expr->expr_paren.expr));
        } break;

        case EXPR_UNARY: {
            result = resolved_expr_unary(expr->expr_unary.op, resolve_expr(expr->expr_unary.expr));
        } break;

        case EXPR_BINARY: {
            Resolved_Expr *left  = resolve_expr(expr->expr_binary.left);
            Resolved_Expr *right = resolve_expr(expr->expr_binary.right);
            Resolved_Expr *type = 0;

            if ( is_eql(expr->expr_binary.op) ) {
                /* @AUFGABE: aufräumen */
                unify_scalar_operands(left, right);
            } else {
                /* @AUFGABE: aufräumen */
                unify_arithmetic_operands(left, right);
            }

            if ( is_cmp(expr->expr_binary.op) ) {
                /* @AUFGABE: aufräumen */
                type = operand_rvalue(type_bool);
            } else if ( left->is_const && right->is_const ) {
                /* @AUFGABE: aufräumen */
                type = eval_binary_op(expr->expr_binary.op, left, right);
            } else {
                /* @AUFGABE: aufräumen */
                type = operand_rvalue(left->type);
            }

            result = resolved_expr_binary(expr->expr_binary.op, type->type, left, right);
        } break;

        case EXPR_TERNARY: {
            Resolved_Expr *left   = resolve_expr_cond(expr->expr_ternary.left);
            Resolved_Expr *middle = resolve_expr(expr->expr_ternary.middle);
            Resolved_Expr *right  = resolve_expr(expr->expr_ternary.right);

            if ( middle->type != right->type ) {
                fatal(left->pos.name, left->pos.row, "beide datentypen der zuweisung müssen gleich sein");
            }

            /* @AUFGABE: aufräumen */
            result = operand_rvalue(middle->type);
        } break;

        case EXPR_FIELD: {
            Resolved_Expr *base = resolve_expr(expr->expr_field.expr);

            assert(base->type);
            Type *type = base->type;

            if ( type->kind == TYPE_DICT || type->kind == TYPE_MACRO ) {
                assert(base->sym);

                Scope *prev_scope = scope_set(base->sym->scope);
                Sym *sym = resolve_name(expr->expr_field.field);
                assert(sym);
                scope_set(prev_scope);

                result = resolved_expr_field(base, sym, sym->type, sym->val);
            } else if ( type->kind == TYPE_MODULE ) {
                Scope *prev_scope = scope_set(type->type_module.scope);
                Sym *sym = resolve_name(expr->expr_field.field);
                assert(sym);
                scope_set(prev_scope);

                result = resolved_expr_field(base, sym, sym->type, sym->val);
            } else {
                result = resolved_expr_field(base, 0, &type_undefined, 0);
            }
        } break;

        case EXPR_RANGE: {
            Resolved_Expr *left  = resolve_expr(expr->expr_range.left);
            Resolved_Expr *right = resolve_expr(expr->expr_range.right);

            if ( type_is_arithmetic(left->type) && type_is_arithmetic(right->type) ) {
                unify_arithmetic_operands(left, right);

                if ( !type_is_int(left->type) ) {
                    fatal(left->pos.name, left->pos.row, "range typ muss vom typ int sein");
                }

                if ( !type_is_int(right->type) ) {
                    fatal(right->pos.name, right->pos.row, "range typ muss vom typ int sein");
                }

                int min = val_int(left->val);
                int max = val_int(right->val);

                result = resolved_expr_range(min, max);
            } else {
                assert(left->type->kind == TYPE_STR);
                assert(right->type->kind == TYPE_STR);
                assert(left->val && left->val->len == 1);

                result = resolved_expr_range((int)*val_str(left->val), (int)*val_str(right->val));
            }
        } break;

        case EXPR_CALL: {
            result = resolve_expr_call(expr);
        } break;

        case EXPR_SUBSCRIPT: {
            Resolved_Expr *resolved_expr  = resolve_expr(expr->expr_subscript.expr);
            Resolved_Expr *resolved_index = resolve_expr(expr->expr_subscript.index);

            result = resolved_expr_subscript(resolved_expr, resolved_index, resolved_expr->type);
            result->type->flags |= resolved_expr->type->flags & TYPE_FLAGS_CONST;
        } break;

        case EXPR_IS: {
            Resolved_Expr *operand = resolve_expr(expr->expr_is.operand);
            Resolved_Expr *tester  = resolve_tester(expr->expr_is.tester);

            assert(tester);

            Type *type = tester->type;
            assert(type->kind == TYPE_TEST);

            result = resolved_expr_is(operand, tester);
        } break;

        case EXPR_IF: {
            Resolved_Expr *cond = resolve_expr(expr->expr_if.cond);
            Resolved_Expr *else_expr = 0;

            if ( expr->expr_if.else_expr ) {
                else_expr = resolve_expr(expr->expr_if.else_expr);
            }

            result = resolved_expr_if(cond, else_expr);
        } break;

        case EXPR_LIST: {
            Resolved_Expr **index = 0;
            Val **vals = 0;

            for ( int i = 0; i < expr->expr_list.num_expr; ++i ) {
                Resolved_Expr *resolved_expr = resolve_expr(expr->expr_list.expr[i]);
                buf_push(index, resolved_expr);
                buf_push(vals, resolved_expr->val);
            }

            result = resolved_expr_list(index, buf_len(index), val_list(vals, buf_len(vals)));
        } break;

        case EXPR_TUPLE: {
            Resolved_Expr **exprs = 0;
            Val **vals = 0;

            for ( int i = 0; i < expr->expr_tuple.num_exprs; ++i ) {
                Resolved_Expr *rexpr = resolve_expr(expr->expr_tuple.exprs[i]);
                buf_push(vals, rexpr->val);
                buf_push(exprs, rexpr);
            }

            size_t num_elems = buf_len(vals);
            result = resolved_expr_tuple(exprs, buf_len(exprs), type_tuple(num_elems), val_tuple(vals, num_elems));
        } break;

        case EXPR_NOT: {
            Resolved_Expr *resolved_expr = resolve_expr(expr->expr_not.expr);

            result = resolved_expr_not(resolved_expr);
        } break;

        case EXPR_DICT: {
            Scope *scope = scope_new(0, "dict");
            Scope *prev_scope = scope_set(scope);

            char **keys = 0;
            Val **pairs = 0;
            for ( int i = 0; i < expr->expr_dict.num_pairs; ++i ) {
                Pair *pair = expr->expr_dict.pairs[i];

                Resolved_Expr *value = resolve_expr(pair->value);

                sym_push_var(pair->key, value->type, value->val);
                buf_push(pairs, val_pair(resolved_pair(expr->pos, val_str(pair->key), value->val)));
                buf_push(keys, pair->key);
            }

            scope_set(prev_scope);

            result = resolved_expr_dict(keys, buf_len(keys), type_dict(scope), val_dict(pairs, buf_len(pairs)));
        } break;

        default: {
            fatal(expr->pos.name, expr->pos.row, "nicht unterstützter ausdruck");
        } break;
    }

    Resolved_Expr **filters = 0;
    for ( int i = 0; i < expr->num_filters; ++i ) {
        buf_push(filters, resolve_filter(expr->filters[i]));
    }

    result->filters = filters;
    result->num_filters = buf_len(filters);
    result->pos = expr->pos;

    return result;
}

internal_proc Resolved_Expr *
resolve_tester(Expr *expr) {
    assert(expr->kind == EXPR_CALL);
    Resolved_Expr *result = resolve_expr_call(expr, &tester_scope);

    return result;
}

internal_proc Resolved_Expr *
resolve_filter(Expr *expr) {
    assert(expr->kind == EXPR_CALL);
    Resolved_Expr *result = resolve_expr_call(expr, &filter_scope);

    return result;
}

internal_proc void
resolve_add_block(char *name, Resolved_Stmt *block) {
}

internal_proc void
resolve_init_builtin_filter() {
    Type_Field *str_type[] = { type_field("s", type_str) };
    Type_Field *default_type[] = { type_field("s", type_str), type_field("boolean", type_bool, val_bool(False)) };
    Type_Field *trunc_type[] = {
        type_field("length", type_int, val_int(255)),
        type_field("killwords", type_bool, val_bool(False)),
        type_field("end", type_str, val_str("...")),
        type_field("leeway", type_int, val_int(0)),
    };

    sym_push_filter("abs",        type_filter(0,            0, type_str, filter_abs));
    sym_push_filter("capitalize", type_filter(0,            0, type_str, filter_capitalize));
    sym_push_filter("default",    type_filter(default_type, 2, type_str, filter_default));
    sym_push_filter("d",          type_filter(default_type, 2, type_str, filter_default));
    sym_push_filter("escape",     type_filter(0,            0, type_str, filter_escape));
    sym_push_filter("e",          type_filter(0,            0, type_str, filter_escape));
    sym_push_filter("format",     type_filter(0,            0, type_str, filter_format));
    sym_push_filter("truncate",   type_filter(trunc_type,   4, type_str, filter_truncate));
    sym_push_filter("upper",      type_filter(0,            0, type_str, filter_upper));
}

internal_proc void
resolve_init_builtin_testers() {
    Type_Field *int_type[]  = { type_field("s",    type_int) };
    Type_Field *any_type[]  = { type_field("s",    type_any) };

    sym_push_test("callable",    type_test(0,         0, test_callable));
    sym_push_test("defined",     type_test(0,         0, test_defined));
    sym_push_test("divisibleby", type_test(int_type,  1, test_divisibleby));
    sym_push_test("eq",          type_test(int_type,  1, test_eq));
    sym_push_test("escaped",     type_test(0,         0, test_escaped));
    sym_push_test("even",        type_test(0,         0, test_even));
    sym_push_test("ge",          type_test(int_type,  1, test_ge));
    sym_push_test("gt",          type_test(int_type,  1, test_gt));
    sym_push_test("in",          type_test(any_type,  1, test_in));
    sym_push_test("iterable",    type_test(0,         0, test_iterable));
    sym_push_test("le",          type_test(int_type,  1, test_le));
    sym_push_test("lt",          type_test(int_type,  1, test_lt));
    sym_push_test("mapping",     type_test(0,         0, test_mapping));
    sym_push_test("ne",          type_test(int_type,  1, test_ne));
    sym_push_test("none",        type_test(0,         0, test_none));
    sym_push_test("number",      type_test(0,         0, test_number));
    sym_push_test("odd",         type_test(0,         0, test_odd));
    sym_push_test("sameas",      type_test(any_type,  1, test_sameas));
    sym_push_test("sequence",    type_test(0,         0, test_sequence));
    sym_push_test("string",      type_test(0,         0, test_string));
    sym_push_test("undefined",   type_test(0,         0, test_undefined));
}

internal_proc void
resolve_init_builtin_procs() {
    Type_Field *range_args[]  = { type_field("start", type_int, val_int(0)), type_field("stop", type_int), type_field("step", type_int, val_int(1)) };
    Type_Field *lipsum_args[] = { type_field("n", type_int, val_int(5)), type_field("html", type_bool, val_bool(true)), type_field("min", type_int, val_int(20)), type_field("max", type_int, val_int(100)) };

    sym_push_sysproc("range",  type_proc(range_args,  3, 0, proc_range));
    sym_push_sysproc("lipsum", type_proc(lipsum_args, 4, 0, proc_lipsum));
}

internal_proc void
resolve_init_arenas() {
    arena_init(&resolve_arena, MB(100));
}

internal_proc void
resolve_init_builtin_types() {
    type_void  = type_new(TYPE_VOID);
    type_void->size = 0;

    type_bool  = type_new(TYPE_BOOL);
    type_bool->size = 1;

    type_int   = type_new(TYPE_INT);
    type_int->size = 4;

    type_float = type_new(TYPE_FLOAT);
    type_float->size = 4;

    type_str   = type_new(TYPE_STR);
    type_str->size = PTR_SIZE;
    type_str->flags = TYPE_FLAGS_ITERABLE;

    type_list  = type_new(TYPE_LIST);
    type_list->size = 0;
    type_list->flags = TYPE_FLAGS_ITERABLE;

    type_any   = type_new(TYPE_ANY);
    type_any->size = PTR_SIZE;

    arithmetic_result_type_table[TYPE_INT][TYPE_INT]     = type_int;
    arithmetic_result_type_table[TYPE_INT][TYPE_FLOAT]   = type_float;

    arithmetic_result_type_table[TYPE_FLOAT][TYPE_INT]   = type_float;
    arithmetic_result_type_table[TYPE_FLOAT][TYPE_FLOAT] = type_float;

    scalar_result_type_table[TYPE_INT][TYPE_BOOL]        = type_void;
    scalar_result_type_table[TYPE_INT][TYPE_INT]         = type_int;
    scalar_result_type_table[TYPE_INT][TYPE_FLOAT]       = type_float;

    scalar_result_type_table[TYPE_FLOAT][TYPE_BOOL]      = type_void;
    scalar_result_type_table[TYPE_FLOAT][TYPE_INT]       = type_float;
    scalar_result_type_table[TYPE_FLOAT][TYPE_FLOAT]     = type_float;

    scalar_result_type_table[TYPE_BOOL][TYPE_BOOL]      = type_bool;
    scalar_result_type_table[TYPE_BOOL][TYPE_INT]       = type_void;
    scalar_result_type_table[TYPE_BOOL][TYPE_FLOAT]     = type_void;
}

internal_proc void
resolve_init_scope() {
    filter_scope.name = "filter scope";
    system_scope.name = "system scope";
    tester_scope.name = "tester scope";
    global_scope.name = "global scope";

    global_scope.parent = &system_scope;
}

internal_proc void
resolve_reset() {
    sym_clear();
}

internal_proc void
resolve_init() {
    resolve_init_scope();
    resolve_init_arenas();
    resolve_init_builtin_types();
    resolve_init_builtin_procs();
    resolve_init_builtin_filter();
    resolve_init_builtin_testers();
}

internal_proc Resolved_Templ *
resolve(Parsed_Templ *parsed_templ, b32 with_context) {
    Resolved_Templ *result = resolved_templ(parsed_templ->name);
    current_templ = result;

    Scope *prev_scope = current_scope;
    if ( !with_context ) {
        current_scope = scope_new(&system_scope, parsed_templ->name);
    }

    for ( int i = 0; i < parsed_templ->num_stmts; ++i ) {
        Stmt *parsed_stmt = parsed_templ->stmts[i];

        Resolved_Stmt *stmt = ( parsed_stmt ) ? resolve_stmt(parsed_templ->stmts[i], result) : 0;

        if ( stmt ) {
            buf_push(result->stmts, stmt);
        }
    }

    result->num_stmts = buf_len(result->stmts);
    if ( !with_context ) {
        scope_set(prev_scope);
    }

    return result;
}

