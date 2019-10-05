struct Type;
struct Sym;
struct Val;
struct Operand;
struct Resolved_Stmt;
struct Resolved_Expr;
struct Resolved_Filter;

global_var Arena resolve_arena;

#define FILTER_CALLBACK(name) char * name(void *val, Resolved_Expr **params, size_t num_params)
typedef FILTER_CALLBACK(Filter_Callback);

#define TEST_CALLBACK(name) Val * name(Val *val, Resolved_Expr **args, size_t num_args)
typedef TEST_CALLBACK(Test_Callback);

Parsed_Doc *current_doc;
internal_proc Parsed_Doc *
doc_enter(Parsed_Doc *d) {
    Parsed_Doc *result = current_doc;
    current_doc = d;

    return result;
}

internal_proc void
doc_leave(Parsed_Doc *d) {
    current_doc = d;
}

internal_proc void
set_this_doc_to_be_parent_of_current_scope(Parsed_Doc *d) {
    current_doc->parent = d;
}

internal_proc Resolved_Expr *   resolve_expr(Expr *expr);
internal_proc Resolved_Expr *   resolve_expr_cond(Expr *expr);
internal_proc Resolved_Filter * resolve_filter(Var_Filter *filter);
internal_proc Resolved_Stmt *   resolve_stmt(Stmt *stmt);
internal_proc void              resolve(Parsed_Doc *d);

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
    VAL_STRUCT,
    VAL_FIELD,
};

struct Val {
    Val_Kind kind;

    union {
        char  _char;
        int   _s32;
        float _f32;
        char* _str;
        int   _range[2];
        bool  _bool;
        void* _ptr;
    };
};

global_var Val val_none;

internal_proc Val *
val_new(Val_Kind kind) {
    Val *result = ALLOC_STRUCT(&resolve_arena, Val);

    result->kind   = kind;

    return result;
}

internal_proc Val *
val_copy(Val *val) {
    Val *result = val_new(val->kind);

    result->_range[0] = val->_range[0];
    result->_range[1] = val->_range[1];

    return result;
}

internal_proc Val *
val_bool(b32 val) {
    Val *result = val_new(VAL_BOOL);

    result->_bool = val;

    return result;
}

internal_proc Val *
val_char(char val) {
    Val *result = val_new(VAL_CHAR);

    result->_char = val;

    return result;
}

internal_proc Val *
val_int(int val) {
    Val *result = val_new(VAL_INT);

    result->_s32 = val;

    return result;
}

internal_proc Val *
val_float(float val) {
    Val *result = val_new(VAL_FLOAT);

    result->_f32 = val;

    return result;
}

internal_proc Val *
val_str(char *val) {
    Val *result = val_new(VAL_STR);

    result->_str = val;

    return result;
}

internal_proc Val *
val_range(int min, int max) {
    Val *result = val_new(VAL_RANGE);

    result->_range[0] = min;
    result->_range[1] = max;

    return result;
}

internal_proc Val *
val_struct(void *ptr, size_t size) {
    Val *result = val_new(VAL_STRUCT);

    result->_ptr = ptr;

    return result;
}

global_var char to_char_buf[1000];
internal_proc char *
to_char(Val *val) {
    switch ( val->kind ) {
        case VAL_STR: {
            return val->_str;
        } break;

        case VAL_INT: {
            sprintf(to_char_buf, "%d", val->_s32);
            return to_char_buf;
        } break;

        case VAL_FLOAT: {
            sprintf(to_char_buf, "%f", val->_f32);
            return to_char_buf;
        } break;

        default: {
            assert(0);
            return "";
        } break;
    }
}

internal_proc Val
operator*(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        result._s32 = left._s32 * right._s32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._s32 * right._f32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        result._range[0] = left._s32 * right._range[0];
        result._range[1] = left._s32 * right._range[1];
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._f32 * right._f32;
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        result._range[0] = left._range[0] * right._range[0];
        result._range[1] = left._range[1] * right._range[1];
    } else {
        assert(0);
    }

    return result;
}

internal_proc Val
operator/(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        assert(right._s32 != 0);
        result._s32 = left._s32 / right._s32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._s32 / right._f32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        assert(right._range[0] != 0 && right._range[1] != 0);
        result._range[0] = left._s32 / right._range[0];
        result._range[1] = left._s32 / right._range[1];
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._f32 / right._f32;
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        assert(right._range[0] != 0 && right._range[1] != 0);
        result._range[0] = left._range[0] * right._range[0];
        result._range[1] = left._range[1] * right._range[1];
    } else {
        assert(0);
    }

    return result;
}

