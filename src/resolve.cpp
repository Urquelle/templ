struct Type;
struct Sym;
struct Operand;

Doc *current_doc;
internal_proc Doc *
doc_enter(Doc *d) {
    Doc *result = current_doc;
    current_doc = d;

    return result;
}

internal_proc void
doc_leave(Doc *d) {
    current_doc = d;
}

internal_proc void
set_parent(Doc *d) {
    current_doc->parent = d;
}

internal_proc void      resolve_item(Item *item);
internal_proc Operand * resolve_expr(Expr *expr);
internal_proc void      resolve_filter(Var_Filter filter);
internal_proc void      resolve_stmt(Stmt *stmt);
internal_proc void      resolve_doc(Doc *d);

enum Val_Kind {
    VAL_NONE,
    VAL_BOOL,
    VAL_CHAR,
    VAL_INT,
    VAL_FLOAT,
    VAL_STR,
    VAL_RANGE,
    VAL_STRUCT,
};

struct Val {
    Val_Kind kind;

    union {
        b32   bool_val;
        char  char_val;
        int   int_val;
        float float_val;
        char *str_val;
        struct {
            int min;
            int max;
        } range_val;

        struct {
            void* ptr;
        } struct_val;
    };
};

global_var Val val_none;

internal_proc Val
val_new(Val_Kind kind) {
    Val result = {};

    result.kind = kind;

    return result;
}

internal_proc Val
val_bool(b32 val) {
    Val result = val_new(VAL_BOOL);

    result.bool_val = val;

    return result;
}

internal_proc Val
val_char(char val) {
    Val result = val_new(VAL_CHAR);

    result.char_val = val;

    return result;
}

internal_proc Val
val_int(int val) {
    Val result = val_new(VAL_INT);

    result.int_val = val;

    return result;
}

internal_proc Val
val_float(float val) {
    Val result = val_new(VAL_FLOAT);

    result.float_val = val;

    return result;
}

internal_proc Val
val_str(char *val) {
    Val result = val_new(VAL_STR);

    result.str_val = val;

    return result;
}

internal_proc Val
val_range(int min, int max) {
    Val result = val_new(VAL_RANGE);

    result.range_val.min = min;
    result.range_val.max = max;

    return result;
}

internal_proc Val
val_struct(void *ptr) {
    Val result = val_new(VAL_STRUCT);

    result.struct_val.ptr = ptr;

    return result;
}

struct Type_Field {
    char *name;
    Type *type;
    Val   default_val;
};

internal_proc Type_Field *
type_field(char *name, Type *type, Val default_val = val_none) {
    Type_Field *result = (Type_Field *)xmalloc(sizeof(Type_Field));

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
};

struct Type {
    Type_Kind kind;

    union {
        struct {
            Type_Field **fields;
            size_t num_fields;
        } type_aggr;

        struct {
            Type_Field **params;
            size_t num_params;
            Type *ret;
        } type_proc;

        struct {
            Type *base;
            int index;
        } type_array;
    };
};

global_var Type *type_void;
global_var Type *type_bool;
global_var Type *type_char;
global_var Type *type_int;
global_var Type *type_float;
global_var Type *type_str;

global_var Type *arithmetic_result_type_table[3][3];
global_var Type *scalar_result_type_table[4][4];

internal_proc Type *
type_new(Type_Kind kind) {
    Type *result = (Type *)xmalloc(sizeof(Type));

    result->kind = kind;

    return result;
}

internal_proc Type *
type_struct(Type_Field **fields, size_t num_fields) {
    Type *result = type_new(TYPE_STRUCT);

    result->type_aggr.fields = (Type_Field **)AST_DUP(fields);
    result->type_aggr.num_fields = num_fields;

    return result;
}

internal_proc Type *
type_proc(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_PROC);

    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;

    return result;
}

internal_proc Type *
type_filter(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_FILTER);

    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;

    return result;
}

internal_proc Type *
type_array(Type *base, int index) {
    Type *result = type_new(TYPE_ARRAY);

    result->type_array.base = base;
    result->type_array.index = index;

    return result;
}

