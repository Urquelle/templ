struct Type;
struct Sym;
struct Val;
struct Operand;
struct Resolved_Stmt;
struct Resolved_Expr;
struct Resolved_Filter;
struct Resolved_Templ;

global_var Arena resolve_arena;

#define PROC_CALLBACK(name) Val * name(Resolved_Stmt *stmt)
typedef PROC_CALLBACK(Proc_Callback);
PROC_CALLBACK(super);

#define FILTER_CALLBACK(name) Val * name(Val *val, Resolved_Expr **params, size_t num_params)
typedef FILTER_CALLBACK(Filter_Callback);

#define TEST_CALLBACK(name) Val * name(Val *val, Resolved_Expr **args, size_t num_args)
typedef TEST_CALLBACK(Test_Callback);

internal_proc Resolved_Expr *   resolve_expr(Expr *expr);
internal_proc Resolved_Expr *   resolve_expr_cond(Expr *expr);
internal_proc Resolved_Filter * resolve_filter(Var_Filter *filter);
internal_proc Resolved_Stmt *   resolve_stmt(Stmt *stmt);
internal_proc Resolved_Templ *  resolve(Parsed_Templ *d);

/* scope {{{ */
struct Scope {
    char*  name;
    Scope* parent;
    Map    syms;
};

global_var Scope global_scope;
global_var Scope *current_scope = &global_scope;

internal_proc Scope *
scope_new(Scope *parent, char *name = NULL) {
    Scope *result = ALLOC_STRUCT(&resolve_arena, Scope);

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
    VAL_NONE,
    VAL_BOOL,
    VAL_CHAR,
    VAL_INT,
    VAL_FLOAT,
    VAL_STR,
    VAL_RANGE,
    VAL_FIELD,
    VAL_ARRAY,
};

struct Val {
    Val_Kind kind;
    size_t size;
    size_t len;
    void  *ptr;
};

global_var Val val_none;

internal_proc Val *
val_new(Val_Kind kind, size_t size) {
    Val *result = ALLOC_STRUCT(&resolve_arena, Val);

    result->kind = kind;
    result->size = size;
    result->ptr  = (void *)ALLOC_SIZE(&resolve_arena, size);

    return result;
}

internal_proc Val *
val_copy(Val *val) {
    if ( !val ) {
        return 0;
    }

    Val *result = val_new(val->kind, val->size);

    result->len = val->len;
    memcpy(result->ptr, val->ptr, val->size);

    return result;
}

internal_proc Val *
val_bool(b32 val) {
    Val *result = val_new(VAL_BOOL, sizeof(bool));

    *((b32 *)result->ptr) = val;

    return result;
}

internal_proc b32
val_bool(Val *val) {
    return *(b32 *)val->ptr;
}