internal_proc Val
operator+(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        result._s32 = left._s32 + right._s32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._s32 + right._f32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        result._range[0] = left._s32 + right._range[0];
        result._range[1] = left._s32 + right._range[1];
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._f32 + right._f32;
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        result._range[0] = left._range[0] + right._range[0];
        result._range[1] = left._range[1] + right._range[1];
    } else {
        assert(0);
    }

    return result;
}

internal_proc Val
operator-(Val left, Val right) {
    Val result = {};

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result.kind = VAL_INT;
        result._s32 = left._s32 - right._s32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._s32 - right._f32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        result._range[0] = left._s32 - right._range[0];
        result._range[1] = left._s32 - right._range[1];
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result.kind = VAL_FLOAT;
        result._f32 = left._f32 - right._f32;
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result.kind = VAL_RANGE;
        result._range[0] = left._range[0] - right._range[0];
        result._range[1] = left._range[1] - right._range[1];
    } else {
        assert(0);
    }

    return result;
}

internal_proc Val
operator<(Val left, Val right) {
    Val result = {};
    result.kind = VAL_BOOL;

    if ( left.kind == VAL_INT && right.kind == VAL_INT ) {
        result._bool = left._s32 < right._s32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_FLOAT ) {
        result._bool = left._s32 < right._f32;
    } else if ( left.kind == VAL_INT && right.kind == VAL_RANGE ) {
        result._bool = left._s32 < right._range[0] && left._s32 < right._range[1];
    } else if ( left.kind == VAL_FLOAT && right.kind == VAL_FLOAT ) {
        result._bool = left._f32 < right._f32;
    } else if ( left.kind == VAL_RANGE && right.kind == VAL_RANGE ) {
        result._bool = left._range[0] < right._range[0] && left._range[1] < right._range[1];
    } else {
        assert(0);
    }

    return result;
}

internal_proc b32
operator==(Val left, Val right) {
    if ( left.kind != right.kind ) {
        return false;
    }

    if ( left.kind == VAL_INT ) {
        return left._s32 == right._s32;
    }

    if ( left.kind == VAL_FLOAT ) {
        f32 eps = 0.00001f;

        if ( (left._f32 - right._f32) < eps || (left._f32 - right._f32) > eps ) {
            return false;
        } else {
            return true;
        }
    }

    if ( left.kind == VAL_RANGE ) {
        return left._range[0] == right._range[0] && left._range[1] == right._range[1];
    }

    if ( left.kind == VAL_STR ) {
        return left._str == right._str;
    }

    if ( left.kind == VAL_BOOL ) {
        return left._bool == right._bool;
    }

    return false;
}
/* }}} */

internal_proc Sym * sym_push_var(char *name, Type *type, Val *val = 0);

/* type {{{ */
struct Type_Field {
    char *name;
    Type *type;
    s64   offset;
    Val  *default_val;
};

internal_proc Type_Field *
type_field(char *name, Type *type, Val *default_val = 0) {
    Type_Field *result = ALLOC_STRUCT(&resolve_arena, Type_Field);

    result->name = intern_str(name);
    result->type = type;
    result->default_val = default_val;

    return result;
}

enum Type_Kind {
    TYPE_NONE,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STR,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_PROC,
    TYPE_FILTER,
    TYPE_TEST,

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
            Filter_Callback *callback;
        } type_proc;

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
    };
};

#define PTR_SIZE 8

global_var Type *type_void;
global_var Type *type_bool;
global_var Type *type_char;
global_var Type *type_int;
global_var Type *type_float;
global_var Type *type_str;

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
type_proc(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_PROC);

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
    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;
    result->type_proc.callback = callback;

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

