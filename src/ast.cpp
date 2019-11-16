internal_proc void *
ast_dup(void *src, size_t size) {
    if (size == 0 || src == 0) {
        return NULL;
    }

    void *ptr = xmalloc(size);
    memcpy(ptr, src, size);

    return ptr;
}

struct Pair {
    char *key;
    Expr *value;
};

internal_proc Pair *
pair_new(char *key, Expr *value) {
    Pair *result = (Pair *)xcalloc(1, sizeof(Pair));

    result->key = key;
    result->value = value;

    return result;
}

struct Arg {
    Pos pos;
    char *name;
    Expr *expr;
};

internal_proc Arg *
arg_new(Pos pos, char *name, Expr *expr) {
    Arg *result = (Arg *)xcalloc(1, sizeof(Arg));

    result->pos  = pos;
    result->name = name;
    result->expr = expr;

    return result;
}

enum Expr_Kind {
    EXPR_NONE,
    EXPR_PAREN,
    EXPR_NAME,
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_BOOL,
    EXPR_STR,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_FIELD,
    EXPR_SUBSCRIPT,
    EXPR_RANGE,
    EXPR_CALL,
    EXPR_IS,
    EXPR_NOT,
    EXPR_IF,
    EXPR_TUPLE,
    EXPR_LIST,
    EXPR_DICT,
};

struct Expr {
    Expr_Kind kind;
    Pos pos;
    Expr *if_expr;
    Expr **filters;
    size_t num_filters;

    union {
        struct {
            Expr *expr;
        } expr_paren;

        struct {
            char *value;
        } expr_name;

        struct {
            int value;
        } expr_int;

        struct {
            float value;
        } expr_float;

        struct {
            b32 value;
        } expr_bool;

        struct {
            char *value;
        } expr_str;

        struct {
            Token_Kind op;
            Expr *expr;
        } expr_unary;

        struct {
            Token_Kind op;
            Expr *left;
            Expr *right;
        } expr_binary;

        struct {
            Expr *expr;
            char *field;
        } expr_field;

        struct {
            Expr *expr;
            Expr *index;
        } expr_subscript;

        struct {
            Expr *left;
            Expr *right;
        } expr_range;

        struct {
            Expr *expr;
            Arg **args;
            size_t num_args;
        } expr_call;

        struct {
            Expr *operand;
            Expr *tester;
        } expr_is;

        struct {
            Expr *expr;
        } expr_not;

        struct {
            Expr *cond;
            Expr *else_expr;
        } expr_if;

        struct {
            Expr **exprs;
            size_t num_exprs;
        } expr_tuple;

        struct {
            Expr **expr;
            size_t num_expr;
        } expr_list;

        struct {
            Pair **pairs;
            size_t num_pairs;
        } expr_dict;
    };
};

global_var Expr expr_illegal = {EXPR_NONE};

internal_proc Expr *
expr_new(Pos pos, Expr_Kind kind) {
    Expr *result = (Expr *)xcalloc(1, sizeof(Expr));

    result->pos  = pos;
    result->kind = kind;

    return result;
}

internal_proc Expr *
expr_none(Pos pos) {
    Expr *result = expr_new(pos, EXPR_NONE);

    return result;
}

internal_proc Expr *
expr_paren(Pos pos, Expr *expr) {
    Expr *result = expr_new(pos, EXPR_PAREN);

    result->expr_paren.expr = expr;

    return result;
}

internal_proc Expr *
expr_name(Pos pos, char *value) {
    Expr *result = expr_new(pos, EXPR_NAME);

    result->expr_name.value = value;

    return result;
}

internal_proc Expr *
expr_int(Pos pos, int value) {
    Expr *result = expr_new(pos, EXPR_INT);

    result->expr_int.value = value;

    return result;
}

internal_proc Expr *
expr_float(Pos pos, float value) {
    Expr *result = expr_new(pos, EXPR_FLOAT);

    result->expr_float.value = value;

    return result;
}

