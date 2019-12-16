/* sym {{{ */
enum Sym_Kind {
    SYM_UNDEFINED,
    SYM_VAR,
    SYM_PROC,
    SYM_MODULE,
};

struct Sym {
    Sym_Kind kind;
    char *name;
    Type *type;
    Val  *val;
};

internal_proc b32
sym_valid(Sym *sym) {
    b32 result = sym && sym->kind != SYM_UNDEFINED;

    return result;
}

internal_proc b32
sym_invalid(Sym *sym) {
    b32 result = !sym || sym->kind == SYM_UNDEFINED;

    return result;
}

internal_proc Sym *
sym_new(Sym_Kind kind, char *name, Type *type, Val *val = val_undefined()) {
    Sym *result = ALLOC_STRUCT(&resolve_arena, Sym);

    result->kind  = kind;
    result->name  = name;
    result->type  = type;
    result->val   = val;

    return result;
}

internal_proc Sym *
sym_undefined() {
    Sym *result = sym_new(SYM_UNDEFINED, "", &type_undefined);

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

    return sym_undefined();
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

internal_proc char *
sym_name(Sym *sym) {
    char *result = sym->name;

    return result;
}

internal_proc Val *
sym_val(Sym *sym) {
    Val *result = sym->val;

    return result;
}

internal_proc void
sym_val(Sym *sym, Val *val) {
    sym->val = val;
}

internal_proc Type *
sym_type(Sym *sym) {
    Type *result = sym->type;

    return result;
}

internal_proc Sym *
sym_push(Sym_Kind kind, char *name, Type *type, Val *val = val_undefined()) {
    name = intern_str(name);

    Sym *sym = (Sym *)map_get(&current_scope->syms, name);
    if ( sym ) {
        return sym;
    }

    Sym *result = sym_new(kind, name, type, val);
    map_put(&current_scope->syms, name, result);

    buf_push(current_scope->sym_list, result);
    current_scope->num_syms = buf_len(current_scope->sym_list);

    return result;
}

internal_proc Sym *
sym_push_test(char *name, Type *type, Val *val) {
    Scope *prev_scope = scope_set(&tester_scope);
    Sym *result = sym_push(SYM_PROC, name, type, val);
    scope_set(prev_scope);

    return result;
}

internal_proc Sym *
sym_push_sysproc(char *name, Type *type, Val *val) {
    Scope *prev_scope = scope_set(&system_scope);
    Sym *result = sym_push(SYM_PROC, name, type, val);
    scope_set(prev_scope);

    return result;
}

internal_proc Sym *
sym_push_var(char *name, Type *type, Val *val) {
    return sym_push(SYM_VAR, name, type, val);
}

internal_proc Sym *
sym_push_proc(char *name, Type *type, Val *val) {
    return sym_push(SYM_PROC, name, type, val);
}

internal_proc Sym *
sym_push_module(char *name, Type *type, Val *val) {
    return sym_push(SYM_MODULE, name, type, val);
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

internal_proc char *
arg_name(Resolved_Arg *arg) {
    char *result = arg->name;

    return result;
}

internal_proc Pos
arg_pos(Resolved_Arg *arg) {
    Pos result = arg->pos;

    return result;
}

internal_proc Type *
arg_type(Resolved_Arg *arg) {
    Type *result = arg->type;

    return result;
}

internal_proc Val *
arg_val(Resolved_Arg *arg) {
    Val *result = arg->val;

    return result;
}
/* }}} */
/* resolved_expr {{{ */
struct Resolved_Expr {
    Expr_Kind kind;
    Pos pos;
    Val *val;
    Type *type;
    Resolved_Expr *if_expr;

    union {
        struct {
            char *name;
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
            char *field;
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
            Resolved_Expr **args;
            size_t num_args;
            Map *nargs;
            char **narg_keys;
            size_t num_narg_keys;
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
    };
};

internal_proc Pos
expr_pos(Resolved_Expr *expr) {
    Pos result = expr->pos;

    return result;
}

internal_proc Type *
expr_type(Resolved_Expr *expr) {
    Type *result = expr->type;

    return result;
}

internal_proc Val *
expr_val(Resolved_Expr *expr) {
    Val *result = expr->val;

    return result;
}

internal_proc Resolved_Expr *
expr_field_base(Resolved_Expr *expr) {
    Resolved_Expr *result = expr->expr_field.base;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_new(Expr_Kind kind, Type *type = 0) {
    Resolved_Expr *result = ALLOC_STRUCT(&resolve_arena, Resolved_Expr);

    result->type = type;
    result->kind = kind;
    result->val = val_undefined();

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_none() {
    Resolved_Expr *result = resolved_expr_new(EXPR_NONE);

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
resolved_expr_name(char *name, Val *val, Type *type) {
    Resolved_Expr *result = resolved_expr_new(EXPR_NAME, type);

    result->val = val;
    result->expr_name.name = name;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_field(Resolved_Expr *base, Val *val, Type *type, char *field) {
    Resolved_Expr *result = resolved_expr_new(EXPR_FIELD, type);

    result->val = val;
    result->expr_field.base  = base;
    result->expr_field.field = field;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_paren(Resolved_Expr *expr) {
    Resolved_Expr *result = resolved_expr_new(EXPR_PAREN, expr->type);

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
resolved_expr_binary(Token_Kind op, Resolved_Expr *left, Resolved_Expr *right, Type *type) {
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
resolved_expr_call(Resolved_Expr *expr, Resolved_Expr **args, size_t num_args,
        Map *nargs, char **narg_keys,
        size_t num_narg_keys, Resolved_Arg **kwargs,
        size_t num_kwargs, Resolved_Arg **varargs, size_t num_varargs,
        Type *type)
{
    Resolved_Expr *result = resolved_expr_new(EXPR_CALL, type);

    result->expr_call.expr        = expr;
    result->expr_call.args        = (Resolved_Expr **)AST_DUP(args);
    result->expr_call.num_args    = num_args;
    result->expr_call.nargs       = nargs;
    result->expr_call.narg_keys   = narg_keys;
    result->expr_call.num_narg_keys = num_narg_keys;
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
resolved_expr_list(Resolved_Expr **expr, size_t num_expr, Type *type, Val *val) {
    Resolved_Expr *result = resolved_expr_new(EXPR_LIST, type);

    result->val = val;
    result->expr_list.expr = expr;
    result->expr_list.num_expr = num_expr;

    return result;
}

internal_proc Resolved_Expr *
resolved_expr_dict(Val *val, Scope *scope) {
    Resolved_Expr *result = resolved_expr_new(EXPR_DICT, type_dict(scope));

    result->val = val;

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
        } stmt_var;

        struct {
            Scope *scope;
            Sym **args;
            size_t num_args;
            Resolved_Expr *expr;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_call;

        struct {
            Resolved_Expr *expr;
        } stmt_do;

        struct {
            Scope *scope;
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
            Sym *loop_depth;
            Sym *loop_depth0;
        } stmt_for;

        struct {
            Resolved_Expr *expr;
            Resolved_Stmt **stmts;
            size_t num_stmts;
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
            Resolved_Expr *name;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_set_block;

        struct {
            Resolved_Expr *filter;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_filter;

        struct {
            Resolved_Templ **templ;
            size_t num_templ;
        } stmt_include;

        struct {
            Resolved_Expr  *name;
            Resolved_Templ *tmpl;
            Resolved_Templ *else_tmpl;
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
            Scope *scope;
            Resolved_Stmt **stmts;
            size_t num_stmts;
        } stmt_with;
    };
};

internal_proc s32
stmt_kind(Resolved_Stmt *stmt) {
    s32 result = stmt->kind;

    return result;
}

internal_proc size_t
stmt_block_num_stmts(Resolved_Stmt *stmt) {
    size_t result = stmt->stmt_block.num_stmts;

    return result;
}

internal_proc Resolved_Stmt **
stmt_block_stmts(Resolved_Stmt *stmt) {
    Resolved_Stmt **result = stmt->stmt_block.stmts;

    return result;
}

internal_proc size_t
stmt_call_num_stmts(Resolved_Stmt *stmt) {
    size_t result = stmt->stmt_call.num_stmts;

    return result;
}

internal_proc Resolved_Stmt **
stmt_call_stmts(Resolved_Stmt *stmt) {
    Resolved_Stmt **result = stmt->stmt_call.stmts;

    return result;
}

internal_proc size_t
stmt_call_num_args(Resolved_Stmt *stmt) {
    size_t result = stmt->stmt_call.num_args;

    return result;
}

internal_proc Sym **
stmt_call_args(Resolved_Stmt *stmt) {
    Sym **result = stmt->stmt_call.args;

    return result;
}

internal_proc size_t
stmt_for_num_else_stmts(Resolved_Stmt *stmt) {
    size_t result = stmt->stmt_for.num_else_stmts;

    return result;
}

internal_proc Resolved_Stmt **
stmt_for_else_stmts(Resolved_Stmt *stmt) {
    Resolved_Stmt **result = stmt->stmt_for.else_stmts;

    return result;
}

internal_proc size_t
stmt_for_num_stmts(Resolved_Stmt *stmt) {
    size_t result = stmt->stmt_for.num_stmts;

    return result;
}

internal_proc Resolved_Stmt **
stmt_for_stmts(Resolved_Stmt *stmt) {
    Resolved_Stmt **result = stmt->stmt_for.stmts;

    return result;
}

internal_proc size_t
stmt_for_num_vars(Resolved_Stmt *stmt) {
    size_t result = stmt->stmt_for.num_vars;

    return result;
}

internal_proc Sym **
stmt_for_vars(Resolved_Stmt *stmt) {
    Sym **result = stmt->stmt_for.vars;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_new(Stmt_Kind kind) {
    Resolved_Stmt *result = ALLOC_STRUCT(&resolve_arena, Resolved_Stmt);

    result->kind = kind;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_call(Scope *scope, Sym **args, size_t num_args,
        Resolved_Expr *expr, Resolved_Stmt **stmts, size_t num_stmts)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_CALL);

    result->stmt_call.scope = scope;
    result->stmt_call.args = args;
    result->stmt_call.num_args = num_args;
    result->stmt_call.expr = expr;
    result->stmt_call.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_call.num_stmts = num_stmts;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_do(Resolved_Expr *expr) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_DO);

    result->stmt_do.expr = expr;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_var(Resolved_Expr *expr) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_VAR);

    result->stmt_var.expr    = expr;

    return result;
}

internal_proc Resolved_Stmt *
resolved_stmt_if(Resolved_Expr *expr, Resolved_Stmt **stmts, size_t num_stmts,
        Resolved_Stmt *else_stmt)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_IF);

    result->stmt_if.expr = expr;
    result->stmt_if.stmts = (Resolved_Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;
    result->stmt_if.else_stmt = else_stmt;

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
resolved_stmt_for(Scope *scope, Sym **vars, size_t num_vars, Resolved_Expr *set,
        Resolved_Stmt **stmts, size_t num_stmts, Resolved_Stmt **else_stmts,
        size_t num_else_stmts,
        Sym *loop_index, Sym *loop_index0, Sym *loop_revindex, Sym *loop_revindex0,
        Sym *loop_first, Sym *loop_last, Sym *loop_length, Sym *loop_cycle,
        Sym *loop_depth, Sym *loop_depth0)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_FOR);

    result->stmt_for.scope = scope;
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
    result->stmt_for.loop_depth     = loop_depth;
    result->stmt_for.loop_depth0    = loop_depth0;

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
resolved_stmt_set_block(Resolved_Expr *name, Resolved_Stmt **stmts,
        size_t num_stmts)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_SET_BLOCK);

    result->stmt_set_block.name = name;
    result->stmt_set_block.stmts = stmts;
    result->stmt_set_block.num_stmts = num_stmts;

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
resolved_stmt_filter(Resolved_Expr *filter,
        Resolved_Stmt **stmts, size_t num_stmts)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_FILTER);

    result->stmt_filter.filter    = filter;
    result->stmt_filter.stmts     = stmts;
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
resolved_stmt_extends(Resolved_Expr *name, Resolved_Templ *tmpl,
        Resolved_Templ *else_tmpl)
{
    Resolved_Stmt *result = resolved_stmt_new(STMT_EXTENDS);

    result->stmt_extends.name = name;
    result->stmt_extends.tmpl = tmpl;
    result->stmt_extends.else_tmpl = else_tmpl;

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
resolved_stmt_with(Scope *scope, Resolved_Stmt **stmts, size_t num_stmts) {
    Resolved_Stmt *result = resolved_stmt_new(STMT_WITH);

    result->stmt_with.scope = scope;
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
    Scope *scope;
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
internal_proc Expr *
resolve_extract_expr_name(Expr *expr) {
    Expr *result = 0;

    switch ( expr->kind ) {
        case EXPR_NAME: {
            result = expr;
        } break;

        case EXPR_CALL: {
            result = resolve_extract_expr_name(expr->expr_call.expr);
        } break;

        case EXPR_FIELD: {
            result = resolve_extract_expr_name(expr->expr_field.expr);
        } break;

        default: {
            illegal_path();
        } break;
    }

    return result;
}

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
            result = resolved_stmt_var(expr);
        } break;

        case STMT_FOR: {
            Scope *scope_for = scope_enter("for");

            Resolved_Expr *set = resolve_expr(stmt->stmt_for.set);
            Type *set_type = type_base(set->type);

            Sym **vars = 0;
            size_t num_vars = 0;
            for ( int i = 0; i < stmt->stmt_for.num_vars; ++i ) {
                ASSERT(stmt->stmt_for.vars[i]->kind == EXPR_NAME);
                Sym *sym = sym_push_var(stmt->stmt_for.vars[i]->expr_name.value, set_type, val_undefined());
                buf_push(vars, sym);
            }

            num_vars = buf_len(vars);

            /* loop variablen {{{ */
            Type_Field *any_type[] = { type_field("s", type_any) };
            Type_Field *loop_type[] = { type_field("s", type_any) };

            Scope *scope = scope_new(current_scope, "loop");

            Type *type = type_proc(loop_type, 1, 0);
            type->scope = scope;
            type->scope->parent = &type_any_scope;

            Val *val = val_proc(loop_type, 1, 0, proc_loop);
            val->scope = scope;
            val->scope->parent = &type_any_scope;

            sym_push_var(symname_loop, type, val);
            Scope *prev_scope = scope_set(scope);

            Sym *loop_index     = sym_push_var(symname_index, type_int,  val_int(1));
            Sym *loop_index0    = sym_push_var("index0",      type_int,  val_int(0));
            Sym *loop_revindex  = sym_push_var("revindex",    type_int,  val_int(0));
            Sym *loop_revindex0 = sym_push_var("revindex0",   type_int,  val_int(0));
            Sym *loop_first     = sym_push_var("first",       type_bool, val_bool(true));
            Sym *loop_last      = sym_push_var("last",        type_bool, val_bool(false));
            Sym *loop_length    = sym_push_var("length",      type_int,  val_int(0));
            Sym *loop_cycle     = sym_push_proc("cycle",      type_proc(any_type, 0, 0), val_proc(any_type, 0, 0, proc_cycle));
            Sym *loop_depth     = sym_push_var("depth",       type_int,  val_int(1));
            Sym *loop_depth0    = sym_push_var("depth0",      type_int,  val_int(0));

            scope_set(prev_scope);
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

            result = resolved_stmt_for(scope_for, vars, num_vars, set, stmts,
                    buf_len(stmts), else_stmts, buf_len(else_stmts),
                    loop_index, loop_index0, loop_revindex, loop_revindex0,
                    loop_first, loop_last, loop_length, loop_cycle, loop_depth,
                    loop_depth0);
        } break;

        case STMT_ELSE:
        case STMT_IF: {
            scope_enter();
            Resolved_Expr *expr = resolve_expr_cond(stmt->stmt_if.cond);

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_if.stmts[i], templ));
            }
            scope_leave();

            Resolved_Stmt *else_stmt = 0;
            if ( stmt->stmt_if.else_stmt ) {
                else_stmt = resolve_stmt(stmt->stmt_if.else_stmt, templ);
            }

            result = resolved_stmt_if(expr, stmts, buf_len(stmts), else_stmt);
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

        case STMT_CALL: {
            Resolved_Expr *expr = resolve_expr(stmt->stmt_call.expr);
            Scope *scope = scope_enter("call");

            Sym **args = 0;
            for ( int i = 0; i < stmt->stmt_call.num_args; ++i ) {
                Resolved_Expr *arg = resolve_expr(stmt->stmt_call.args[i]);
                assert(arg->kind == EXPR_NAME);
                buf_push(args, sym_push_var(arg->expr_name.name, arg->type, val_undefined()));
            }

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_call.num_stmts; ++i ) {
                Resolved_Stmt *resolved_stmt = resolve_stmt(stmt->stmt_call.stmts[i], templ);
                buf_push(stmts, resolved_stmt);
            }

            scope_leave();
            result = resolved_stmt_call(scope, args, buf_len(args), expr, stmts, buf_len(stmts));
        } break;

        case STMT_DO: {
            Resolved_Expr *expr = resolve_expr(stmt->stmt_do.expr);

            result = resolved_stmt_do(expr);
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
                sym_push_proc(name, type_proc(0, 0, 0), val_proc(0, 0, 0, proc_super));
            }

            Resolved_Templ *prev_templ = current_templ;

            Resolved_Expr *name_expr = resolve_expr(stmt->stmt_extends.name);
            Resolved_Templ *t = resolve(stmt->stmt_extends.templ);
            Resolved_Templ *else_templ = 0;
            if ( stmt->stmt_extends.else_templ ) {
                else_templ = resolve(stmt->stmt_extends.else_templ);
            }

            current_templ = prev_templ;

            result = resolved_stmt_extends(name_expr, t, else_templ);
        } break;

        case STMT_SET: {
            Resolved_Expr *expr = resolve_expr(stmt->stmt_set.expr);

            Resolved_Expr **names = 0;
            for ( int i = 0; i < stmt->stmt_set.num_names; ++i ) {
                Expr *name = stmt->stmt_set.names[i];
                Resolved_Expr *resolved_name = resolve_expr(name);
                resolved_name->type = expr->type;
                sym_push_var(name->expr_name.value, expr->type, val_undefined());
                buf_push(names, resolved_name);
            }

            result = resolved_stmt_set(names, buf_len(names), expr);
        } break;

        case STMT_SET_BLOCK: {
            Expr *expr = stmt->stmt_set_block.name;
            Resolved_Expr *resolved_expr = resolve_expr(expr);

            Expr *name = resolve_extract_expr_name(expr);
            sym_push_var(name->expr_name.value, &type_undefined, val_undefined());

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_set_block.num_stmts; ++i ) {
                Stmt *set_stmt = stmt->stmt_set_block.stmts[i];
                Resolved_Stmt *resolved_stmt = resolve_stmt(set_stmt, templ);
                buf_push(stmts, resolved_stmt);
            }

            result = resolved_stmt_set_block(resolved_expr, stmts, buf_len(stmts));
        } break;

        case STMT_FILTER: {
            Resolved_Expr *filter = resolve_filter(stmt->stmt_filter.filter);

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_filter.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_filter.stmts[i], templ));
            }

            result = resolved_stmt_filter(filter, stmts, buf_len(stmts));
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
                Type *default_type = 0;
                if ( param->default_value ) {
                    Resolved_Expr *t = resolve_expr(param->default_value);
                    default_value = t->val;
                    default_type = t->type;
                }

                buf_push(params, type_field(param->name, (default_type) ? default_type : type_any, default_value));
            }

            Val *val = val_proc(params, buf_len(params), 0, proc_exec_macro);
            Type *type = type_proc(params, buf_len(params), 0);

            char *macro_name = ( stmt->stmt_macro.alias ) ? stmt->stmt_macro.alias : stmt->stmt_macro.name;
            Sym *sym = sym_push_proc(macro_name, type, val);
            Scope *scope = scope_enter(macro_name);

            type->scope = scope;
            val->scope = scope;

            Val **param_names = 0;
            for ( int i = 0; i < buf_len(params); ++i ) {
                params[i]->sym = sym_push_var(params[i]->name, params[i]->type);
                buf_push(param_names, val_str(params[i]->name));
            }
            size_t num_param_names = buf_len(param_names);

            /* macro variablen {{{ */
            sym_push_var("name",      type_str, val_str(macro_name));
            sym_push_var("arguments", type_tuple(num_param_names), val_tuple(param_names, num_param_names));
            sym_push_var("varargs",   type_list(type_any), val_undefined());
            sym_push_var("caller",    type_proc(0, 0, type_str), val_proc(0, 0, type_str, proc_caller));

            Val **default_values = 0;
            for ( int i = 0; i < type->type_proc.num_params; ++i ) {
                Type_Field *field = type->type_proc.params[i];
                buf_push(default_values, field->default_value);
            }
            size_t num_default_values = buf_len(default_values);
            sym_push_var("defaults", type_tuple(num_default_values), val_tuple(default_values, num_default_values));
            /* }}} */

            Resolved_Stmt **stmts = 0;
            for ( int i = 0; i < stmt->stmt_macro.num_stmts; ++i ) {
                buf_push(stmts, resolve_stmt(stmt->stmt_macro.stmts[i], templ));
            }

            Val_Proc *proc_data = (Val_Proc *)val->ptr;
            proc_data->stmts = stmts;
            proc_data->num_stmts = buf_len(stmts);

            scope_leave();

            result = resolved_stmt_macro(sym, type);
        } break;

        case STMT_IMPORT: {
            Type *type = type_dict(0);
            Val *val = val_dict(0);

            Sym *sym = sym_push_module(stmt->stmt_import.name, type, val);
            Resolved_Templ *prev_templ = current_templ;

            Scope *scope = scope_new(&system_scope, stmt->stmt_import.name);
            Scope *prev_scope = scope_set(scope);

            scope->parent = &type_dict_scope;
            type->scope = scope;
            val->scope = scope;

            Resolved_Templ *t = resolve(stmt->stmt_import.templ);

            scope_set(prev_scope);

            current_templ = prev_templ;
            result = resolved_stmt_import(sym, t->stmts, t->num_stmts);
        } break;

        case STMT_FROM_IMPORT: {
            Parsed_Templ *t = stmt->stmt_from_import.templ;
            Resolved_Stmt **stmts = 0;

            /* @AUFGABE: with/without context angabe beachten */
            for ( int i = 0; i < t->num_stmts; ++i ) {
                Stmt *parsed_stmt = t->stmts[i];

                for ( int j = 0; j < stmt->stmt_from_import.num_syms; ++j ) {
                    Imported_Sym *import_sym = stmt->stmt_from_import.syms[j];

                    erstes_if ( parsed_stmt->kind == STMT_LIT ) {
                        continue;
                    } else if ( parsed_stmt->kind == STMT_MACRO ) {
                        if ( import_sym->name == parsed_stmt->stmt_macro.name ) {
                            parsed_stmt->stmt_macro.alias = import_sym->alias;
                            resolve_stmt(parsed_stmt, templ);
                        }
                    } else {
                        ASSERT(parsed_stmt->kind == STMT_SET);
                        for ( int k = 0; k < parsed_stmt->stmt_set.num_names; ++k ) {
                            Expr *name = parsed_stmt->stmt_set.names[k];
                            ASSERT(name->kind == EXPR_NAME);

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
            Scope *scope = scope_enter("with");

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

            result = resolved_stmt_with(scope, stmts, buf_len(stmts));
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
        case STMT_ENDCALL:
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
        fatal(expr->pos.name, expr->pos.line, "boolischen ausdruck erwartet");
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
    ASSERT(type);

    Type_Field **proc_params = type->type_proc.params;
    size_t num_proc_params = type->type_proc.num_params;
    Type *type_ret = type->type_proc.ret;

    Resolved_Expr **args = 0;
    /* benamte argumente */
    Map *nargs = ALLOC_STRUCT(&resolve_arena, Map);
    char **narg_keys = 0;
    /* benamte argumente, die nicht im typen definiert wurden */
    Resolved_Arg **kwargs = 0;
    /* mehr übergebener argumente als vom typ vorgegeben */
    Resolved_Arg **varargs = 0;
    b32 must_be_named = false;

    /* @INFO: alle übergebenen argumente durchgehen */
    for ( int i = 0; i < expr->expr_call.num_args; ++i ) {
        Arg *arg = expr->expr_call.args[i];

        if ( must_be_named && !arg->name ) {
            fatal(expr->pos.name, expr->pos.line, "nach benamten parameter müssen alle folgende parameter benamt sein");
        }

        char *name = 0;
        if ( arg->name ) {
            name = arg->name;
        }

        Resolved_Expr *arg_expr = resolve_expr(arg->expr);
        buf_push(args, arg_expr);

        /* @INFO: wenn benamtes argument */
        if ( name ) {
            must_be_named = true;

            Type_Field *matching_param = 0;
            for ( int j = 0; j < num_proc_params; ++j ) {
                Type_Field *param = proc_params[j];
                if ( name == param->name ) {
                    matching_param = param;
                    break;
                }
            }

            if ( matching_param ) {
                map_put(nargs, name, resolved_arg(expr->pos, name, arg_expr->type, arg_expr->val));
                buf_push(narg_keys, name);
            } else {
                Resolved_Arg *kwarg = resolved_arg(expr->pos, name, arg_expr->type, arg_expr->val);
                buf_push(kwargs, kwarg);
            }

        /* @INFO: wenn unbenamtes argument */
        } else {
            /* @INFO: mehr positionsargumente als im typ angegeben kommen
             *        in die varargs sammlung
             */
            if ( i >= num_proc_params ) {
                buf_push(varargs, resolved_arg(expr->pos, 0, arg_expr->type, arg_expr->val));

            /* @INFO: namen für positionsargument aus dem typ ermitteln */
            } else {
                Type_Field *param = 0;
                size_t num_non_default_params = 0;
                for ( int j = 0; j < num_proc_params; ++j ) {
                    if ( val_is_undefined(proc_params[j]->default_value) ) {
                        ++num_non_default_params;
                    }
                }

                for ( int j = 0; j < num_proc_params; ++j ) {
                    /* @INFO: ersten parameter ohne standardwert ermitteln und diesen nehmen, falls
                     *        er nicht bereits in der nargs map eingetragen ist.
                     */
                    size_t num_remaining_args = expr->expr_call.num_args - i;
                    if ( num_remaining_args > num_non_default_params ||
                            val_is_undefined(proc_params[j]->default_value) )
                    {
                        if ( !map_get(nargs, proc_params[j]->name) ) {
                            param = proc_params[j];
                            break;
                        }
                    }
                }

                if ( !param ) {
                    param = proc_params[i];
                }

                ASSERT(param);
                Resolved_Arg *r_arg = resolved_arg(expr->pos, param->name, arg_expr->type, arg_expr->val);
                map_put(nargs, param->name, r_arg);
                buf_push(narg_keys, param->name);
            }
        }
    }

    /* @INFO: alle typparameter durchgehen und für nicht gesetzte
     *        parameter standardwerte setzen
     */
    for ( int i = 0; i < num_proc_params; ++i ) {
        Type_Field *param = proc_params[i];

        if ( !map_get(nargs, param->name) ) {
            map_put(nargs, param->name, resolved_arg(expr->pos, param->name, param->type, param->default_value));
            buf_push(narg_keys, param->name);
        }
    }

    return resolved_expr_call(call_expr, args, buf_len(args), nargs, narg_keys,
            buf_len(narg_keys), kwargs, buf_len(kwargs), varargs, buf_len(varargs), type_ret);
}

internal_proc Resolved_Expr *
resolve_expr(Expr *expr) {
    Resolved_Expr *result = 0;

    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
            result = resolved_expr_name(expr->expr_name.value, sym->val, sym->type);
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

            Type *type = 0;
            erstes_if (is_cmp(expr->expr_binary.op)) {
                type = type_bool;
            } else {
                type = left->type;
            }

            result = resolved_expr_binary(expr->expr_binary.op, left, right, type);
        } break;

        case EXPR_FIELD: {
            Resolved_Expr *base = resolve_expr(expr->expr_field.expr);
            Type *type = base->type;

            if ( type ) {
                Type *base_type = type_base(base->type);
                Scope *scope = type->scope;

                if ( scope ) {
                    Sym *sym = scope_attr(scope, expr->expr_field.field);

                    if ( sym_invalid(sym) && base_type ) {
                        sym = scope_attr(base_type->scope, expr->expr_field.field);
                    }

                    if ( sym_invalid(sym) ) {
                        fatal(expr->pos.name, expr->pos.line, "unknown symbol %s", expr->expr_field.field);
                    }

                    result = resolved_expr_field(base, sym->val, sym->type, expr->expr_field.field);
                } else {
                    result = resolved_expr_field(base, val_undefined(), type_any, expr->expr_field.field);
                }
            } else {
                fatal(expr->pos.name, expr->pos.line, "unknown datatype");
                result = resolved_expr_field(base, val_undefined(), type_any, expr->expr_field.field);
            }
        } break;

        case EXPR_RANGE: {
            Resolved_Expr *left  = resolve_expr(expr->expr_range.left);
            Resolved_Expr *right = resolve_expr(expr->expr_range.right);

            if ( type_is_arithmetic(left->type) && type_is_arithmetic(right->type) ) {
                s32 min = val_int(left->val);
                s32 max = val_int(right->val);

                result = resolved_expr_range(min, max);
            } else {
                ASSERT(left->type->kind == TYPE_STR);
                ASSERT(right->type->kind == TYPE_STR);
                ASSERT(left->val && left->val->len == 1);

                result = resolved_expr_range((s32)*val_str(left->val), (s32)*val_str(right->val));
            }
        } break;

        case EXPR_CALL: {
            result = resolve_expr_call(expr);
        } break;

        case EXPR_SUBSCRIPT: {
            Resolved_Expr *resolved_expr  = resolve_expr(expr->expr_subscript.expr);
            Resolved_Expr *resolved_index = resolve_expr(expr->expr_subscript.index);

            result = resolved_expr_subscript(resolved_expr, resolved_index, resolved_expr->type);
        } break;

        case EXPR_IS: {
            Resolved_Expr *operand = resolve_expr(expr->expr_is.operand);
            Resolved_Expr *tester  = resolve_tester(expr->expr_is.tester);

            ASSERT(tester);
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

            result = resolved_expr_list(index, buf_len(index), type_list(type_any), val_list(vals, buf_len(vals)));
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

            for ( int i = 0; i < expr->expr_dict.num_pairs; ++i ) {
                Pair *pair = expr->expr_dict.pairs[i];
                Resolved_Expr *value = resolve_expr(pair->value);
                sym_push_var(pair->key, value->type, value->val);
            }

            scope_set(prev_scope);
            result = resolved_expr_dict(val_dict(scope), scope);
        } break;

        case EXPR_NONE: {
            result = resolved_expr_none();
        } break;

        default: {
            fatal(expr->pos.name, expr->pos.line, "nicht unterstützter ausdruck");
        } break;
    }

    Resolved_Expr *if_expr = 0;
    if ( expr->if_expr ) {
        if_expr = resolve_expr(expr->if_expr);
    }

    result->if_expr = if_expr;
    result->pos = expr->pos;

    return result;
}

internal_proc Resolved_Expr *
resolve_tester(Expr *expr) {
    ASSERT(expr->kind == EXPR_CALL);
    Resolved_Expr *result = resolve_expr_call(expr, &tester_scope);

    return result;
}

internal_proc Resolved_Expr *
resolve_filter(Expr *expr) {
    ASSERT(expr->kind == EXPR_CALL);
    Resolved_Expr *result = resolve_expr_call(expr, &filter_scope);

    return result;
}

internal_proc void
resolve_init_builtin_testers() {
    Type_Field *divby_type[] = { type_field("num", type_int) };
    Type_Field *int_type[]  = { type_field("s",    type_int) };
    Type_Field *any_type[]  = { type_field("s",    type_any) };

    sym_push_test("callable",    type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_callable));
    sym_push_test("defined",     type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_defined));
    sym_push_test("divisibleby", type_proc(divby_type, 1, type_bool), val_proc(divby_type,  1, type_bool, test_divisibleby));
    sym_push_test("eq",          type_proc(int_type,   1, type_bool), val_proc(int_type,    1, type_bool, test_eq));
    sym_push_test("escaped",     type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_escaped));
    sym_push_test("even",        type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_even));
    sym_push_test("ge",          type_proc(int_type,   1, type_bool), val_proc(int_type,    1, type_bool, test_ge));
    sym_push_test("gt",          type_proc(int_type,   1, type_bool), val_proc(int_type,    1, type_bool, test_gt));
    sym_push_test("in",          type_proc(any_type,   1, type_bool), val_proc(any_type,    1, type_bool, test_in));
    sym_push_test("iterable",    type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_iterable));
    sym_push_test("le",          type_proc(int_type,   1, type_bool), val_proc(int_type,    1, type_bool, test_le));
    sym_push_test("lt",          type_proc(int_type,   1, type_bool), val_proc(int_type,    1, type_bool, test_lt));
    sym_push_test("mapping",     type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_mapping));
    sym_push_test("ne",          type_proc(int_type,   1, type_bool), val_proc(int_type,    1, type_bool, test_ne));
    sym_push_test("none",        type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_none));
    sym_push_test("number",      type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_number));
    sym_push_test("odd",         type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_odd));
    sym_push_test("sameas",      type_proc(any_type,   1, type_bool), val_proc(any_type,    1, type_bool, test_sameas));
    sym_push_test("sequence",    type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_sequence));
    sym_push_test("string",      type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_string));
    sym_push_test("undefined",   type_proc(0,          0, type_bool), val_proc(0,           0, type_bool, test_undefined));
}