internal_proc Val *
val_char(char val) {
    Val *result = val_new(VAL_CHAR, sizeof(char));

    *((char *)result->ptr) = val;

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
val_str(char *val) {
    Val *result = val_new(VAL_STR, sizeof(char*));

    result->len = strlen(val);
    *((char **)result->ptr) = val;

    return result;
}

internal_proc char *
val_str(Val *val) {
    return *(char **)val->ptr;
}

internal_proc Val *
val_range(int min, int max) {
    Val *result = val_new(VAL_RANGE, sizeof(int)*2);

    result->len = max - min;
    *((int *)result->ptr)   = min;
    *((int *)result->ptr+1) = max;

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

internal_proc Val *
val_array(Val **vals, size_t num_vals) {
    Val *result = val_new(VAL_ARRAY, sizeof(Val)*num_vals);

    result->ptr = (Val **)AST_DUP(vals);
    result->len = num_vals;

    return result;
}

internal_proc void
val_set(Val *val, s32 value) {
    assert(val->kind == VAL_INT);
    *((s32 *)val->ptr) = value;
}

internal_proc void
val_set(Val *val, bool value) {
    assert(val->kind == VAL_BOOL);
    *((b32 *)val->ptr) = value;
}

internal_proc void
val_set(Val *val, f32 value) {
    assert(val->kind == VAL_FLOAT);
    *((f32 *)val->ptr) = value;
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

internal_proc Val *
val_op(Token_Kind op, Val *val) {
    if ( op == T_MINUS ) {
        if ( val->kind == VAL_INT ) {
            return val_int( val_int(val)*-1 );
        }
    }

    return val;
}

global_var char to_char_buf[1000];
internal_proc char *
to_char(Val *val) {
    switch ( val->kind ) {
        case VAL_STR: {
            return val_str(val);
        } break;

        case VAL_INT: {
            sprintf(to_char_buf, "%d", val_int(val));
            return to_char_buf;
        } break;

        case VAL_FLOAT: {
            sprintf(to_char_buf, "%f", val_float(val));
            return to_char_buf;
        } break;

        default: {
            illegal_path();
            return "";
        } break;
    }
}

internal_proc Val *
val_item(Val *val, int idx) {
    switch ( val->kind ) {
        case VAL_RANGE: {
            return val_int(val_range0(val) + idx);
        } break;

        case VAL_ARRAY: {
            return *((Val **)val->ptr + idx);
        } break;

        default: {
            illegal_path();
        } break;
    }

    return 0;
}

internal_proc Val
operator*(Val left, Val right) {
    Val result = {};

    /* @AUFGABE: val_set implementieren */
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
        val_set(&result, val_int(&left) / val_int(&right));
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

internal_proc b32
operator==(Val left, Val right) {
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
/* }}} */

internal_proc Sym * sym_push_var(char *name, Type *type, Val *val = 0);

/* type {{{ */
struct Type_Field {
    char *name;
    Sym *sym;
    Type *type;
    s64   offset;
    Val  *default_value;
};

internal_proc Type_Field *
type_field(char *name, Type *type, Val *default_value = 0) {
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
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_PROC,
    TYPE_MACRO,
    TYPE_FILTER,
    TYPE_TEST,
    TYPE_MODULE,

    TYPE_COUNT,
};

struct Type {
    Type_Kind kind;
    s64 size;

    union {
        struct {
            Type_Field **fields;
            size_t num_fields;
            Scope *scope;
        } type_aggr;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
            Proc_Callback *callback;
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
        } type_filter;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
            Test_Callback *callback;
        } type_test;

        struct {
            Type *base;
            int num_elems;
        } type_array;

        struct {
            char *name;
            Scope *scope;
        } type_module;
    };
};

#define PTR_SIZE 8

global_var Type *type_void;
global_var Type *type_bool;
global_var Type *type_int;
global_var Type *type_float;
global_var Type *type_str;
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
type_struct(Type_Field **fields, size_t num_fields) {
    Type *result = type_new(TYPE_STRUCT);

    result->type_aggr.fields = (Type_Field **)AST_DUP(fields);
    result->type_aggr.num_fields = num_fields;
    result->type_aggr.scope = scope_enter();

    s64 offset = 0;
    for ( int i = 0; i < num_fields; ++i ) {
        Type_Field *field = fields[i];

        field->offset = offset;
        result->size += field->type->size;
        offset += field->type->size;

        sym_push_var(field->name, field->type);
    }
    scope_leave();

    return result;
}

internal_proc Type *
type_proc(Type_Field **params, size_t num_params, Type *ret, Proc_Callback *callback) {
    Type *result = type_new(TYPE_PROC);

    result->size = PTR_SIZE;
    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;
    result->type_proc.callback = callback;

    return result;
}

internal_proc Type *
type_macro(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_MACRO);

    result->size = PTR_SIZE;
    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;

    return result;
}

internal_proc Type *
type_filter(Type_Field **params, size_t num_params, Type *ret, Filter_Callback *callback) {
    Type *result = type_new(TYPE_FILTER);

    result->size = PTR_SIZE;
    result->type_filter.params = (Type_Field **)AST_DUP(params);
    result->type_filter.num_params = num_params;
    result->type_filter.ret = ret;
    result->type_filter.callback = callback;

    return result;
}

internal_proc Type *
type_test(Type_Field **params, size_t num_params, Test_Callback *callback) {
    Type *result = type_new(TYPE_TEST);

    result->size = PTR_SIZE;
    result->type_test.params = (Type_Field **)AST_DUP(params);
    result->type_test.num_params = num_params;
    result->type_test.ret = type_bool;
    result->type_test.callback = callback;

    return result;
}

internal_proc Type *
type_array(Type *base, int num_elems) {
    Type *result = type_new(TYPE_ARRAY);

    result->size = base->size * num_elems;
    result->type_array.base = base;
    result->type_array.num_elems = num_elems;

    return result;
}

internal_proc Type *
type_module(char *name, Scope *scope) {
    Type *result = type_new(TYPE_MODULE);

    result->type_module.name = name;
    result->type_module.scope = scope;

    return result;
}

internal_proc void
init_builtin_types() {
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

internal_proc b32
is_int(Type *type) {
    b32 result = (type->kind == TYPE_INT);

    return result;
}

internal_proc b32
is_arithmetic(Type *type) {
    b32 result = TYPE_INT <= type->kind && type->kind <= TYPE_FLOAT;

    return result;
}

internal_proc b32
is_scalar(Type *type) {
    b32 result = TYPE_BOOL <= type->kind && type->kind <= TYPE_FLOAT;

    return result;
}

internal_proc b32
is_callable(Type *type) {
    b32 result = ( type->kind == TYPE_PROC || type->kind == TYPE_FILTER || type->kind == TYPE_MACRO );

    return result;
}
/* }}} */
/* sym {{{ */
enum Sym_Kind {
    SYM_NONE,
    SYM_VAR,
    SYM_PROC,
    SYM_STRUCT,
    SYM_MODULE,
};

struct Sym {
    Sym_Kind kind;
    Scope *scope;
    char *name;
    Type *type;
    Val  *val;
};

internal_proc Sym *
sym_new(Sym_Kind kind, char *name, Type *type, Val *val = 0) {
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
    for ( Scope *it = current_scope; it; it = it->parent ) {
        Sym *sym = (Sym *)map_get(&it->syms, name);
        if ( sym ) {
            return sym;
        }
    }

    return 0;
}

internal_proc Sym *
sym_push(Sym_Kind kind, char *name, Type *type, Val *val = 0) {
    name = intern_str(name);

    Sym *sym = (Sym *)map_get(&current_scope->syms, name);
    if ( sym && sym->scope == current_scope ) {
        fatal("symbol existiert bereits");
    } else if ( sym ) {
        fatal("warnung: symbol wird überschattet");
    }

    Sym *result = sym_new(kind, name, type, val);
    map_put(&current_scope->syms, name, result);

    return result;
}

internal_proc Sym *
sym_push_filter(char *name, Type *type) {
    return sym_push(SYM_PROC, name, type);
}

internal_proc Sym *
sym_push_test(char *name, Type *type) {
    return sym_push(SYM_PROC, name, type);
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
    char *name;
    Val *val;
};

internal_proc Resolved_Arg *
resolved_arg_new(char *name, Val *val) {
    Resolved_Arg *result = ALLOC_STRUCT(&resolve_arena, Resolved_Arg);

    result->name = name;
    result->val = val;

    return result;
}
/* }}} */
/* resolved_expr {{{ */
struct Resolved_Expr {
    Expr_Kind kind;
    Type *type;
    Sym *sym;
    Val *val;
    Resolved_Stmt *stmt;

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
            s64 offset;
        } expr_field;

        struct {
            Resolved_Expr *expr;
            Resolved_Expr **index;
            size_t num_index;
        } expr_index;

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
            Resolved_Expr *expr;
            Resolved_Expr *test;
            Resolved_Expr **args;
            size_t num_args;
        } expr_is;

        struct {
            Resolved_Expr *expr;
            Resolved_Arg **args;
            size_t num_args;
        } expr_call;

        struct {
            Resolved_Expr *cond;
            Resolved_Expr *else_expr;
        } expr_if;

        struct {
            Resolved_Expr **expr;
            size_t num_expr;
        } expr_array_lit;
    };
};

