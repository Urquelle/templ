struct Type;
struct Sym;
struct Operand;

global_var Arena resolve_arena;

global_var Map resolved_expr;
internal_proc void
push_resolved_expr(Expr *expr, Operand *operand) {
    map_put(&resolved_expr, expr, operand);
}

internal_proc Operand *
fetch_resolved_expr(Expr *expr) {
    return (Operand *)map_get(&resolved_expr, expr);
}

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
set_this_doc_to_be_parent_of_current_scope(Doc *d) {
    current_doc->parent = d;
}

internal_proc void      resolve_item(Item *item);
internal_proc Operand * resolve_expr(Expr *expr);
internal_proc Operand * resolve_expr_cond(Expr *expr);
internal_proc void      resolve_filter(Var_Filter *filter);
internal_proc void      resolve_stmt(Stmt *stmt);
internal_proc void      resolve(Doc *d);

struct Scope {
    char*  name;
    Scope* parent;
    Frame* frame;
    Map    syms;
};

global_var Scope global_scope;
global_var Scope *current_scope = &global_scope;

internal_proc Scope *
scope_new(Scope *parent, char *name = NULL) {
    Scope *result = ALLOC_STRUCT(&resolve_arena, Scope);

    result->name   = name;
    result->parent = parent;
    result->frame  = frame_new(KB(1));
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

internal_proc void
scope_set(Scope *scope) {
    if ( scope ) {
        current_scope = scope;
    }
}

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
    size_t offset;
    Frame *frame;
};

global_var Val val_none;

internal_proc Val
val_new(Val_Kind kind, size_t offset) {
    Val result = {};

    result.kind   = kind;
    result.offset = offset;
    result.frame  = current_scope->frame;

    return result;
}

internal_proc Val
val_field(size_t offset) {
    return val_new(VAL_FIELD, offset);
}

internal_proc Val
val_bool(b32 val) {
    size_t size = 1;

    size_t offset = frame_set(current_scope->frame, &val, size);
    Val result = val_new(VAL_BOOL, offset);

    return result;
}

internal_proc Val
val_char(char val) {
    size_t size = 1;

    size_t offset = frame_set(current_scope->frame, &val, size);
    Val result = val_new(VAL_CHAR, offset);

    return result;
}

internal_proc Val
val_int(int val) {
    size_t size = 8;

    size_t offset = frame_set(current_scope->frame, &val, size);
    Val result = val_new(VAL_INT, offset);

    return result;
}

internal_proc Val
val_float(float val) {
    size_t size = 8;

    size_t offset = frame_set(current_scope->frame, &val, size);
    Val result = val_new(VAL_FLOAT, offset);

    return result;
}

internal_proc Val
val_str(char *val) {
    size_t size = 8;

    size_t offset = frame_set(current_scope->frame, val, strlen(val));
    Val result = val_new(VAL_STR, offset);

    return result;
}

internal_proc Val
val_range(int min, int max) {
    size_t size = 2*8;
    int t[] = {min, max};

    size_t offset = frame_set(current_scope->frame, t, size);
    Val result = val_new(VAL_RANGE, offset);

    return result;
}

internal_proc Val
val_struct(void *ptr, size_t size) {

    size_t offset = frame_set(current_scope->frame, ptr, size);
    Val result = val_new(VAL_STRUCT, offset);

    return result;
}

/* @TODO: ins frame verfrachten */
internal_proc void *
val_get(Val val) {
    void *result = (void *)(val.frame->mem + val.offset);

    return result;
}

/* @TODO: ins frame verfrachten */
internal_proc int
val_get_int(Val val) {
    int result = *(int *)val_get(val);

    return result;
}

/* @TODO: ins frame verfrachten */
internal_proc char *
val_get_str(Val val) {
    char *result = (char *)val_get(val);

    return result;
}

internal_proc Sym * sym_push_var(char *name, Type *type, Val val = val_none);

#define FILTER_CALLBACK(name) char * name(void *val, Expr **params, size_t num_params)
typedef FILTER_CALLBACK(Callback);

struct Type_Field {
    char *name;
    Type *type;
    s64   offset;
    Val   default_val;
};

internal_proc Type_Field *
type_field(char *name, Type *type, Val default_val = val_none) {
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
            Callback *callback;
        } type_proc;

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