internal_proc void
init_builtin_types() {
    type_void  = type_new(TYPE_VOID);
    type_void->size = 0;

    type_bool  = type_new(TYPE_BOOL);
    type_bool->size = 1;

    type_char  = type_new(TYPE_CHAR);
    type_char->size = 1;

    type_int   = type_new(TYPE_INT);
    type_int->size = 4;

    type_float = type_new(TYPE_FLOAT);
    type_float->size = 4;

    type_str   = type_new(TYPE_STR);
    type_str->size = PTR_SIZE;

    arithmetic_result_type_table[TYPE_CHAR][TYPE_CHAR]   = type_char;
    arithmetic_result_type_table[TYPE_CHAR][TYPE_INT]    = type_int;
    arithmetic_result_type_table[TYPE_CHAR][TYPE_FLOAT]  = type_float;

    arithmetic_result_type_table[TYPE_INT][TYPE_CHAR]    = type_int;
    arithmetic_result_type_table[TYPE_INT][TYPE_INT]     = type_int;
    arithmetic_result_type_table[TYPE_INT][TYPE_FLOAT]   = type_float;

    arithmetic_result_type_table[TYPE_FLOAT][TYPE_CHAR]  = type_float;
    arithmetic_result_type_table[TYPE_FLOAT][TYPE_INT]   = type_float;
    arithmetic_result_type_table[TYPE_FLOAT][TYPE_FLOAT] = type_float;

    scalar_result_type_table[TYPE_CHAR][TYPE_BOOL]       = type_void;
    scalar_result_type_table[TYPE_CHAR][TYPE_CHAR]       = type_char;
    scalar_result_type_table[TYPE_CHAR][TYPE_INT]        = type_int;
    scalar_result_type_table[TYPE_CHAR][TYPE_FLOAT]      = type_float;

    scalar_result_type_table[TYPE_INT][TYPE_BOOL]        = type_void;
    scalar_result_type_table[TYPE_INT][TYPE_CHAR]        = type_int;
    scalar_result_type_table[TYPE_INT][TYPE_INT]         = type_int;
    scalar_result_type_table[TYPE_INT][TYPE_FLOAT]       = type_float;

    scalar_result_type_table[TYPE_FLOAT][TYPE_BOOL]      = type_void;
    scalar_result_type_table[TYPE_FLOAT][TYPE_CHAR]      = type_float;
    scalar_result_type_table[TYPE_FLOAT][TYPE_INT]       = type_float;
    scalar_result_type_table[TYPE_FLOAT][TYPE_FLOAT]     = type_float;

    scalar_result_type_table[TYPE_BOOL][TYPE_BOOL]      = type_bool;
    scalar_result_type_table[TYPE_BOOL][TYPE_CHAR]      = type_void;
    scalar_result_type_table[TYPE_BOOL][TYPE_INT]       = type_void;
    scalar_result_type_table[TYPE_BOOL][TYPE_FLOAT]     = type_void;
}

internal_proc b32
is_int(Type *type) {
    b32 result = (type->kind == TYPE_INT || type->kind == TYPE_CHAR);

    return result;
}

internal_proc b32
is_arithmetic(Type *type) {
    b32 result = TYPE_CHAR <= type->kind && type->kind <= TYPE_FLOAT;

    return result;
}

internal_proc b32
is_scalar(Type *type) {
    b32 result = TYPE_BOOL <= type->kind && type->kind <= TYPE_FLOAT;

    return result;
}