internal_proc Resolved_Expr *
resolved_expr_new(Expr_Kind kind, Type *type = 0) {
    Resolved_Expr *result = ALLOC_STRUCT(&resolve_arena, Resolved_Expr);

    result->is_lvalue = false;
    result->is_const = false;
    result->type = type;
    result->kind = kind;

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
resolved_expr_field(Resolved_Expr *base, Sym *sym, s64 offset, Type *type) {
    Resolved_Expr *result = resolved_expr_new(EXPR_FIELD, type);

    result->is_lvalue = true;
    result->sym = sym;
    result->expr_field.base = base;
    result->expr_field.offset = offset;

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
resolved_expr_is(Resolved_Expr *expr, Resolved_Expr *test, Resolved_Expr **args, size_t num_args) {
    Resolved_Expr *result = resolved_expr_new(EXPR_IS, type_bool);

    result->expr_is.expr = expr;
    result->expr_is.test = test;
    result->expr_is.args = args;
    result->expr_is.num_args = num_args;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_call(Resolved_Expr *expr, Resolved_Arg **args, size_t num_args, Type *type) {
    Resolved_Expr *result = resolved_expr_new(EXPR_CALL, type);

    result->expr_call.expr = expr;
    result->expr_call.args = (Resolved_Arg **)AST_DUP(args);
    result->expr_call.num_args = num_args;

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
resolved_expr_index(Resolved_Expr *expr, Resolved_Expr **index, size_t num_index) {
    Resolved_Expr *result = resolved_expr_new(EXPR_INDEX);

    result->expr_index.expr = expr;
    result->expr_index.index = index;
    result->expr_index.num_index = num_index;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_array_lit(Resolved_Expr **expr, size_t num_expr, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_ARRAY_LIT);

    result->val = val;
    result->expr_array_lit.expr = expr;
    result->expr_array_lit.num_expr = num_expr;

    return result;
}
/* }}} */

internal_proc Resolved_Expr *
operand_new(Type *type, Val *val) {
    Resolved_Expr *result = ALLOC_STRUCT(&resolve_arena, Resolved_Expr);

    result->type      = type;
    result->val       = val;
    result->is_const  = false;
    result->is_lvalue = false;

    return result;
}

internal_proc Resolved_Expr *
operand_lvalue(Type *type, Sym *sym = NULL, Val *val = 0) {
    Resolved_Expr *result = operand_new(type, val);

    result->is_lvalue = true;
    result->sym = sym;

    return result;
}

internal_proc Resolved_Expr *
operand_rvalue(Type *type, Val *val = 0) {
    Resolved_Expr *result = operand_new(type, val);

    return result;
}

internal_proc Resolved_Expr *
operand_const(Type *type, Val *val) {
    Resolved_Expr *result = operand_new(type, val);

    result->is_const = true;

    return result;
}

/* resolved_stmt {{{ */
struct Resolved_Stmt {
    Stmt_Kind kind;

    Resolved_Stmt *block;
    Resolved_Stmt *super;

    union {
        struct {
            Resolved_Expr *expr;
            Resolved_Filter **filter;
            size_t num_filter;
            Resolved_Expr *if_expr;
        } stmt_var;

        struct {
            Sym *it;
            Resolved_Expr *expr;
            Resolved_Stmt **stmts;
            size_t num_stmts;
            Resolved_Stmt **else_stmts;
            size_t num_else_stmts;
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
            char *name;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_block;

        struct {
            char *lit;
        } stmt_lit;

        struct {
            Sym *sym;
            Resolved_Expr *expr;
        } stmt_set;

        struct {
            Resolved_Filter **filter;
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
        } stmt_module;
    };
};

internal_proc Resolved_Stmt *
resolved_stmt_new(Stmt_Kind kind) {
    Resolved_Stmt *result = ALLOC_STRUCT(&resolve_arena, Resolved_Stmt);

    result->kind = kind;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_var(Resolved_Expr *expr, Resolved_Filter **filter,
        size_t num_filter, Resolved_Expr *if_expr)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_VAR);

    result->stmt_var.expr       = expr;
    result->stmt_var.filter     = filter;
    result->stmt_var.num_filter = num_filter;
    result->stmt_var.if_expr    = if_expr;

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
resolved_stmt_for(Sym *it, Resolved_Expr *expr, Resolved_Stmt **stmts,
        size_t num_stmts, Resolved_Stmt **else_stmts, size_t num_else_stmts)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_FOR);

    result->stmt_for.it = it;
    result->stmt_for.expr = expr;
    result->stmt_for.stmts = stmts;
    result->stmt_for.num_stmts = num_stmts;
    result->stmt_for.else_stmts = else_stmts;
    result->stmt_for.num_else_stmts = num_else_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_lit(char *val) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_LIT);

    result->stmt_lit.lit = val;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_set(Sym *sym, Resolved_Expr *expr) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_SET);

    result->stmt_set.sym = sym;
    result->stmt_set.expr = expr;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_block(char *name, Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_BLOCK);

    result->stmt_block.name = name;
    result->stmt_block.stmts = stmts;
    result->stmt_block.num_stmts = num_stmts;

    for ( int i = 0; i < num_stmts; ++i ) {
        stmts[i]->block = result;
    }

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_filter(Resolved_Filter **filter, size_t num_filter,
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
resolved_stmt_import(Sym *sym) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_IMPORT);

    result->stmt_module.sym = sym;

    return result;
}
/* }}} */

struct Resolved_Filter {
    Sym *sym;
    Type *type;
    Resolved_Expr **args;
    size_t num_args;
    Filter_Callback *proc;
};

internal_proc Resolved_Filter *
resolved_filter_new() {
    Resolved_Filter *result = ALLOC_STRUCT(&resolve_arena, Resolved_Filter);

    return result;
}

internal_proc Resolved_Filter *
resolved_filter(Sym *sym, Type *type, Resolved_Expr **args, size_t num_args, Filter_Callback *proc) {
    Resolved_Filter *result = resolved_filter_new();

    result->sym = sym;
    result->type = type;
    result->args = args;
    result->num_args = num_args;
    result->proc = proc;

    return result;
}

internal_proc b32
unify_arithmetic_operands(Resolved_Expr *left, Resolved_Expr *right) {
    if ( is_arithmetic(left->type) && is_arithmetic(right->type) ) {
        Type *type  = arithmetic_result_type_table[left->type->kind][right->type->kind];
        left->type  = type;
        right->type = type;

        return true;
    }

    return false;
}

internal_proc b32
convert_operand(Resolved_Expr *op, Type *dest_type) {
    b32 result = false;

    if ( op->type == dest_type ) {
        return true;
    }

    if ( op->type->kind == TYPE_INT ) {
        if ( dest_type->kind == TYPE_FLOAT ) {
            op->type = type_float;

            return true;
        } else if ( dest_type->kind == TYPE_BOOL ) {
            return true;
        }
    }

    return result;
}

internal_proc b32
unify_scalar_operands(Resolved_Expr *left, Resolved_Expr *right) {
    if ( is_scalar(left->type) && is_scalar(right->type) ) {
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

/* filter {{{ */
internal_proc FILTER_CALLBACK(filter_abs) {
    assert(val->kind == VAL_INT);
    s32 i = abs(val_int(val));

    return val_int(i);
}

internal_proc FILTER_CALLBACK(filter_upper) {
    assert(val->kind == VAL_STR);
    char *str = val_str(val);
    char *result = "";

    for ( int i = 0; i < strlen(str); ++i ) {
        result = strf("%s%c", result, toupper(str[i]));
    }

    return val_str(result);
}

internal_proc FILTER_CALLBACK(filter_escape) {
    return val_str("<!-- @AUFGABE: implementiere filter escape -->");
}

internal_proc FILTER_CALLBACK(filter_truncate) {
    return val_str("<!-- @AUFGABE: implementiere filter truncate -->");
}

internal_proc void
init_builtin_filter() {
    Type_Field *str_type[] = { type_field("s", type_str) };
    Type_Field *int_type[] = { type_field("s", type_int) };

    sym_push_filter("abs",    type_filter(int_type, 1, type_str, filter_abs));
    sym_push_filter("upper",  type_filter(str_type, 1, type_str, filter_upper));
    sym_push_filter("escape", type_filter(str_type, 1, type_str, filter_escape));

    Type_Field *trunc_type[] = {
        type_field("s", type_str),
        type_field("length", type_int, val_int(255)),
        type_field("end", type_str, val_str("...")),
        type_field("killwords", type_bool, val_bool(false)),
        type_field("leeway", type_int, val_int(0)),
    };
    sym_push_filter("truncate", type_filter(trunc_type, 5, type_str, filter_truncate));
}
/* }}} */
/* tests {{{ */
TEST_CALLBACK(test_callable) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_defined) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_divisibleby) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_eq) {
    assert(val);
    assert(num_args == 1);

    Val *result = val_bool(*val == *args[0]->val);

    return result;
}