internal_proc void
resolve_init_builtin_procs() {
    Type_Field *range_args[]  = { type_field("start", type_int, val_int(0)), type_field("stop", type_int), type_field("step", type_int, val_int(1)) };
    Type_Field *lipsum_args[] = { type_field("n",     type_int, val_int(5)), type_field("html", type_bool, val_bool(true)), type_field("min", type_int, val_int(20)), type_field("max", type_int, val_int(100)) };
    Type_Field *joiner_args[] = { type_field("sep",   type_str, val_str(",")) };

    Scope *cycler_scope = scope_new(0, "cycler");
    Scope *prev_scope = scope_set(cycler_scope);
    sym_push_proc("next",    type_proc(0, 0, type_any), val_proc(0, 0, type_any, proc_cycler_next));
    sym_push_proc("reset",   type_proc(0, 0,        0), val_proc(0, 0, 0,        proc_cycler_reset));
    sym_push_var("current",  type_any);
    scope_set(prev_scope);

    Type *cycler_type = type_dict(cycler_scope);
    sym_push_sysproc("cycler", type_proc(0, 0, cycler_type), val_proc(0, 0, cycler_type, proc_cycler));

    Scope *local_dict_scope = scope_new(0, "dict");

    sym_push_sysproc("range", type_proc(range_args, 3, type_range), val_proc(range_args, 3, type_range, proc_range));
    sym_push_sysproc("lipsum", type_proc(lipsum_args, 4, type_str), val_proc(lipsum_args, 4, type_str, proc_lipsum));
    sym_push_sysproc("dict", type_proc(0, 0, type_dict(local_dict_scope)), val_proc(0, 0, type_dict(local_dict_scope), proc_dict));
    sym_push_sysproc("joiner", type_proc(joiner_args, 1, type_proc(0, 0, type_str)), val_proc(joiner_args, 1, type_proc(0, 0, type_str), proc_joiner));

    Scope *ns_scope = scope_new(0, "namespace");
    sym_push_sysproc("namespace", type_proc(0, 0, type_dict(ns_scope)), val_proc(0, 0, type_dict(ns_scope), proc_namespace));
}