global_var Type *arithmetic_result_type_table[3][3];
global_var Type *scalar_result_type_table[4][4];

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
type_filter(Type_Field **params, size_t num_params, Type *ret, Callback *callback) {
    Type *result = type_new(TYPE_FILTER);

    result->size = PTR_SIZE;
    result->type_proc.params = (Type_Field **)AST_DUP(params);
    result->type_proc.num_params = num_params;
    result->type_proc.ret = ret;
    result->type_proc.callback = callback;

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

struct Operand {
    Type* type;
    Val   val;
    Sym*  sym;

    b32 is_const;
    b32 is_lvalue;
};

internal_proc Operand *
operand_new(Type *type, Val val) {
    Operand *result = ALLOC_STRUCT(&resolve_arena, Operand);

    result->type      = type;
    result->val       = val;
    result->is_const  = false;
    result->is_lvalue = false;

    return result;
}

internal_proc Operand *
operand_lvalue(Type *type, Sym *sym = NULL, Val val = val_none) {
    Operand *result = operand_new(type, val);

    result->is_lvalue = true;
    result->sym = sym;

    return result;
}

internal_proc Operand *
operand_rvalue(Type *type, Val val = val_none) {
    Operand *result = operand_new(type, val);

    return result;
}

internal_proc Operand *
operand_const(Type *type, Val val) {
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

internal_proc b32
convert_operand(Operand *op, Type *dest_type) {
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
    Val   val;
};

internal_proc Sym *
sym_new(Sym_Kind kind, char *name, Type *type, Val val = val_none) {
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
sym_push(Sym_Kind kind, char *name, Type *type, Val val = val_none) {
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
sym_push_var(char *name, Type *type, Val val) {
    return sym_push(SYM_VAR, name, type, val);
}

internal_proc FILTER_CALLBACK(upper) {
    char *str = *(char **)val;
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

            size_t offset = frame_set(current_scope->frame, 0, operand->type->size);
            sym_push_var(stmt->stmt_for.it, operand->type, val_new(VAL_NONE, offset));

            resolve_stmts(stmt->stmt_for.stmts, stmt->stmt_for.num_stmts);
            scope_leave();
        } break;

        case STMT_IF: {
            scope_enter();
            Operand *operand = resolve_expr_cond(stmt->stmt_if.cond);
            resolve_stmts(stmt->stmt_if.stmts, stmt->stmt_if.num_stmts);
            scope_leave();

            if ( stmt->stmt_if.num_elseifs ) {
                for ( int i = 0; i < stmt->stmt_if.num_elseifs; ++i ) {
                    scope_enter();
                    Stmt *elseif = stmt->stmt_if.elseifs[i];
                    Operand *elseif_operand = resolve_expr_cond(elseif->stmt_if.cond);
                    resolve_stmts(elseif->stmt_if.stmts, elseif->stmt_if.num_stmts);
                    scope_leave();
                }
            }

            if ( stmt->stmt_if.else_stmt ) {
                scope_enter();
                Stmt *else_stmt = stmt->stmt_if.else_stmt;
                resolve_stmts(else_stmt->stmt_if.stmts, else_stmt->stmt_if.num_stmts);
                scope_leave();
            }
        } break;

        case STMT_BLOCK: {
            scope_enter(stmt->stmt_block.name);
            resolve_stmts(stmt->stmt_block.stmts, stmt->stmt_block.num_stmts);
            scope_leave();
        } break;

        case STMT_END:
        case STMT_LIT: {
            /* nichts zu tun */
        } break;

        case STMT_EXTENDS: {
            Doc *doc = parse_file(stmt->stmt_extends.name);
            resolve(doc);
            set_this_doc_to_be_parent_of_current_scope(doc);
        } break;

        case STMT_SET: {
            Sym *sym = resolve_name(stmt->stmt_set.name);
            Operand *operand = resolve_expr(stmt->stmt_set.expr);

            if ( !sym ) {
                sym_push_var(sym->name, operand->type, operand->val);
            } else {
                if ( !convert_operand(operand, operand->sym->type) ) {
                    assert(!"datentyp des operanden passt nicht");
                }
            }
        } break;

        case STMT_FILTER: {
            for ( int i = 0; i < stmt->stmt_filter.num_filter; ++i ) {
                resolve_filter(&stmt->stmt_filter.filter[i]);
            }
            resolve_stmts(stmt->stmt_filter.stmts, stmt->stmt_filter.num_stmts);
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc Operand *
resolve_expr_cond(Expr *expr) {
    Operand *operand = resolve_expr(expr);

    /* @TODO: char, int, str akzeptieren */
    if ( operand->type != type_bool ) {
        assert(!"boolischen ausdruck erwartet");
    }

    return operand;
}

internal_proc Operand *
resolve_expr(Expr *expr) {
    Operand *result = 0;

    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
            if ( !sym ) {
                assert(!"konnte symbol nicht auflösen");
            }

            result = operand_lvalue(sym->type, sym, sym->val);
        } break;

        case EXPR_STR: {
            result = operand_const(type_str, val_str(expr->expr_str.value));
        } break;

        case EXPR_INT: {
            result = operand_const(type_int, val_int(expr->expr_int.value));
        } break;

        case EXPR_FLOAT: {
            result = operand_const(type_float, val_float(expr->expr_float.value));
        } break;

        case EXPR_BOOL: {
            result = operand_const(type_bool, val_bool(expr->expr_bool.value));
        } break;

        case EXPR_PAREN: {
            result = resolve_expr(expr->expr_paren.expr);
        } break;

        case EXPR_UNARY: {
            result = resolve_expr(expr->expr_unary.expr);
        } break;

        case EXPR_BINARY: {
            Operand *left  = resolve_expr(expr->expr_binary.left);
            Operand *right = resolve_expr(expr->expr_binary.right);

            if ( is_eql(expr->expr_binary.op) ) {
                unify_scalar_operands(left, right);
            } else {
                unify_arithmetic_operands(left, right);
            }

            if ( is_cmp(expr->expr_binary.op) ) {
                result = operand_rvalue(type_bool);
            } else {
                result = operand_rvalue(left->type);
            }
        } break;

        case EXPR_TERNARY: {
            Operand *left   = resolve_expr_cond(expr->expr_ternary.left);
            Operand *middle = resolve_expr(expr->expr_ternary.middle);
            Operand *right  = resolve_expr(expr->expr_ternary.right);

            if ( middle->type != right->type ) {
                assert(!"beide datentypen der zuweisung müssen gleich sein");
            }

            result = operand_rvalue(middle->type);
        } break;

        case EXPR_FIELD: {
            Operand *base = resolve_expr(expr->expr_field.expr);

            assert(base->type);
            Type *type = base->type;
            assert(type->kind == TYPE_STRUCT);

            scope_set(type->type_aggr.scope);
            Operand *operand = resolve_expr(expr->expr_field.field);
            scope_set(type->type_aggr.scope->parent);

            result = operand;
        } break;

        case EXPR_RANGE: {
            Operand *left  = resolve_expr(expr->expr_range.left);
            Operand *right = resolve_expr(expr->expr_range.right);
            unify_arithmetic_operands(left, right);

            if ( !is_int(left->type) ) {
                assert(!"range typ muss vom typ int sein");
            }

            if ( !is_int(right->type) ) {
                assert(!"range typ muss vom typ int sein");
            }

            int min = val_get_int(left->val);
            int max = val_get_int(right->val);

            result = operand_rvalue(type_int, val_range(min, max));
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

            result = operand_rvalue(type->type_proc.ret);
        } break;

        case EXPR_INDEX: {
            Operand *operand = resolve_expr(expr->expr_index.expr);
            if (operand->type->kind != TYPE_ARRAY) {
                assert(!"indizierung auf einem nicht-array");
            }

            Operand *index = resolve_expr(expr->expr_index.index);
            if ( !is_int(index->type) ) {
                assert(!"index muss vom typ int sein");
            }

            result = operand_rvalue(operand->type);
        } break;

        default: {
            assert(0);
        } break;
    }

    push_resolved_expr(expr, result);

    return result;
}

internal_proc void
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

    for ( int i = 1; i < type->type_proc.num_params-1; ++i ) {
        Type_Field *param = type->type_proc.params[i];

        if ( param->default_val.kind == VAL_NONE && (i-1 >= filter->num_params) ) {
            assert(!"zu wenige parameter übergeben");
        }

        if ( i-1 < filter->num_params ) {
            Operand *arg = resolve_expr(filter->params[i-1]);
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
        resolve_filter(&item->item_var.filter[i]);
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

struct User {
    char *name;
    int age;
};

internal_proc void
init_test_datatype() {
    Type_Field *user_fields[] = { type_field("name", type_str), type_field("age", type_int) };
    Type *user_type = type_struct(user_fields, 2);

    User *user = (User *)xmalloc(sizeof(User));
    user->name = "Noob";
    user->age  = 40;

    Sym *sym = sym_push_var("user", user_type, val_struct(user, sizeof(User)));
}

internal_proc void
init_arenas() {
    arena_init(&resolve_arena, MB(100));
}

internal_proc void
init_global_scope() {
    global_scope.frame = frame_new(KB(1));
}

internal_proc void
init_resolver() {
    init_arenas();
    init_global_scope();
    init_builtin_types();
    init_builtin_filter();
    init_test_datatype();
}

internal_proc void
resolve(Doc *d) {
    Doc *prev_doc = doc_enter(d);
    for ( int i = 0; i < d->num_items; ++i ) {
        resolve_item(d->items[i]);
    }
    doc_leave(prev_doc);
}

