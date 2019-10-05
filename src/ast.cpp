internal_proc void *
ast_dup(void *src, size_t size) {
    if (size == 0) {
        return NULL;
    }

    void *ptr = xmalloc(size);
    memcpy(ptr, src, size);

    return ptr;
}

#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))

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
    EXPR_TERNARY,
    EXPR_FIELD,
    EXPR_INDEX,
    EXPR_RANGE,
    EXPR_CALL,
    EXPR_IS,
};

struct Expr {
    Pos pos;
    Expr_Kind kind;

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
            bool value;
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
            Expr *left;
            Expr *middle;
            Expr *right;
        } expr_ternary;

        struct {
            Expr *expr;
            char *field;
        } expr_field;

        struct {
            Expr *expr;
            Expr *index;
        } expr_index;

        struct {
            Expr *left;
            Expr *right;
        } expr_range;

        struct {
            Expr *expr;
            Expr **params;
            size_t num_params;
        } expr_call;

        struct {
            Expr *var;
            Expr *test;
            Expr **args;
            size_t num_args;
        } expr_is;
    };
};

internal_proc Expr *
expr_new(Expr_Kind kind) {
    Expr *result = (Expr *)xcalloc(1, sizeof(Expr));

    result->kind = kind;

    return result;
}

internal_proc Expr *
expr_paren(Expr *expr) {
    Expr *result = expr_new(EXPR_PAREN);

    result->expr_paren.expr = expr;

    return result;
}

internal_proc Expr *
expr_name(char *value) {
    Expr *result = expr_new(EXPR_NAME);

    result->expr_name.value = value;

    return result;
}

internal_proc Expr *
expr_int(int value) {
    Expr *result = expr_new(EXPR_INT);

    result->expr_int.value = value;

    return result;
}

internal_proc Expr *
expr_float(float value) {
    Expr *result = expr_new(EXPR_FLOAT);

    result->expr_float.value = value;

    return result;
}

internal_proc Expr *
expr_str(char *value) {
    Expr *result = expr_new(EXPR_STR);

    result->expr_str.value = value;

    return result;
}

internal_proc Expr *
expr_bool(bool value) {
    Expr *result = expr_new(EXPR_BOOL);

    result->expr_bool.value = value;

    return result;
}

internal_proc Expr *
expr_unary(Token_Kind op, Expr *expr) {
    Expr *result = expr_new(EXPR_UNARY);

    result->expr_unary.op = op;
    result->expr_unary.expr = expr;

    return result;
}

internal_proc Expr *
expr_binary(Token_Kind op, Expr *left, Expr *right) {
    Expr *result = expr_new(EXPR_BINARY);

    result->expr_binary.op = op;
    result->expr_binary.left = left;
    result->expr_binary.right = right;

    return result;
}

internal_proc Expr *
expr_ternary(Expr *left, Expr *middle, Expr *right) {
    Expr *result = expr_new(EXPR_TERNARY);

    result->expr_ternary.left = left;
    result->expr_ternary.middle = middle;
    result->expr_ternary.right = right;

    return result;
}

internal_proc Expr *
expr_field(Expr *expr, char *field) {
    Expr *result = expr_new(EXPR_FIELD);

    result->expr_field.expr = expr;
    result->expr_field.field = field;

    return result;
}

internal_proc Expr *
expr_index(Expr *expr, Expr *index) {
    Expr *result = expr_new(EXPR_INDEX);

    result->expr_index.expr = expr;
    result->expr_index.index = index;

    return result;
}

internal_proc Expr *
expr_range(Expr *left, Expr *right) {
    Expr *result = expr_new(EXPR_RANGE);

    result->expr_range.left = left;
    result->expr_range.right = right;

    return result;
}

internal_proc Expr *
expr_call(Expr *expr, Expr **params, size_t num_params) {
    Expr *result = expr_new(EXPR_CALL);

    result->expr_call.expr = expr;
    result->expr_call.params = (Expr **)AST_DUP(params);
    result->expr_call.num_params = num_params;

    return result;
}

internal_proc Expr *
expr_is(Expr *var, Expr *test, Expr **args, size_t num_args) {
    Expr *result = expr_new(EXPR_IS);

    result->expr_is.var = var;
    result->expr_is.test = test;
    result->expr_is.args = args;
    result->expr_is.num_args = num_args;

    return result;
}

struct Var_Filter {
    char*  name;
    Expr** params;
    size_t num_params;
};

enum Stmt_Kind {
    STMT_NONE,
    STMT_FOR,
    STMT_IF,
    STMT_BLOCK,
    STMT_ELSEIF,
    STMT_ELSE,
    STMT_END,
    STMT_VAR,
    STMT_LIT,
    STMT_EXTENDS,
    STMT_SET,
    STMT_FILTER,
};

struct Stmt {
    Stmt_Kind kind;

    union {
        struct {
            char *it;
            Expr *cond;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_for;

        struct {
            Expr *cond;
            Expr *test;
            Stmt **stmts;
            size_t num_stmts;
            Stmt **elseifs;
            size_t num_elseifs;
            Stmt *else_stmt;
        } stmt_if;

        struct {
            char *name;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_block;

        struct {
            Expr *expr;
            Var_Filter *filter;
            size_t num_filter;
        } stmt_var;

        struct {
            char *value;
        } stmt_lit;

        struct {
            char *name;
        } stmt_extends;

        struct {
            char *name;
            Expr *expr;
        } stmt_set;

        struct {
            Var_Filter *filter;
            size_t num_filter;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_filter;
    };
};

internal_proc Stmt *
stmt_new(Stmt_Kind kind) {
    Stmt *result = (Stmt *)xmalloc(sizeof(Stmt));

    result->kind = kind;

    return result;
}

internal_proc Stmt *
stmt_for(char *it, Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_FOR);

    result->stmt_for.it = it;
    result->stmt_for.cond = cond;
    result->stmt_for.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_for.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_if(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_IF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_elseif(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSEIF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_else(Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSE);

    result->stmt_if.cond = 0;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

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
stmt_end() {
    Stmt *result = stmt_new(STMT_END);

    return result;
}

internal_proc Stmt *
stmt_var(Expr *expr, Var_Filter *filter, size_t num_filter) {
    Stmt *result = stmt_new(STMT_VAR);

    result->stmt_var.expr = expr;
    result->stmt_var.filter = filter;
    result->stmt_var.num_filter = num_filter;

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
stmt_extends(char *name) {
    Stmt *result = stmt_new(STMT_EXTENDS);

    result->stmt_extends.name = name;

    return result;
}

internal_proc Stmt *
stmt_set(char *name, Expr *expr) {
    Stmt *result = stmt_new(STMT_SET);

    result->stmt_set.name = name;
    result->stmt_set.expr = expr;

    return result;
}

internal_proc Stmt *
stmt_filter(Var_Filter *filter, size_t num_filter, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_FILTER);

    result->stmt_filter.filter = (Var_Filter *)AST_DUP(filter);
    result->stmt_filter.num_filter = num_filter;
    result->stmt_filter.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_filter.num_stmts = num_stmts;

    return result;
}

struct Parsed_Doc {
    Parsed_Doc *parent;
    Stmt **stmts;
    size_t num_stmts;
};

internal_proc Var_Filter
var_filter(char *name, Expr **params, size_t num_params) {
    Var_Filter result = {};

    result.name = name;
    result.params = params;
    result.num_params = num_params;

    return result;
}