internal_proc void
init_builtin_types() {
    type_void  = type_new(TYPE_VOID);
    type_bool  = type_new(TYPE_BOOL);
    type_char  = type_new(TYPE_CHAR);
    type_int   = type_new(TYPE_INT);
    type_float = type_new(TYPE_FLOAT);
    type_str   = type_new(TYPE_STR);

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

struct Scope {
    char*  name;
    Sym**  syms;
    size_t num_syms;
};

enum { MAX_SCOPE_DEPTH = 124 };
Scope scopes[MAX_SCOPE_DEPTH] = {0};
global_var int current_scope_idx;

internal_proc void
scope_enter() {
    assert(current_scope_idx < MAX_SCOPE_DEPTH);
    current_scope_idx++;
}

internal_proc void
scope_leave() {
    assert(current_scope_idx > 0);
    current_scope_idx--;
}

struct Operand {
    Type* type;
    Val   val;

    b32 is_const;
    b32 is_lvalue;
};

internal_proc Operand *
operand_new(Type *type, Val val) {
    Operand *result = (Operand *)xcalloc(1, sizeof(Operand));

    result->type = type;
    result->val  = val;

    return result;
}

internal_proc Operand *
operand_lvalue(Type *type, Val val = val_none) {
    Operand *result = operand_new(type, val);

    result->is_lvalue = true;

    return result;
}

internal_proc Operand *
operand_rvalue(Type *type, Val val = val_none) {
    Operand *result = operand_new(type, val);

    return result;
}

internal_proc Operand *
operand_const(Type *type, Val val = val_none) {
    Operand *result = operand_new(type, val);

    result->is_const = true;

    return result;
}

internal_proc b32
unify_arithmetic_operands(Operand *left, Operand *right) {
    if ( is_arithmetic(left->type) && is_arithmetic(right->type) ) {
        Type *type  = arithmetic_result_type_table[left->type->kind][right->type->kind];
        left->type  = type;
        right->type = type;

        return true;
    }

    return false;
}

internal_proc b32
unify_scalar_operands(Operand *left, Operand *right) {
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

enum Sym_Kind {
    SYM_NONE,
    SYM_VAR,
    SYM_PROC,
    SYM_STRUCT,
};

struct Sym {
    Sym_Kind kind;
    char *name;
    Type *type;
};

internal_proc Sym *
sym_new(Sym_Kind kind, char *name, Type *type) {
    Sym *result = (Sym *)xmalloc(sizeof(Sym));

    result->name = name;
    result->kind = kind;
    result->type = type;

    return result;
}

internal_proc Sym *
sym_get(char *name) {
    for ( int i = current_scope_idx; i >= 0; --i ) {
        for ( int j = 0; j < scopes[i].num_syms; ++j ) {
            Sym *sym = scopes[i].syms[j];
            if ( sym->name == name ) {
                return sym;
            }
        }
    }

    return 0;
}

internal_proc void
sym_push(Sym_Kind kind, char *name, Type *type) {
    name = intern_str(name);

    /* @INFO: im lokalen scope nachschauen. falls vorhanden fehler ausgeben! */
    for ( int i = 0; i < scopes[current_scope_idx].num_syms; ++i ) {
        if ( scopes[current_scope_idx].syms[i]->name == name ) {
            assert(!"symbol existiert bereits");
        }
    }

    Sym *shadowed_sym = sym_get(name);
    if ( shadowed_sym ) {
        assert(!"warnung: symbol wird überschattet");
    }

    buf_push(scopes[current_scope_idx].syms, sym_new(kind, name, type));
    scopes[current_scope_idx].num_syms++;
}

internal_proc void
sym_push_filter(char *name, Type *type) {
    sym_push(SYM_PROC, name, type);
}

internal_proc void
sym_push_var(char *name, Type *type) {
    sym_push(SYM_VAR, name, type);
}

internal_proc void
init_builtin_filter() {
    Type_Field *str_type[] = { type_field("s", type_str) };
    sym_push_filter("upper",  type_filter(str_type, 1, type_str));
    sym_push_filter("escape", type_filter(str_type, 1, type_str));

    Type_Field *trunc_type[] = {
        type_field("s", type_str),
        type_field("length", type_int, val_int(255)),
        type_field("end", type_str, val_str("...")),
        type_field("killwords", type_bool, val_bool(false)),
        type_field("leeway", type_int, val_int(0)),
    };
    sym_push_filter("truncate", type_filter(trunc_type, 5, type_str));
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

internal_proc void
resolve_stmt(Stmt *stmt) {
    switch (stmt->kind) {
        case STMT_VAR: {
            resolve_item(stmt->stmt_var.item);
        } break;

        case STMT_FOR: {
            scope_enter();
            Operand *operand = resolve_expr(stmt->stmt_for.cond);
            sym_push_var(stmt->stmt_for.it, operand->type);
            resolve_stmts(stmt->stmt_for.stmts, stmt->stmt_for.num_stmts);
            scope_leave();
        } break;

        case STMT_IF: {
            scope_enter();
            Operand *operand = resolve_expr(stmt->stmt_if.cond);
            resolve_stmts(stmt->stmt_if.stmts, stmt->stmt_if.num_stmts);
            scope_leave();
        } break;

        case STMT_BLOCK: {
            scope_enter();
            resolve_stmts(stmt->stmt_block.stmts, stmt->stmt_block.num_stmts);
            scope_leave();
        } break;

        case STMT_END: {
        } break;

        case STMT_LIT: {
            resolve_item(stmt->stmt_lit.item);
        } break;

        case STMT_EXTENDS: {
            Doc *doc = parse_file(stmt->stmt_extends.name);
            resolve_doc(doc);
            set_parent(doc);
        } break;

        case STMT_SET: {
            Sym *sym = resolve_name(stmt->stmt_set.name);
            Operand *operand = resolve_expr(stmt->stmt_set.expr);

            if ( !sym ) {
                sym_push_var(stmt->stmt_set.name, operand->type);
            } else {
                if ( sym->type != operand->type ) {
                    assert(!"datentyp des operanden passt nicht");
                }
            }
        } break;

        case STMT_FILTER: {
            for ( int i = 0; i < stmt->stmt_filter.num_filter; ++i ) {
                resolve_filter(stmt->stmt_filter.filter[i]);
            }
            resolve_stmts(stmt->stmt_filter.stmts, stmt->stmt_filter.num_stmts);
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc Operand *
resolve_expr_field(Expr *expr) {
    assert(expr->kind == EXPR_FIELD);
    Operand *operand = resolve_expr(expr->expr_field.expr);
    assert(operand->type);
    Type *type = operand->type;

    char *name = NULL;
    switch (expr->expr_field.field->kind) {
        case EXPR_NAME: {
            name = expr->expr_field.field->expr_name.value;
        } break;

        case EXPR_CALL: {
            assert(expr->expr_call.expr->kind == EXPR_NAME);
            name = expr->expr_call.expr->expr_name.value;
        } break;

        case EXPR_INDEX: {
            assert(expr->expr_index.expr->kind == EXPR_NAME);
            name = expr->expr_index.expr->expr_name.value;
        } break;

        default: {
            assert(!"nicht unterstützter ausdruck");
        } break;
    }

    for ( int i = 0; i < type->type_aggr.num_fields; ++i ) {
        Type_Field *field = type->type_aggr.fields[i];

        if (name == field->name) {
            return operand_rvalue(field->type);
        }
    }

    assert(!"kein passendes feld gefunden");
    return 0;
}

internal_proc Operand *
resolve_expr_cond(Expr *expr) {
    Operand *operand = resolve_expr(expr);

    if ( operand->type != type_bool ) {
        assert(!"boolischen ausdruck erwartet");
    }

    return operand;
}

internal_proc Operand *
resolve_expr(Expr *expr) {
    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
            if ( !sym ) {
                assert(!"konnte symbol nicht auflösen");
            }

            return operand_lvalue(sym->type);
        } break;

        case EXPR_STR: {
            return operand_const(type_str, val_str(expr->expr_str.value));
        } break;

        case EXPR_INT: {
            return operand_const(type_int, val_int(expr->expr_int.value));
        } break;

        case EXPR_FLOAT: {
            return operand_const(type_float, val_float(expr->expr_float.value));
        } break;

        case EXPR_BOOL: {
            return operand_const(type_bool, val_bool(expr->expr_bool.value));
        } break;

        case EXPR_PAREN: {
            return resolve_expr(expr->expr_paren.expr);
        } break;

        case EXPR_UNARY: {
            return resolve_expr(expr->expr_unary.expr);
        } break;

        case EXPR_BINARY: {
            Operand *left  = resolve_expr(expr->expr_binary.left);
            Operand *right = resolve_expr(expr->expr_binary.right);

            if ( is_eql(expr->expr_binary.op) ) {
                unify_scalar_operands(left, right);
            } else {
                unify_arithmetic_operands(left, right);
            }

            return operand_rvalue(left->type);
        } break;

        case EXPR_TERNARY: {
            Operand *left   = resolve_expr_cond(expr->expr_ternary.left);
            Operand *middle = resolve_expr(expr->expr_ternary.middle);
            Operand *right  = resolve_expr(expr->expr_ternary.right);

            if ( middle->type != right->type ) {
                assert(!"beide datentypen der zuweisung müssen gleich sein");
            }

            return operand_rvalue(middle->type);
        } break;

        case EXPR_FIELD: {
            return resolve_expr_field(expr);
        } break;

        case EXPR_RANGE: {
            Operand *left  = resolve_expr(expr->expr_range.left);
            Operand *right = resolve_expr(expr->expr_range.right);

            if ( !is_int(left->type) ) {
                assert(!"range typ muss vom typ int sein");
            }

            if ( !is_int(right->type) ) {
                assert(!"range typ muss vom typ int sein");
            }

            /* @TODO: operanden zurückgeben mit range als Val */
            return operand_rvalue(type_int);
        } break;

        case EXPR_CALL: {
            Operand *operand = resolve_expr(expr->expr_call.expr);
            Type *type = operand->type;
            if ( !is_callable(type) ) {
                assert(!"aufruf einer nicht-prozedur");
            }

            if ( type->type_proc.num_params < expr->expr_call.num_params ) {
                assert(!"zu viele argumente");
            }

            for ( int i = 0; i < type->type_proc.num_params; ++i ) {
                Type_Field *param = type->type_proc.params[i];

                if ( param->default_val.kind == VAL_NONE && (i >= expr->expr_call.num_params) ) {
                    assert(!"zu wenige parameter übergeben");
                }

                if ( i < expr->expr_call.num_params ) {
                    Operand *arg = resolve_expr(expr->expr_call.params[i]);
                    if (arg->type != param->type) {
                        assert(!"datentyp des arguments stimmt nicht");
                    }
                }
            }

            return operand_rvalue(type->type_proc.ret);
        } break;

        case EXPR_INDEX: {
            Operand *operand = resolve_expr(expr->expr_index.expr);
            if (operand->type->kind != TYPE_ARRAY) {
                assert(!"indizierung auf einem nicht-array");
            }

            return operand_const(operand->type);
        } break;

        default: {
            assert(0);
            return 0;
        } break;
    }
}

internal_proc void
resolve_filter(Var_Filter filter) {
    Sym *sym = resolve_name(filter.name);

    if ( !sym ) {
        assert(!"symbol konnte nicht gefunden werden!");
    }

    assert(sym->type);
    assert(sym->type->kind == TYPE_FILTER);
    Type *type = sym->type;

    if ( type->type_proc.num_params-1 < filter.num_params ) {
        assert(!"zu viele argumente");
    }

    for ( int i = 1; i < type->type_proc.num_params-1; ++i ) {
        Type_Field *param = type->type_proc.params[i];

        if ( param->default_val.kind == VAL_NONE && (i-1 >= filter.num_params) ) {
            assert(!"zu wenige parameter übergeben");
        }

        if ( i-1 < filter.num_params ) {
            Operand *arg = resolve_expr(filter.params[i-1]);
            if (arg->type != param->type) {
                assert(!"datentyp des arguments stimmt nicht");
            }
        }
    }
}

internal_proc void
resolve_var(Item *item) {
    assert(item->kind == ITEM_VAR);

    Operand *operand = resolve_expr(item->item_var.expr);
    for ( int i = 0; i < item->item_var.num_filter; ++i ) {
        resolve_filter(item->item_var.filter[i]);
    }
}

internal_proc void
resolve_code(Item *item) {
    assert(item->kind == ITEM_CODE);

    resolve_stmt(item->item_code.stmt);
}

internal_proc void
resolve_item(Item *item) {
    switch (item->kind) {
        case ITEM_LIT: {
            // nichts tun
        } break;

        case ITEM_VAR: {
            resolve_var(item);
        } break;

        case ITEM_CODE: {
            resolve_code(item);
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
resolve_doc(Doc *d) {
    Doc *prev_doc = doc_enter(d);
    for ( int i = 0; i < d->num_items; ++i ) {
        resolve_item(d->items[i]);
    }
    doc_leave(prev_doc);
}

struct Test_User {
    char *name;
};

internal_proc void
init_test_datatype() {
    Type_Field *user_fields[] = { type_field("name", type_str) };
    Type *user_type = type_struct(user_fields, 1);

    Test_User *user = (Test_User *)xmalloc(sizeof(Test_User));
    user->name = "Noob";

    sym_push_var("user", user_type);
}

internal_proc void
init_resolver() {
    init_builtin_types();
    init_builtin_filter();
    init_test_datatype();
}