internal_proc void
resolve_init_builtin_type_procs() {
    Scope *prev_scope = current_scope;

    /* @INFO: any methoden {{{ */
    Type_Field *default_type[] = { type_field("s", type_str), type_field("boolean",  type_bool, val_bool(false)) };
    size_t dt_size = ARRAY_SIZE(default_type);

    Type_Field *max_type[]     = {
        type_field("case_sensitive", type_bool, val_bool(false)),
        type_field("attribute",      type_str, val_none())
    };
    size_t mx_size = ARRAY_SIZE(max_type);

    Type_Field *pprint_type[] = { type_field("verbose", type_bool, val_bool(false)) };
    size_t pp_size = ARRAY_SIZE(pprint_type);

    Type_Field *tojson_type[] = { type_field("indent", type_bool, val_none()) };
    size_t tj_size = ARRAY_SIZE(tojson_type);

    scope_set(&type_any_scope);
    sym_push_proc("default",    type_proc(default_type, dt_size, type_str), val_proc(default_type, dt_size, type_str, proc_any_default));
    sym_push_proc("d",          type_proc(default_type, dt_size, type_str), val_proc(default_type, dt_size, type_str, proc_any_default));
    sym_push_proc("list",       type_proc(0,            0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_any_list));
    sym_push_proc("max",        type_proc(max_type,     mx_size, type_any), val_proc(max_type, mx_size, type_any, proc_any_max));
    sym_push_proc("min",        type_proc(max_type,     mx_size, type_any), val_proc(max_type, mx_size, type_any, proc_any_min));
    sym_push_proc("pprint",     type_proc(pprint_type,  pp_size, type_str), val_proc(pprint_type, pp_size, type_str, proc_any_pprint));
    sym_push_proc("safe",       type_proc(0,            0,       type_any), val_proc(0, 0, type_any, proc_any_safe));
    sym_push_proc("tojson",     type_proc(tojson_type,  tj_size, type_str), val_proc(tojson_type, tj_size, type_str, proc_any_tojson));
    /* }}} */
    /* @INFO: sequence methoden {{{ */
    Type_Field *batch_type[]  = { type_field("line_count", type_int), type_field("fill_with", type_str, val_none()) };
    size_t bt_size = ARRAY_SIZE(batch_type);

    Type_Field *groupby_type[] = { type_field("attribute", type_str) };
    size_t gb_size = ARRAY_SIZE(groupby_type);

    Type_Field *join_type[] = {
        type_field("d",          type_str,  val_str("")),
        type_field("attribute", &type_none, val_none())
    };
    size_t j_size = ARRAY_SIZE(join_type);

    Type_Field *replace_type[] = {
        type_field("old", type_str),
        type_field("new", type_str),
        type_field("count", type_int, val_none())
    };
    size_t re_size = ARRAY_SIZE(replace_type);

    Type_Field *sort_type[] = {
        type_field("reverse", type_bool, val_bool(false)),
        type_field("case_sensitive", type_bool, val_bool(false)),
        type_field("attribute", type_str, val_none())
    };
    size_t so_size = ARRAY_SIZE(sort_type);

    Type_Field *unique_type[] = {
        type_field("case_sensitive", type_bool, val_bool(false)),
        type_field("attribute", type_str, val_none())
    };
    size_t uq_size = ARRAY_SIZE(unique_type);

    Scope *groupby_scope = scope_new(0, "groupby");
    scope_set(groupby_scope);
    sym_push_var("grouper", type_str);
    sym_push_var("list", type_dict(0));
    Type *groupby_ret = type_list(type_dict(groupby_scope));

    scope_set(&type_sequence_scope);
    sym_push_proc("batch", type_proc(batch_type, bt_size, type_str), val_proc(batch_type, bt_size, type_str, proc_seq_batch));
    sym_push_proc("count", type_proc(0, 0, type_int), val_proc(0, 0, type_int, proc_seq_length));
    sym_push_proc("first", type_proc(0, 0, type_str), val_proc(0, 0, type_str, proc_seq_first));
    sym_push_proc("groupby", type_proc(groupby_type, gb_size, groupby_ret), val_proc(groupby_type, gb_size, groupby_ret, proc_seq_groupby));
    sym_push_proc("join", type_proc(join_type, j_size, type_str), val_proc(join_type, j_size, type_str, proc_seq_join));
    sym_push_proc("last", type_proc(0, 0, type_str), val_proc(0, 0, type_str, proc_seq_last));
    sym_push_proc("length", type_proc(0, 0, type_int), val_proc(0, 0, type_int, proc_seq_length));
    sym_push_proc("map", type_proc(0, 0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_seq_map));
    sym_push_proc("random", type_proc(0, 0, type_any), val_proc(0, 0, type_any, proc_seq_random));
    sym_push_proc("reject", type_proc(0, 0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_seq_reject));
    sym_push_proc("rejectattr", type_proc(0, 0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_seq_rejectattr));
    sym_push_proc("replace", type_proc(replace_type, re_size, type_list(type_any)), val_proc(replace_type, re_size, type_list(type_any), proc_seq_replace));
    sym_push_proc("reverse", type_proc(0, 0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_seq_reverse));
    sym_push_proc("select", type_proc(0, 0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_seq_select));
    sym_push_proc("selectattr", type_proc(0, 0, type_list(type_any)), val_proc(0, 0, type_list(type_any), proc_seq_selectattr));
    sym_push_proc("sort", type_proc(sort_type, so_size, type_list(type_any)), val_proc(sort_type, so_size, type_list(type_any), proc_seq_sort));
    sym_push_proc("unique", type_proc(unique_type, uq_size, type_list(type_any)), val_proc(unique_type, uq_size, type_list(type_any), proc_seq_unique));
    /* }}} */
    /* @INFO: numeric methoden {{{ */
    scope_set(&type_numeric_scope);
    /* }}} */
    /* @INFO: dict methoden {{{ */
    Type_Field *attr_type[]     = {  type_field("name", type_str) };
    size_t at_size = ARRAY_SIZE(attr_type);

    Type_Field *dictsort_type[] = {
        type_field("case_sensitive", type_bool, val_bool(false)),
        type_field("by",             type_str, val_str("key")),
        type_field("reverse",        type_bool, val_bool(false))
    };
    size_t ds_size = ARRAY_SIZE(dictsort_type);

    Type_Field *xm_type[]      = {
        type_field("autospace", type_bool, val_bool(true))
    };
    size_t xm_size = ARRAY_SIZE(xm_type);

    scope_set(&type_dict_scope);
    sym_push_proc("attr",     type_proc(attr_type, at_size, type_any), val_proc(attr_type, at_size, type_any, proc_dict_attr));
    sym_push_proc("dictsort", type_proc(dictsort_type, ds_size, type_any), val_proc(dictsort_type, ds_size, type_any, proc_dict_dictsort));
    sym_push_proc("items",    type_proc(0, 0, type_dict(0)), val_proc(0, 0, type_dict(0), proc_dict_items));
    sym_push_proc("xmlattr",  type_proc(xm_type, xm_size, type_str), val_proc(xm_type, xm_size, type_str, proc_dict_xmlattr));
    /* }}} */
    /* @INFO: float methoden {{{ */
    Type_Field *round_type[] = {
        type_field("precision", type_int, val_int(0)),
        type_field("method", type_str, val_str(intern_str("common")))
    };
    size_t rd_size = ARRAY_SIZE(round_type);

    scope_set(&type_float_scope);
    sym_push_proc("round", type_proc(round_type, rd_size, type_float), val_proc(round_type, rd_size, type_float, proc_float_round));
    /* }}} */
    /* @INFO: int methoden {{{ */
    Type_Field *fs_type[] = { type_field("binary", type_bool, val_bool(false)) };
    size_t fs_size = ARRAY_SIZE(fs_type);

    scope_set(&type_int_scope);
    sym_push_proc("abs", type_proc(0, 0, type_int), val_proc(0, 0, type_int, proc_int_abs));
    sym_push_proc("filesizeformat", type_proc(fs_type, fs_size, type_str), val_proc(fs_type, fs_size, type_str, proc_int_filesizeformat));
    /* }}} */
    /* @INFO: list methoden {{{ */
    Type_Field *append_type[] = { type_field("elem", type_any) };
    size_t app_size = ARRAY_SIZE(append_type);

    Type_Field *sum_type[] = {
        type_field("attribute", type_str, val_none()),
        type_field("start", type_int, val_int(0))
    };
    size_t sm_size = ARRAY_SIZE(sum_type);

    scope_set(&type_list_scope);
    sym_push_proc("append", type_proc(append_type, app_size, type_list(type_any)), val_proc(append_type, app_size, type_list(type_any), proc_list_append));
    sym_push_proc("slice", type_proc(batch_type, bt_size, type_str), val_proc(batch_type, bt_size, type_str, proc_list_slice));
    sym_push_proc("sum", type_proc(sum_type, sm_size, type_int), val_proc(sum_type, sm_size, type_int, proc_list_sum));
    /* }}} */
    /* @INFO: string methoden {{{ */
    Type_Field *format_type[]  = { type_field("fmt",     type_str) };
    Type_Field *float_type[]   = { type_field("default", type_float, val_float(0.0f)) };
    Type_Field *center_type[]  = {
        type_field("width",   type_int,   val_int(80)),
        type_field("fillchar", type_str,  val_str(" "))
    };
    Type_Field *indent_type[]  = {
        type_field("width", type_int, val_int(4)),
        type_field("first", type_bool, val_bool(false)),
        type_field("blank", type_bool, val_bool(false))
    };
    Type_Field *int_type[]     = {
        type_field("default", type_int, val_int(0)),
        type_field("base", type_int, val_int(10))
    };

    Type_Field *trunc_type[]   = {
        type_field("length", type_int, val_int(255)),
        type_field("killwords", type_bool, val_bool(false)),
        type_field("end", type_str, val_str("...")),
        type_field("leeway", type_int, val_int(0)),
    };

    Type_Field *urlz_type[]    = {
        type_field("trim_url_limit", type_bool, val_none()),
        type_field("nofollow", type_bool, val_bool(false)),
        type_field("target", type_str, val_none()),
        type_field("rel", type_str, val_none())
    };
    size_t urlz_size = ARRAY_SIZE(urlz_type);

    Type_Field *ww_type[]      = {
        type_field("width", type_int, val_int(79)),
        type_field("break_long_words", type_bool, val_bool(true)),
        type_field("wrapstring", type_str, val_none())
    };
    size_t ww_size = ARRAY_SIZE(ww_type);

    scope_set(&type_string_scope);
    sym_push_proc("capitalize",     type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_capitalize));
    sym_push_proc("center",         type_proc(center_type,  2, type_str),      val_proc(center_type,  2, type_str, proc_string_center));
    sym_push_proc("escape",         type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_escape));
    sym_push_proc("e",              type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_escape));
    sym_push_proc("float",          type_proc(float_type,   1, type_str),      val_proc(float_type,   1, type_str, proc_string_float));
    sym_push_proc("format",         type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_format));
    sym_push_proc("indent",         type_proc(indent_type,  3, type_str),      val_proc(indent_type,  3, type_str, proc_string_indent));
    sym_push_proc("int",            type_proc(int_type,     2, type_str),      val_proc(int_type,     2, type_str, proc_string_int));
    sym_push_proc("lower",          type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_lower));
    sym_push_proc("striptags",      type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_striptags));
    sym_push_proc("title",          type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_title));
    sym_push_proc("trim",           type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_trim));
    sym_push_proc("truncate",       type_proc(trunc_type,   4, type_str),      val_proc(trunc_type,   4, type_str, proc_string_truncate));
    sym_push_proc("upper",          type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_upper));
    sym_push_proc("wordcount",      type_proc(0,            0, type_int),      val_proc(0,            0, type_int, proc_string_wordcount));
    sym_push_proc("urlencode",      type_proc(0,            0, type_str),      val_proc(0,            0, type_str, proc_string_urlencode));
    sym_push_proc("urlize",         type_proc(urlz_type, urlz_size, type_str), val_proc(urlz_type, urlz_size, type_str, proc_string_urlize));
    sym_push_proc("wordwrap",       type_proc(ww_type, ww_size, type_str),     val_proc(ww_type, ww_size, type_str, proc_string_wordwrap));
    /* }}} */

    scope_set(prev_scope);
}