TEST_CALLBACK(test_escaped) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_even) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_ge) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_gt) {
    implement_me();
    return false;
}

TEST_CALLBACK(test_in) {
    implement_me();
    return false;
}

internal_proc void
init_builtin_tests() {
    Type_Field *str_type[]  = { type_field("s", type_str) };
    Type_Field *int_type[]  = { type_field("s", type_int) };
    Type_Field *int2_type[] = { type_field("left", type_int), type_field("right", type_int) };

    sym_push_test("callable", type_test(str_type, 1, test_callable));
    sym_push_test("defined", type_test(str_type, 1, test_defined));
    sym_push_test("divisibleby", type_test(int2_type, 2, test_divisibleby));
    sym_push_test("eq", type_test(int2_type, 2, test_eq));
    sym_push_test("escaped", type_test(str_type, 1, test_escaped));
    sym_push_test("even", type_test(int_type, 1, test_even));
    sym_push_test("ge", type_test(int2_type, 2, test_ge));
    sym_push_test("gt", type_test(int2_type, 2, test_gt));
}
/* }}} */

struct Resolved_Templ {
    char *name;
    Resolved_Stmt **stmts;
    size_t num_stmts;
};

internal_proc Resolved_Templ *
resolved_templ_new() {
    Resolved_Templ *result = ALLOC_STRUCT(&resolve_arena, Resolved_Templ);

    result->name = 0;
    result->stmts = 0;
    result->num_stmts = 0;

    return result;
}