internal_proc Expr *
expr_str(Pos pos, char *value) {
    Expr *result = expr_new(pos, EXPR_STR);

    result->expr_str.value = value;

    return result;
}

internal_proc Expr *
expr_bool(Pos pos, b32 value) {
    Expr *result = expr_new(pos, EXPR_BOOL);

    result->expr_bool.value = value;

    return result;
}

internal_proc Expr *
expr_unary(Pos pos, Token_Kind op, Expr *expr) {
    Expr *result = expr_new(pos, EXPR_UNARY);

    result->expr_unary.op = op;
    result->expr_unary.expr = expr;

    return result;
}

internal_proc Expr *
expr_binary(Pos pos, Token_Kind op, Expr *left, Expr *right) {
    Expr *result = expr_new(pos, EXPR_BINARY);

    result->expr_binary.op = op;
    result->expr_binary.left = left;
    result->expr_binary.right = right;

    return result;
}

internal_proc Expr *
expr_field(Pos pos, Expr *expr, char *field) {
    Expr *result = expr_new(pos, EXPR_FIELD);

    result->expr_field.expr = expr;
    result->expr_field.field = field;

    return result;
}

internal_proc Expr *
expr_subscript(Pos pos, Expr *expr, Expr *index) {
    Expr *result = expr_new(pos, EXPR_SUBSCRIPT);

    result->expr_subscript.expr  = expr;
    result->expr_subscript.index = index;

    return result;
}

internal_proc Expr *
expr_range(Pos pos, Expr *left, Expr *right) {
    Expr *result = expr_new(pos, EXPR_RANGE);

    result->expr_range.left = left;
    result->expr_range.right = right;

    return result;
}

internal_proc Expr *
expr_call(Pos pos, Expr *expr, Arg **args, size_t num_args) {
    Expr *result = expr_new(pos, EXPR_CALL);

    result->expr_call.expr = expr;
    result->expr_call.args = (Arg **)AST_DUP(args);
    result->expr_call.num_args = num_args;

    return result;
}

internal_proc Expr *
expr_is(Pos pos, Expr *operand, Expr *tester) {
    Expr *result = expr_new(pos, EXPR_IS);

    result->expr_is.operand = operand;
    result->expr_is.tester = tester;

    return result;
}

internal_proc Expr *
expr_not(Pos pos, Expr *expr) {
    Expr *result = expr_new(pos, EXPR_NOT);

    result->expr_not.expr = expr;

    return result;
}

internal_proc Expr *
expr_if(Pos pos, Expr *cond, Expr *else_expr) {
    Expr *result = expr_new(pos, EXPR_IF);

    result->expr_if.cond = cond;
    result->expr_if.else_expr = else_expr;

    return result;
}

internal_proc Expr *
expr_tuple(Pos pos, Expr **exprs, size_t num_exprs) {
    Expr *result = expr_new(pos, EXPR_TUPLE);

    result->expr_tuple.exprs = (Expr **)AST_DUP(exprs);
    result->expr_tuple.num_exprs = num_exprs;

    return result;
}

internal_proc Expr *
expr_list(Pos pos, Expr **expr, size_t num_expr) {
    Expr *result = expr_new(pos, EXPR_LIST);

    result->expr_list.expr = (Expr **)AST_DUP(expr);
    result->expr_list.num_expr = num_expr;

    return result;
}

internal_proc Expr *
expr_dict(Pos pos, Pair **pairs, size_t num_pairs) {
    Expr *result = expr_new(pos, EXPR_DICT);

    result->expr_dict.pairs = (Pair **)AST_DUP(pairs);
    result->expr_dict.num_pairs = num_pairs;

    return result;
}

struct Param {
    char *name;
    Expr *default_value;
};

internal_proc Param *
param_new(char *name, Expr *default_value) {
    Param *result = (Param *)xcalloc(1, sizeof(Param));

    result->name = name;
    result->default_value = default_value;

    return result;
}

struct Imported_Sym {
    char *name;
    char *alias;
};