internal_proc void
resolve_init_arenas(size_t size) {
    arena_init(&resolve_arena, size);
}

internal_proc void
resolve_init_builtin_types() {
    type_bool  = type_new(TYPE_BOOL);
    type_bool->size = 1;
    type_bool->scope = &type_bool_scope;

    type_int   = type_new(TYPE_INT);
    type_int->size = 4;
    type_int->scope = &type_int_scope;

    type_float = type_new(TYPE_FLOAT);
    type_float->size = 4;

    type_str   = type_new(TYPE_STR);
    type_str->size = PTR_SIZE;
    type_str->flags = TYPE_FLAGS_ITERABLE;
    type_str->scope = &type_string_scope;

    type_range = type_new(TYPE_RANGE);
    type_range->size = PTR_SIZE;
    type_range->scope = &type_range_scope;

    type_any   = type_new(TYPE_ANY);
    type_any->size = PTR_SIZE;
    type_any->scope = &type_any_scope;
}

internal_proc void
resolve_init_scope() {
    filter_scope.name = "filter scope";
    system_scope.name = "system scope";
    tester_scope.name = "tester scope";
    global_scope.name = "global scope";

    global_scope.parent = &system_scope;

    type_any_scope.name    = "any scope";

    type_sequence_scope.name = "sequence scope";
    type_sequence_scope.parent = &type_any_scope;

    type_numeric_scope.name = "numeric scope";
    type_numeric_scope.parent = &type_any_scope;

    type_bool_scope.name   = "bool scope";
    type_bool_scope.parent = &type_any_scope;

    type_dict_scope.name   = "dict scope";
    type_dict_scope.parent = &type_sequence_scope;

    type_int_scope.name    = "int scope";
    type_int_scope.parent  = &type_numeric_scope;

    type_float_scope.name  = "float scope";
    type_float_scope.parent = &type_numeric_scope;

    type_list_scope.name   = "list scope";
    type_list_scope.parent = &type_sequence_scope;

    type_range_scope.name  = "range scope";
    type_range_scope.parent = &type_sequence_scope;

    type_string_scope.name = "string scope";
    type_string_scope.parent = &type_sequence_scope;
}

internal_proc void
resolve_reset() {
    sym_clear();
}

internal_proc void
resolve_init(size_t arena_size) {
    resolve_init_scope();
    resolve_init_arenas(arena_size);
    resolve_init_builtin_types();
    resolve_init_builtin_type_procs();
    resolve_init_builtin_procs();
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
    result->scope = current_scope;

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