internal_proc b32
is_callable(Type *type) {
    b32 result = ( type->kind == TYPE_PROC || type->kind == TYPE_FILTER );

    return result;
}
/* }}} */
/* sym {{{ */
enum Sym_Kind {
    SYM_NONE,
    SYM_VAR,
    SYM_PROC,
    SYM_STRUCT,
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
        assert(!"symbol existiert bereits");
    } else if ( sym ) {
        assert(!"warnung: symbol wird überschattet");
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
/* }}} */
/* resolved_expr {{{ */
struct Resolved_Expr {
    Expr_Kind kind;
    Type *type;
    Sym *sym;
    Val *val;

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
    Resolved_Expr *result = resolved_expr_new(EXPR_UNARY);

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

    union {
        struct {
            Resolved_Expr *expr;
            Resolved_Filter **filter;
            size_t num_filter;
        } stmt_var;

        struct {
            Sym *it;
            Resolved_Expr *expr;
            Resolved_Stmt **stmts;
            size_t num_stmts;
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
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_filter;
    };
};

global_var Resolved_Stmt **resolved_stmts;

internal_proc Resolved_Stmt *
resolved_stmt_new(Stmt_Kind kind) {
    Resolved_Stmt *result = ALLOC_STRUCT(&resolve_arena, Resolved_Stmt);

    result->kind = kind;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_var(Resolved_Expr *expr, Resolved_Filter **filter, size_t num_filter) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_VAR);

    result->stmt_var.expr = expr;
    result->stmt_var.filter = filter;
    result->stmt_var.num_filter = num_filter;

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
resolved_stmt_for(Sym *it, Resolved_Expr *expr, Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_FOR);

    result->stmt_for.it = it;
    result->stmt_for.expr = expr;
    result->stmt_for.stmts = stmts;
    result->stmt_for.num_stmts = num_stmts;

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

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_filter(Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_FILTER);

    result->stmt_filter.stmts = stmts;
    result->stmt_filter.num_stmts = num_stmts;

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
        } else if ( dest_type->kind == TYPE_CHAR ) {
            return true;
        } else if ( dest_type->kind == TYPE_BOOL ) {
            return true;
        }
    }

    if ( op->type->kind == TYPE_CHAR ) {
        if ( dest_type->kind == TYPE_INT ) {
            op->type = type_int;

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
internal_proc FILTER_CALLBACK(upper) {
    char *str = (char *)val;
    char *result = "";

    for ( int i = 0; i < strlen(str); ++i ) {
        result = strf("%s%c", result, toupper(str[i]));
    }

    return result;
}

internal_proc FILTER_CALLBACK(escape) {
    char *str = *(char **)val;

    return str;
}

internal_proc FILTER_CALLBACK(truncate) {
    char *str = *(char **)val;

    return str;
}

internal_proc void
init_builtin_filter() {
    Type_Field *str_type[] = { type_field("s", type_str) };
    sym_push_filter("upper",  type_filter(str_type, 1, type_str, upper));
    sym_push_filter("escape", type_filter(str_type, 1, type_str, escape));

    Type_Field *trunc_type[] = {
        type_field("s", type_str),
        type_field("length", type_int, val_int(255)),
        type_field("end", type_str, val_str("...")),
        type_field("killwords", type_bool, val_bool(false)),
        type_field("leeway", type_int, val_int(0)),
    };
    sym_push_filter("truncate", type_filter(trunc_type, 5, type_str, truncate));
}
/* }}} */
/* tests {{{ */
internal_proc b32
test_callable(Resolved_Expr *expr) {
    return false;
}

internal_proc b32
test_defined(Resolved_Expr *expr) {
    return false;
}

TEST_CALLBACK(test_divisibleby) {
    return false;
}

TEST_CALLBACK(test_eq) {
    assert(val);
    assert(num_args == 1);

    Val *result = val_bool(*val == *args[0]->val);

    return result;
}

internal_proc b32
test_escaped(Val *value) {
    return false;
}

internal_proc b32
test_even(Val *val) {
    return false;
}

internal_proc b32
test_ge(Val *a, Val *b) {
    return false;
}

internal_proc b32
test_gt(Val *a, Val *b) {
    return false;
}

internal_proc b32
test_in(Val *value, Val *min, Val *max) {
    return false;
}

internal_proc void
init_builtin_tests() {
    Type_Field *str_type[]  = { type_field("s", type_str) };
    Type_Field *int_type[]  = { type_field("s", type_int) };
    Type_Field *int2_type[] = { type_field("left", type_int), type_field("right", type_int) };

    /*
    sym_push_test("callable", type_test());
    sym_push_test("defined", type_test());
    */
    sym_push_test("divisibleby", type_test(int2_type, 2, test_divisibleby));
    sym_push_test("eq", type_test(int2_type, 2, test_eq));
}
/* }}} */

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

            result = resolved_stmt_var(resolve_expr(stmt->stmt_var.expr), res_filter, buf_len(res_filter));
        } break;

        case STMT_FOR: {
            scope_enter();

            Resolved_Expr *expr = resolve_expr(stmt->stmt_for.cond);
            Sym *it = sym_push_var(stmt->stmt_for.it, expr->type, val_int(0));

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_for.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_for.stmts[i]));
            }

            scope_leave();

            result = resolved_stmt_for(it, expr, stmts, buf_len(stmts));
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

        case STMT_END: {
        } break;

        case STMT_LIT: {
            result = resolved_stmt_lit(stmt->stmt_lit.value);
        } break;

        case STMT_EXTENDS: {
            Parsed_Doc *doc = parse_file(stmt->stmt_extends.name);
            resolve(doc);
            set_this_doc_to_be_parent_of_current_scope(doc);
        } break;

        case STMT_SET: {
            Sym *sym = resolve_name(stmt->stmt_set.name);
            Resolved_Expr *expr = resolve_expr(stmt->stmt_set.expr);

            if ( !sym ) {
                sym = sym_push_var(stmt->stmt_set.name, expr->type, val_copy(expr->val));
            } else {
                if ( !convert_operand(expr, sym->type) ) {
                    assert(!"datentyp des operanden passt nicht");
                }
            }

            result = resolved_stmt_set(sym, expr);
        } break;

        case STMT_FILTER: {
            assert(0);

            for ( int i = 0; i < stmt->stmt_filter.num_filter; ++i ) {
                resolve_filter(&stmt->stmt_filter.filter[i]);
            }

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_filter.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_filter.stmts[i]));
            }

            result = resolved_stmt_filter(stmts, buf_len(stmts));
        } break;

        default: {
            assert(0);
            return result;
        } break;
    }

    return result;
}