internal_proc Sym *
resolve_name(char *name) {
    Sym *result = sym_get(name);

    return result;
}

internal_proc void
resolve_stmts(Stmt **stmts, size_t num_stmts) {
    for ( int i = 0; i < num_stmts; ++i ) {
        resolve_stmt(stmts[i]);
    }
}

internal_proc Resolved_Stmt *
resolve_stmt(Stmt *stmt) {
    Resolved_Stmt *result = 0;

    switch (stmt->kind) {
        case STMT_VAR: {
            Resolved_Filter **res_filter = 0;
            for ( int i = 0; i < stmt->stmt_var.num_filter; ++i ) {
                Var_Filter filter = stmt->stmt_var.filter[i];
                buf_push(res_filter, resolve_filter(&filter));
            }

            Resolved_Expr *expr = resolve_expr(stmt->stmt_var.expr);
            Resolved_Expr *if_expr = 0;
            if ( stmt->stmt_var.if_expr ) {
                if_expr = resolve_expr(stmt->stmt_var.if_expr);
            }

            result = resolved_stmt_var(expr, res_filter, buf_len(res_filter), if_expr);
            expr->stmt = result;
        } break;

        case STMT_FOR: {
            scope_enter();

            Resolved_Expr *expr = resolve_expr(stmt->stmt_for.cond);
            Sym *it = sym_push_var(stmt->stmt_for.it, expr->type, val_int(0));

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_for.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_for.stmts[i]));
            }

            Resolved_Stmt **else_stmts = 0;
            for ( int i = 0; i < stmt->stmt_for.num_else_stmts; ++i ) {
                buf_push(else_stmts, resolve_stmt(stmt->stmt_for.else_stmts[i]));
            }

            scope_leave();

            result = resolved_stmt_for(it, expr, stmts, buf_len(stmts), else_stmts, buf_len(else_stmts));
        } break;

        case STMT_IF: {
            scope_enter();
            Resolved_Expr *expr = resolve_expr_cond(stmt->stmt_if.cond);

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_if.stmts[i]));
            }
            scope_leave();

            Resolved_Stmt **elseifs = 0;
            if ( stmt->stmt_if.num_elseifs ) {
                for ( int i = 0; i < stmt->stmt_if.num_elseifs; ++i ) {
                    scope_enter();
                    Stmt *elseif = stmt->stmt_if.elseifs[i];
                    Resolved_Expr *elseif_expr = resolve_expr_cond(elseif->stmt_if.cond);

                    Resolved_Stmt **elseif_stmts = 0;
                    for ( int j = 0; i < elseif->stmt_if.num_stmts; ++j ) {
                        buf_push(elseif_stmts, resolve_stmt(elseif->stmt_if.stmts[j]));
                    }
                    scope_leave();

                    buf_push(elseifs, resolved_stmt_elseif(elseif_expr, elseif_stmts, buf_len(elseif_stmts)));
                }
            }

            Resolved_Stmt *else_resolved_stmt = 0;
            Resolved_Stmt **else_stmts = 0;
            if ( stmt->stmt_if.else_stmt ) {
                scope_enter();
                Stmt *else_stmt = stmt->stmt_if.else_stmt;

                for ( int i = 0; i < else_stmt->stmt_if.num_stmts; ++i ) {
                    buf_push(else_stmts, resolve_stmt(else_stmt->stmt_if.stmts[i]));
                }
                scope_leave();

                else_resolved_stmt = resolved_stmt_else(else_stmts, buf_len(else_stmts));
            }

            result = resolved_stmt_if(expr, stmts, buf_len(stmts), elseifs, buf_len(elseifs), else_resolved_stmt);
        } break;

        case STMT_BLOCK: {
            scope_enter(stmt->stmt_block.name);

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_block.num_stmts; ++i ) {
                Resolved_Stmt *resolved_stmt = resolve_stmt(stmt->stmt_block.stmts[i]);
                if ( resolved_stmt ) {
                    buf_push(stmts, resolved_stmt);
                }
            }

            scope_leave();

            result = resolved_stmt_block(stmt->stmt_block.name, stmts, buf_len(stmts));
        } break;

        case STMT_ENDIF:
        case STMT_ENDBLOCK:
        case STMT_ENDFILTER:
        case STMT_ENDFOR: {
        } break;

        case STMT_LIT: {
            result = resolved_stmt_lit(stmt->stmt_lit.value);
        } break;

        case STMT_EXTENDS: {
            if ( !sym_get(intern_str("super")) ) {
                sym_push_proc("super", type_proc(0, 0, 0, super));
            }

            Resolved_Expr *if_expr = 0;
            if ( stmt->stmt_extends.if_expr ) {
                if_expr = resolve_expr(stmt->stmt_extends.if_expr);
            }

            Resolved_Templ *templ = resolve(stmt->stmt_extends.templ);
            Resolved_Templ *else_templ = 0;
            if ( stmt->stmt_extends.else_templ ) {
                else_templ = resolve(stmt->stmt_extends.else_templ);
            }

            result = resolved_stmt_extends(stmt->stmt_extends.name, templ, else_templ, if_expr);
        } break;

        case STMT_SET: {
            Sym *sym = resolve_name(stmt->stmt_set.name);
            Resolved_Expr *expr = resolve_expr(stmt->stmt_set.expr);

            if ( !sym ) {
                sym = sym_push_var(stmt->stmt_set.name, expr->type, val_copy(expr->val));
            } else {
                if ( !convert_operand(expr, sym->type) ) {
                    fatal("datentyp des operanden passt nicht");
                }
            }

            result = resolved_stmt_set(sym, expr);
        } break;

        case STMT_FILTER: {
            Resolved_Filter **filter = 0;
            for ( int i = 0; i < stmt->stmt_filter.num_filter; ++i ) {
                buf_push(filter, resolve_filter(&stmt->stmt_filter.filter[i]));
            }

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_filter.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_filter.stmts[i]));
            }

            result = resolved_stmt_filter(filter, buf_len(filter), stmts, buf_len(stmts));
        } break;

        case STMT_INCLUDE: {
            Resolved_Templ **templ = 0;
            for ( int i = 0; i < stmt->stmt_include.num_templ; ++i ) {
                buf_push(templ, resolve(stmt->stmt_include.templ[i]));
            }

            result = resolved_stmt_include(templ, buf_len(templ));
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

            Scope *scope = scope_enter();

            for ( int i = 0; i < buf_len(params); ++i ) {
                params[i]->sym = sym_push_var(params[i]->name, params[i]->type);
            }

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_macro.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_macro.stmts[i]));
            }

            type->type_macro.stmts = stmts;
            type->type_macro.num_stmts = buf_len(stmts);

            scope_leave();

            result = resolved_stmt_macro(sym, type);
        } break;

        case STMT_IMPORT: {
            Sym *sym = sym_push_module(stmt->stmt_import.name, type_module(stmt->stmt_import.name, 0));

            Scope *scope = scope_enter(stmt->stmt_import.name);
            resolve(stmt->stmt_import.templ);
            scope_leave();

            sym->type->type_module.scope = scope;
            result = resolved_stmt_import(sym);
        } break;

        case STMT_FROM_IMPORT: {
            Parsed_Templ *templ = stmt->stmt_from_import.templ;
            for ( int i = 0; i < templ->num_stmts; ++i ) {
                Stmt *parsed_stmt = templ->stmts[i];

                for ( int j = 0; j < stmt->stmt_from_import.num_syms; ++j ) {
                    Imported_Sym *import_sym = stmt->stmt_from_import.syms[j];

                    if ( parsed_stmt->kind == STMT_MACRO ) {
                        if ( import_sym->name == parsed_stmt->stmt_macro.name ) {
                            parsed_stmt->stmt_macro.alias = import_sym->alias;
                            resolve_stmt(parsed_stmt);
                        }
                    } else {
                        assert(parsed_stmt->kind == STMT_SET);
                        if ( import_sym->name == parsed_stmt->stmt_set.name ) {
                            parsed_stmt->stmt_macro.alias = import_sym->alias;
                            resolve_stmt(parsed_stmt);
                        }
                    }
                }
            }
        } break;

        default: {
            illegal_path();
            return result;
        } break;
    }

    return result;
}