internal_proc Imported_Sym *
imported_sym(char *name, char *alias) {
    Imported_Sym *result = (Imported_Sym *)xcalloc(1, sizeof(Imported_Sym));

    result->name = name;
    result->alias = alias;

    return result;
}

enum Stmt_Kind {
    STMT_NONE,
    STMT_FOR,
    STMT_IF,
    STMT_BLOCK,
    STMT_ELSE,
    STMT_ENDFOR,
    STMT_ENDIF,
    STMT_ENDBLOCK,
    STMT_ENDFILTER,
    STMT_ENDMACRO,
    STMT_ENDWITH,
    STMT_VAR,
    STMT_LIT,
    STMT_EXTENDS,
    STMT_SET,
    STMT_FILTER,
    STMT_INCLUDE,
    STMT_MACRO,
    STMT_IMPORT,
    STMT_FROM_IMPORT,
    STMT_RAW,
    STMT_WITH,
    STMT_BREAK,
    STMT_CONTINUE,
};

struct Stmt {
    Stmt_Kind kind;
    Pos pos;

    union {
        struct {
            Expr **vars;
            size_t num_vars;
            Expr *set;
            b32 recursive;

            Stmt **stmts;
            size_t num_stmts;

            Stmt **else_stmts;
            size_t num_else_stmts;
        } stmt_for;

        struct {
            Expr *cond;
            Stmt *else_stmt;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_if;

        struct {
            char *name;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_block;

        struct {
            Expr *expr;
        } stmt_var;

        struct {
            char *value;
        } stmt_lit;

        struct {
            Expr *name;
            Parsed_Templ *templ;
            Parsed_Templ *else_templ;
        } stmt_extends;

        struct {
            Expr **names;
            size_t num_names;
            Expr *expr;
        } stmt_set;

        struct {
            Expr **filter;
            size_t num_filter;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_filter;

        struct {
            Parsed_Templ **templ;
            size_t num_templ;
            b32 ignore_missing;
            b32 with_context;
        } stmt_include;

        struct {
            char *name;
            char *alias;
            Param **params;
            size_t num_params;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_macro;

        struct {
            Parsed_Templ *templ;
            char *name;
        } stmt_import;

        struct {
            Parsed_Templ *templ;
            Imported_Sym **syms;
            size_t num_syms;
        } stmt_from_import;

        struct {
            char *value;
        } stmt_raw;

        struct {
            Arg **args;
            size_t num_args;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_with;
    };
};

global_var Stmt stmt_illegal   = { STMT_NONE };
global_var Stmt stmt_endblock  = { STMT_ENDBLOCK };
global_var Stmt stmt_endfilter = { STMT_ENDFILTER };
global_var Stmt stmt_endfor    = { STMT_ENDFOR };
global_var Stmt stmt_endif     = { STMT_ENDIF };
global_var Stmt stmt_endmacro  = { STMT_ENDMACRO };
global_var Stmt stmt_endwith   = { STMT_ENDWITH };
global_var Stmt stmt_break     = { STMT_BREAK };
global_var Stmt stmt_continue  = { STMT_CONTINUE };

internal_proc Stmt *
stmt_new(Stmt_Kind kind) {
    Stmt *result = (Stmt *)xmalloc(sizeof(Stmt));

    result->kind = kind;

    return result;
}

internal_proc Stmt *
stmt_for(Expr **vars, size_t num_vars, Expr *set, Stmt **stmts,
        size_t num_stmts, Stmt **else_stmts, size_t num_else_stmts,
        b32 recursive)
{
    Stmt *result = stmt_new(STMT_FOR);

    result->stmt_for.vars      = (Expr **)AST_DUP(vars);
    result->stmt_for.num_vars  = num_vars;
    result->stmt_for.set       = set;

    result->stmt_for.stmts     = (Stmt **)AST_DUP(stmts);
    result->stmt_for.num_stmts = num_stmts;

    result->stmt_for.else_stmts = (Stmt **)AST_DUP(else_stmts);
    result->stmt_for.num_else_stmts = num_else_stmts;

    return result;
}

internal_proc Stmt *
stmt_if(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_IF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;
    result->stmt_if.else_stmt = 0;

    return result;
}

internal_proc Stmt *
stmt_else(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSE);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;
    result->stmt_if.else_stmt = 0;