internal_proc Resolved_Expr *
resolve_expr_cond(Expr *expr) {
    Resolved_Expr *result = resolve_expr(expr);

    /* @TODO: char, int, str akzeptieren */
    if ( result->type != type_bool ) {
        assert(!"boolischen ausdruck erwartet");
    }

    return result;
}

internal_proc Resolved_Expr *
eval_binary_op(Token_Kind op, Resolved_Expr *left, Resolved_Expr *right) {
    switch (op) {
        case T_PLUS: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(left->val->_s32 + right->val->_s32);
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(left->val->_f32 + right->val->_f32);
                    return left;
                } break;

                default: {
                    assert(!"nicht unterstützer datentyp");
                } break;
            }
        } break;

        case T_MINUS: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(left->val->_s32 - right->val->_s32);
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(left->val->_f32 - right->val->_f32);
                    return left;
                } break;

                default: {
                    assert(!"nicht unterstützer datentyp");
                } break;
            }
        } break;

        case T_MUL: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(left->val->_s32 * right->val->_s32);
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(left->val->_f32 * right->val->_f32);
                    return left;
                } break;

                default: {
                    assert(!"nicht unterstützer datentyp");
                } break;
            }
        } break;

        case T_DIV: {
            switch ( left->val->kind ) {
                case VAL_INT: {
                    left->val = val_int(left->val->_s32 / right->val->_s32);
                    return left;
                } break;

                case VAL_FLOAT: {
                    left->val = val_float(left->val->_f32 / right->val->_f32);
                    return left;
                } break;

                default: {
                    assert(!"nicht unterstützer datentyp");
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
                assert(!"konnte symbol nicht auflösen");
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
                assert(!"beide datentypen der zuweisung müssen gleich sein");
            }

            result = operand_rvalue(middle->type);
        } break;

        case EXPR_FIELD: {
            Resolved_Expr *base = resolve_expr(expr->expr_field.expr);

            assert(base->type);
            Type *type = base->type;
            assert(type->kind == TYPE_STRUCT);

            Scope *old_scope = scope_set(type->type_aggr.scope);
            Sym *sym = resolve_name(expr->expr_field.field);
            assert(sym);

            s64 offset = offset_from_base(type, sym->name);
            // scope_set(type->type_aggr.scope->parent);
            scope_set(old_scope);

            assert(sym);
            result = resolved_expr_field(base, sym, offset, sym->type);
        } break;

        case EXPR_RANGE: {
            Resolved_Expr *left  = resolve_expr(expr->expr_range.left);
            Resolved_Expr *right = resolve_expr(expr->expr_range.right);
            unify_arithmetic_operands(left, right);

            if ( !is_int(left->type) ) {
                assert(!"range typ muss vom typ int sein");
            }

            if ( !is_int(right->type) ) {
                assert(!"range typ muss vom typ int sein");
            }

            int min = left->val->_s32;
            int max = right->val->_s32;

            result = resolved_expr_range(min, max);
        } break;

        case EXPR_CALL: {
            Resolved_Expr *operand = resolve_expr(expr->expr_call.expr);
            Type *type = operand->type;
            if ( !is_callable(type) ) {
                assert(!"aufruf einer nicht-prozedur");
            }

            if ( type->type_proc.num_params < expr->expr_call.num_params ) {
                assert(!"zu viele argumente");
            }

            for ( int i = 0; i < type->type_proc.num_params; ++i ) {
                Type_Field *param = type->type_proc.params[i];

                if ( param->default_val->kind == VAL_NONE && (i >= expr->expr_call.num_params) ) {
                    assert(!"zu wenige parameter übergeben");
                }

                if ( i < expr->expr_call.num_params ) {
                    Resolved_Expr *arg = resolve_expr(expr->expr_call.params[i]);
                    if (arg->type != param->type) {
                        assert(!"datentyp des arguments stimmt nicht");
                    }
                }
            }

            result = operand_rvalue(type->type_proc.ret);
        } break;

        case EXPR_INDEX: {
            Resolved_Expr *operand = resolve_expr(expr->expr_index.expr);
            if (operand->type->kind != TYPE_ARRAY) {
                assert(!"indizierung auf einem nicht-array");
            }

            Resolved_Expr *index = resolve_expr(expr->expr_index.index);
            if ( !is_int(index->type) ) {
                assert(!"index muss von typ int sein");
            }

            result = operand_rvalue(operand->type);
        } break;

        case EXPR_IS: {
            Resolved_Expr *test_expr = resolve_expr(expr->expr_is.var);
            Resolved_Expr *test_proc = resolve_expr(expr->expr_is.test);

            assert(test_proc);

            Type *type = test_proc->type;
            assert(type->kind == TYPE_TEST);

            if ( type->type_test.num_params != expr->expr_is.num_args+1 ) {
                assert(!"falsche anzahl übergebener parameter");
            }

            Resolved_Expr **args = 0;
            Type_Field *param = type->type_test.params[0];
            if ( test_expr->type != param->type ) {
                assert(!"datentyp des arguments ist falsch");
            }

            for ( int i = 1; i < type->type_test.num_params; ++i ) {
                param = type->type_test.params[i];

                Resolved_Expr *arg = resolve_expr(expr->expr_is.args[i-1]);
                if (arg->type != param->type) {
                    assert(!"datentyp des arguments ist falsch");
                }
                buf_push(args, arg);
            }

            result = resolved_expr_is(test_expr, test_proc, args, buf_len(args));
        } break;

        default: {
            assert(0);
        } break;
    }

    return result;
}