internal_proc Resolved_Expr *
resolve_expr_cond(Expr *expr) {
    Resolved_Expr *result = resolve_expr(expr);

    /* @AUFGABE: int, str akzeptieren */
    if ( result->type != type_bool ) {
        fatal("boolischen ausdruck erwartet");
    }

    return result;
}

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
                    fatal("nicht unterstützer datentyp");
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
                    fatal("nicht unterstützer datentyp");
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
                    fatal("nicht unterstützer datentyp");
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
                    fatal("nicht unterstützer datentyp");
                } break;
            }
        } break;
    }

    return left;
}


internal_proc s64
offset_from_base(Type *type, char *name) {
    s64 result = 0;
    for ( int i = 0; i < type->type_aggr.num_fields; ++i ) {
        Type_Field *field = type->type_aggr.fields[i];
        if ( field->name == name ) {
            return result;
        }
        result += field->type->size;
    }

    return result;
}

internal_proc Resolved_Expr *
resolve_expr(Expr *expr) {
    Resolved_Expr *result = 0;

    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
            if ( !sym ) {
                fatal("konnte symbol nicht auflösen");
            }

            result = resolved_expr_name(sym, sym->type, sym->val);
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
                unify_scalar_operands(left, right);
            } else {
                unify_arithmetic_operands(left, right);
            }

            if ( is_cmp(expr->expr_binary.op) ) {
                type = operand_rvalue(type_bool);
            } else if ( left->is_const && right->is_const ) {
                type = eval_binary_op(expr->expr_binary.op, left, right);
            } else {
                type = operand_rvalue(left->type);
            }

            result = resolved_expr_binary(expr->expr_binary.op, type->type, left, right);
        } break;

        case EXPR_TERNARY: {
            Resolved_Expr *left   = resolve_expr_cond(expr->expr_ternary.left);
            Resolved_Expr *middle = resolve_expr(expr->expr_ternary.middle);
            Resolved_Expr *right  = resolve_expr(expr->expr_ternary.right);

            if ( middle->type != right->type ) {
                fatal("beide datentypen der zuweisung müssen gleich sein");
            }

            result = operand_rvalue(middle->type);
        } break;

        case EXPR_FIELD: {
            Resolved_Expr *base = resolve_expr(expr->expr_field.expr);

            assert(base->type);
            Type *type = base->type;

            if ( type->kind == TYPE_STRUCT ) {
                Scope *prev_scope = scope_set(type->type_aggr.scope);
                Sym *sym = resolve_name(expr->expr_field.field);
                assert(sym);

                s64 offset = offset_from_base(type, sym->name);
                scope_set(prev_scope);

                result = resolved_expr_field(base, sym, offset, sym->type);
            } else {
                assert(type->kind == TYPE_MODULE);
                Scope *prev_scope = scope_set(type->type_module.scope);
                Sym *sym = resolve_name(expr->expr_field.field);
                assert(sym);

                scope_set(prev_scope);

                result = resolved_expr_field(base, sym, 0, sym->type);
            }
        } break;

        case EXPR_RANGE: {
            Resolved_Expr *left  = resolve_expr(expr->expr_range.left);
            Resolved_Expr *right = resolve_expr(expr->expr_range.right);

            if ( is_arithmetic(left->type) && is_arithmetic(right->type) ) {
                unify_arithmetic_operands(left, right);

                if ( !is_int(left->type) ) {
                    fatal("range typ muss vom typ int sein");
                }

                if ( !is_int(right->type) ) {
                    fatal("range typ muss vom typ int sein");
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
            Resolved_Expr *call_expr = resolve_expr(expr->expr_call.expr);
            Type *type = call_expr->type;

            if ( !is_callable(type) ) {
                fatal("aufruf einer nicht-prozedur");
            }

            if ( type->type_proc.num_params < expr->expr_call.num_args ) {
                fatal("zu viele argumente");
            }

            for ( int i = 0; i < type->type_proc.num_params; ++i ) {
                Type_Field *param = type->type_proc.params[i];

                if ( !param->default_value && (i >= expr->expr_call.num_args) ) {
                    fatal("zu wenige parameter übergeben");
                }
            }

            Resolved_Arg **args = 0;
            b32 must_be_named = false;
            char **params = 0;
            for ( int i = 0; i < expr->expr_call.num_args; ++i ) {
                Arg *arg = expr->expr_call.args[i];

                char *name = 0;
                if ( arg->name ) {
                    must_be_named = true;
                    name = arg->name;
                }

                if ( !arg->name && must_be_named ) {
                    fatal("nach benamten parameter müssen alle folgende parameter benamt sein");
                }

                b32 found = false;
                for ( int j = 0; j < type->type_proc.num_params; ++j ) {
                    Type_Field *param = type->type_proc.params[j];

                    if ( !arg->name || arg->name && arg->name == param->name ) {
                        found = true;

                        if ( !name ) {
                            name = param->name;
                        }
                    }
                }

                if ( !found ) {
                    fatal("kein argument mit der bezeichnung %s gefunden", arg->name);
                }

                for ( int j = 0; j < buf_len(params); ++j ) {
                    if ( name == params[j] ) {
                        fatal("parameter %s wurde bereits gesetzt", name);
                    }
                }

                buf_push(params, name);

                Resolved_Expr *arg_expr = resolve_expr(expr->expr_call.args[i]->expr);
                Resolved_Arg *resolved_arg = resolved_arg_new(name, arg_expr->val);
                buf_push(args, resolved_arg);
            }

            result = resolved_expr_call(call_expr, args, buf_len(args), type);
        } break;

        case EXPR_INDEX: {
            Resolved_Expr *resolved_expr = resolve_expr(expr->expr_index.expr);
            if (resolved_expr->type->kind != TYPE_ARRAY) {
                fatal("indizierung auf einem nicht-array");
            }

            Resolved_Expr **index = 0;
            for ( int i = 0; i < expr->expr_index.num_index; ++i ) {
                buf_push(index, resolve_expr(expr->expr_index.index[i]));
            }

            result = resolved_expr_index(resolved_expr, index, buf_len(index));
        } break;

        case EXPR_IS: {
            Resolved_Expr *test_expr = resolve_expr(expr->expr_is.var);
            Resolved_Expr *test_proc = resolve_expr(expr->expr_is.test);

            assert(test_proc);

            Type *type = test_proc->type;
            assert(type->kind == TYPE_TEST);

            if ( type->type_test.num_params != expr->expr_is.num_args+1 ) {
                fatal("falsche anzahl übergebener parameter");
            }

            Resolved_Expr **args = 0;
            Type_Field *param = type->type_test.params[0];
            if ( test_expr->type != param->type ) {
                fatal("datentyp des arguments ist falsch");
            }

            for ( int i = 1; i < type->type_test.num_params; ++i ) {
                param = type->type_test.params[i];

                Resolved_Expr *arg = resolve_expr(expr->expr_is.args[i-1]);
                if (arg->type != param->type) {
                    fatal("datentyp des arguments ist falsch");
                }
                buf_push(args, arg);
            }

            result = resolved_expr_is(test_expr, test_proc, args, buf_len(args));
        } break;

        case EXPR_IF: {
            Resolved_Expr *cond = resolve_expr(expr->expr_if.cond);
            Resolved_Expr *else_expr = 0;
            if ( expr->expr_if.else_expr ) {
                else_expr = resolve_expr(expr->expr_if.else_expr);
            }

            result = resolved_expr_if(cond, else_expr);
        } break;

        case EXPR_ARRAY_LIT: {
            Resolved_Expr **index = 0;
            Val **vals = 0;
            for ( int i = 0; i < expr->expr_array_lit.num_expr; ++i ) {
                Resolved_Expr *resolved_expr = resolve_expr(expr->expr_array_lit.expr[i]);
                buf_push(index, resolved_expr);
                buf_push(vals, resolved_expr->val);
            }

            result = resolved_expr_array_lit(index, buf_len(index), val_array(vals, buf_len(vals)));
        } break;

        default: {
            illegal_path();
        } break;
    }

    return result;
}