    return result;
}

internal_proc Stmt *
stmt_block(char *name, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_BLOCK);

    result->stmt_block.name = name;
    result->stmt_block.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_block.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_var(Expr *expr) {
    Stmt *result = stmt_new(STMT_VAR);

    result->stmt_var.expr = expr;

    return result;
}

internal_proc Stmt *
stmt_lit(char *value, size_t len) {
    Stmt *result = stmt_new(STMT_LIT);

    result->stmt_lit.value = value;
    result->stmt_lit.value[len] = 0;

    return result;
}

internal_proc Stmt *
stmt_extends(Expr *name, Parsed_Templ *templ, Parsed_Templ *else_templ) {
    Stmt *result = stmt_new(STMT_EXTENDS);

    result->stmt_extends.name       = name;
    result->stmt_extends.templ      = templ;
    result->stmt_extends.else_templ = else_templ;

    return result;
}

internal_proc Stmt *
stmt_set(Expr **names, size_t num_names, Expr *expr) {
    Stmt *result = stmt_new(STMT_SET);

    result->stmt_set.names = (Expr **)AST_DUP(names);
    result->stmt_set.num_names = num_names;
    result->stmt_set.expr = expr;

    return result;
}

internal_proc Stmt *
stmt_filter(Expr **filter, size_t num_filter, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_FILTER);

    result->stmt_filter.filter = (Expr **)AST_DUP(filter);
    result->stmt_filter.num_filter = num_filter;
    result->stmt_filter.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_filter.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_include(Parsed_Templ **templ, size_t num_templ, b32 ignore_missing,
        b32 with_context)
{
    Stmt *result = stmt_new(STMT_INCLUDE);

    result->stmt_include.templ     = (Parsed_Templ **)AST_DUP(templ);
    result->stmt_include.num_templ = num_templ;
    result->stmt_include.ignore_missing = ignore_missing;
    result->stmt_include.with_context = with_context;

    return result;
}

internal_proc Stmt *
stmt_macro(char *name, Param **params, size_t num_params, Stmt **stmts,
        size_t num_stmts)
{
    Stmt *result = stmt_new(STMT_MACRO);

    result->stmt_macro.name = name;
    result->stmt_macro.alias = 0;
    result->stmt_macro.params = params;
    result->stmt_macro.num_params = num_params;
    result->stmt_macro.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_macro.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_import(Parsed_Templ *templ, char *name) {
    Stmt *result = stmt_new(STMT_IMPORT);

    result->stmt_import.templ = templ;
    result->stmt_import.name = name;

    return result;
}

internal_proc Stmt *
stmt_from_import(Parsed_Templ *templ, Imported_Sym **syms, size_t num_syms) {
    Stmt *result = stmt_new(STMT_FROM_IMPORT);

    result->stmt_from_import.templ = templ;
    result->stmt_from_import.syms = (Imported_Sym **)AST_DUP(syms);
    result->stmt_from_import.num_syms = num_syms;

    return result;
}

internal_proc Stmt *
stmt_raw(char *value) {
    Stmt *result = stmt_new(STMT_RAW);

    result->stmt_raw.value = value;

    return result;
}

internal_proc Stmt *
stmt_with(Arg **args, size_t num_args, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_WITH);

    result->stmt_with.args = (Arg **)AST_DUP(args);
    result->stmt_with.num_args = num_args;
    result->stmt_with.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_with.num_stmts = num_stmts;

    return result;
}

struct Parsed_Templ {
    char *name;
    Parsed_Templ *parent;
    Stmt **stmts;
    size_t num_stmts;
};

internal_proc Parsed_Templ *
parsed_templ(char *name) {
    Parsed_Templ *result = (Parsed_Templ *)xcalloc(1, sizeof(Parsed_Templ));

    result->name = name;

    return result;
}