internal_proc Resolved_Filter *
resolve_filter(Var_Filter *filter) {
    Sym *sym = resolve_name(filter->name);

    if ( !sym ) {
        assert(!"symbol konnte nicht gefunden werden!");
    }

    assert(sym->type);
    assert(sym->type->kind == TYPE_FILTER);
    Type *type = sym->type;

    if ( type->type_proc.num_params-1 < filter->num_params ) {
        assert(!"zu viele argumente");
    }

    Resolved_Expr **args = 0;
    for ( int i = 1; i < type->type_proc.num_params-1; ++i ) {
        Type_Field *param = type->type_proc.params[i];

        if ( param->default_val->kind == VAL_NONE && (i-1 >= filter->num_params) ) {
            assert(!"zu wenige parameter übergeben");
        }

        if ( i-1 < filter->num_params ) {
            Resolved_Expr *arg = resolve_expr(filter->params[i-1]);
            if (arg->type != param->type) {
                assert(!"datentyp des arguments stimmt nicht");
            }
            buf_push(args, arg);
        }
    }

    return resolved_filter(sym, type, args, buf_len(args), type->type_proc.callback);
}

struct Address {
    char *street;
    char *city;
};

struct User {
    char *name;
    int age;
    Address address;
};

internal_proc void
init_test_datatype() {
    Type_Field *address_fields[] = { type_field("street", type_str), type_field("city", type_str) };
    Type *address_type = type_struct(address_fields, 2);

    Type_Field *user_fields[] = { type_field("name", type_str), type_field("age", type_int), type_field("address", address_type) };
    Type *user_type = type_struct(user_fields, 3);

    User *user = (User *)xmalloc(sizeof(User));
    user->name = "Noob";
    user->age  = 40;
    user->address.street = "Traube";
    user->address.city = "Mannheim";

    Sym *sym = sym_push_var("user", user_type, val_struct(user, sizeof(User)));
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
    init_test_datatype();
}

internal_proc void
resolve(Parsed_Doc *d) {
    Parsed_Doc *prev_doc = doc_enter(d);
    b32 in_template = false;

    for ( int i = 0; i < d->num_stmts; ++i ) {
        Resolved_Stmt *stmt = resolve_stmt(d->stmts[i]);
        int stmt_kind = d->stmts[i]->kind;

        if ( stmt_kind == STMT_EXTENDS ) {
            in_template = true;
        }

        if ( stmt ) {
            if ( stmt_kind == STMT_EXTENDS ) {
                if (i > 0 ) {
                    fatal("extends anweisung muss die erste anweisung im template sein");
                }
            }

            buf_push(resolved_stmts, stmt);
        }
    }

    doc_leave(prev_doc);
}