internal_proc Resolved_Filter *
resolve_filter(Var_Filter *filter) {
    Sym *sym = resolve_name(filter->name);

    if ( !sym ) {
        fatal("symbol konnte nicht gefunden werden!");
    }

    assert(sym->type);
    assert(sym->type->kind == TYPE_FILTER);
    Type *type = sym->type;

    if ( type->type_proc.num_params-1 < filter->num_params ) {
        fatal("zu viele argumente");
    }

    Resolved_Expr **args = 0;
    for ( int i = 1; i < type->type_proc.num_params-1; ++i ) {
        Type_Field *param = type->type_proc.params[i];

        if ( param->default_value->kind == VAL_NONE && (i-1 >= filter->num_params) ) {
            fatal("zu wenige parameter übergeben");
        }

        if ( i-1 < filter->num_params ) {
            Resolved_Expr *arg = resolve_expr(filter->params[i-1]);
            if (arg->type != param->type) {
                fatal("datentyp des arguments stimmt nicht");
            }
            buf_push(args, arg);
        }
    }

    return resolved_filter(sym, type, args, buf_len(args), type->type_filter.callback);
}

internal_proc void
init_arenas() {
    arena_init(&resolve_arena, MB(100));
}

internal_proc void
init_resolver() {
    init_arenas();
    init_builtin_types();
    init_builtin_filter();
    init_builtin_tests();
}

internal_proc Resolved_Templ *
resolve(Parsed_Templ *parsed_templ) {
    Resolved_Templ *result = resolved_templ_new();
    result->name = parsed_templ->name;

    for ( int i = 0; i < parsed_templ->num_stmts; ++i ) {
        Resolved_Stmt *stmt = resolve_stmt(parsed_templ->stmts[i]);
        buf_push(result->stmts, stmt);
    }

    result->num_stmts = buf_len(result->stmts);

    return result;
}

