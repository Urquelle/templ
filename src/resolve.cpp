struct Type;
struct Sym;

internal_proc void   resolve_item(Item *item);
internal_proc Type * resolve_expr(Expr *expr);
internal_proc void   resolve_filter(Var_Filter filter);
internal_proc void   resolve_stmt(Stmt *stmt);

enum Val_Kind {
    VAL_NONE,
    VAL_BOOL,
    VAL_CHAR,
    VAL_INT,
    VAL_STR,
};

struct Val {
    Val_Kind kind;

    union {
        b32   bool_val;
        char  char_val;
        int   int_val;
        char *str_val;
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
val_str(char *val) {
    Val result = val_new(VAL_STR);

    result.str_val = val;

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
global_var Type *type_str;

internal_proc Type *
type_new(Type_Kind kind) {
    Type *result = (Type *)xmalloc(sizeof(Type));

    result->kind = kind;

    return result;
}

internal_proc Type *
type_struct(Type_Field **fields, size_t num_fields) {
    Type *result = type_new(TYPE_STRUCT);

    result->type_aggr.fields = fields;
    result->type_aggr.num_fields = num_fields;

    return result;
}

internal_proc Type *
type_proc(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_PROC);

    result->type_proc.params = params;
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;

    return result;
}

internal_proc Type *
type_filter(Type_Field **params, size_t num_params, Type *ret) {
    Type *result = type_new(TYPE_FILTER);

    result->type_proc.params = params;
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
init_types() {
    type_void = type_new(TYPE_VOID);
    type_bool = type_new(TYPE_BOOL);
    type_char = type_new(TYPE_CHAR);
    type_int  = type_new(TYPE_INT);
    type_str  = type_new(TYPE_STR);
}

internal_proc b32
is_int(Type *type) {
    b32 result = (type->kind == TYPE_INT || type->kind == TYPE_CHAR);

    return result;
}

struct Scope {
    char *name;
    Scope* parent;
    Scope* next;
    Sym**  syms;
    size_t num_syms;
};

global_var Scope global_scope;
global_var Scope *current_scope = &global_scope;

internal_proc void
scope_enter() {
    Scope *new_scope = 0;

    if ( !current_scope->next ) {
        new_scope = (Scope *)xcalloc(1, sizeof(Scope));
    } else {
        new_scope = current_scope->next;
        new_scope->syms = 0;
        new_scope->num_syms = 0;
    }

    current_scope->next = new_scope;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

internal_proc void
scope_leave() {
    if ( current_scope != &global_scope ) {
        current_scope = current_scope->parent;
    }
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
    for ( Scope *it = current_scope; it; it = it->parent ) {
        for ( int i = 0; i < it->num_syms; ++i ) {
            Sym *sym = it->syms[i];
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
    for ( int i = 0; i < current_scope->num_syms; ++i ) {
        if ( current_scope->syms[i]->name == name ) {
            assert(!"symbols existiert bereits");
        }
    }

    Sym *shadowed_sym = sym_get(name);
    if ( shadowed_sym ) {
        assert(!"warnung: symbol wird überschattet");
    }

    buf_push(current_scope->syms, sym_new(kind, name, type));
    current_scope->num_syms++;
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
        type_field("killwords", type_bool, val_bool(false)),
        type_field("end", type_str, val_str("...")),
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
            Type *type = resolve_expr(stmt->stmt_for.cond);
            sym_push_var(stmt->stmt_for.it, type);
            resolve_stmts(stmt->stmt_for.stmts, stmt->stmt_for.num_stmts);
            scope_leave();
        } break;

        case STMT_IF: {
            scope_enter();
            Type *type = resolve_expr(stmt->stmt_if.cond);
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

internal_proc Type *
resolve_expr_field(Expr *expr) {
    assert(expr->kind == EXPR_FIELD);
    Type *type = resolve_expr(expr->expr_field.expr);
    assert(type);

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
            return field->type;
        }
    }

    assert(!"kein passendes feld gefunden");
    return 0;
}

internal_proc Type *
resolve_expr(Expr *expr) {
    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
            if ( !sym ) {
                assert(!"konnte symbol nicht auflösen");
            }

            return sym->type;
        } break;

        case EXPR_STR: {
            return type_str;
        } break;

        case EXPR_INT: {
            return type_int;
        } break;

        case EXPR_PAREN: {
            return resolve_expr(expr->expr_paren.expr);
        } break;

        case EXPR_UNARY: {
            return resolve_expr(expr->expr_unary.expr);
        } break;

        case EXPR_BINARY: {
            Type *type_left  = resolve_expr(expr->expr_binary.left);
            Type *type_right = resolve_expr(expr->expr_binary.right);

            /* @TODO: typen überprüfen */
            return 0;
        } break;

        case EXPR_TERNARY: {
            Type *type_left = resolve_expr(expr->expr_ternary.left);
            Type *type_middle = resolve_expr(expr->expr_ternary.middle);
            Type *type_right = resolve_expr(expr->expr_ternary.right);

            /* @TODO: typen überprüfen */
            return 0;
        } break;

        case EXPR_FIELD: {
            return resolve_expr_field(expr);
        } break;

        case EXPR_RANGE: {
            Type *type_left  = resolve_expr(expr->expr_range.left);
            Type *type_right = resolve_expr(expr->expr_range.right);

            if ( !is_int(type_left) ) {
                assert(!"range typ muss vom typ int sein");
            }

            if ( !is_int(type_right) ) {
                assert(!"range typ muss vom typ int sein");
            }

            return type_int;
        } break;

        case EXPR_CALL: {
            Type *type = resolve_expr(expr->expr_call.expr);
            if ( type->kind != TYPE_PROC ) {
                assert(!"aufruf einer nicht-prozedur");
            }

            return type->type_proc.ret;
        } break;

        case EXPR_INDEX: {
            Type *type = resolve_expr(expr->expr_index.expr);
            if (type->kind != TYPE_ARRAY) {
                assert(!"indizierung auf einem nicht-array");
            }

            return type;
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

        if ( param->default_val.kind != VAL_NONE && (i-1 > filter.num_params) ) {
            assert(!"zu wenige parameter übergeben");
        }

        if ( i-1 < filter.num_params ) {
            Type *arg_type = resolve_expr(filter.params[i-1]);
            if (arg_type != param->type) {
                assert(!"datentyp des arguments stimmt nicht");
            }
        }
    }
}

internal_proc void
resolve_var(Item *item) {
    assert(item->kind == ITEM_VAR);

    Type *type = resolve_expr(item->item_var.expr);
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
    for ( int i = 0; i < d->num_items; ++i ) {
        resolve_item(d->items[i]);
    }
}

internal_proc void
init_test_datatype() {
    Type_Field *user_fields[] = { type_field("name", type_str) };
    Type *user_type = type_struct(user_fields, 1);
    sym_push_var("user", user_type);
}

internal_proc void
init_resolver() {
    init_types();
    init_builtin_filter();
    init_test_datatype();
}
