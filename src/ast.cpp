struct Item;

enum Expr_Kind {
    EXPR_NONE,
    EXPR_NAME,
    EXPR_INT,
    EXPR_STR,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_TERNARY,
    EXPR_FIELD,
    EXPR_RANGE,
};

struct Expr {
    Expr_Kind kind;

    union {
        struct {
            char *value;
        } expr_name;

        struct {
            int value;
        } expr_int;

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
            Expr *field;
        } expr_field;

        struct {
            Expr *left;
            Expr *right;
        } expr_range;
    };
};

internal_proc Expr *
expr_new(Expr_Kind kind) {
    Expr *result = (Expr *)xmalloc(sizeof(Expr));

    result->kind = kind;

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
expr_str(char *value) {
    Expr *result = expr_new(EXPR_STR);

    result->expr_str.value = value;

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
expr_field(Expr *expr, Expr *field) {
    Expr *result = expr_new(EXPR_FIELD);

    result->expr_field.expr = expr;
    result->expr_field.field = field;

    return result;
}

internal_proc Expr *
expr_range(Expr *left, Expr *right) {
    Expr *result = expr_new(EXPR_RANGE);

    result->expr_range.left = left;
    result->expr_range.right = right;

    return result;
}

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
};

struct Stmt {
    Stmt_Kind kind;

    union {
        struct {
            Expr *it;
            Expr *cond;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_for;

        struct {
            Expr *cond;
            Stmt **stmts;
            size_t num_stmts;
            Stmt **elseif_stmts;
            size_t num_elseifs;
            Stmt *else_stmt;
        } stmt_if;

        struct {
            char *name;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_block;

        struct {
            Item *item;
        } stmt_var;

        struct {
            Item *item;
        } stmt_lit;
    };
};

internal_proc Stmt *
stmt_new(Stmt_Kind kind) {
    Stmt *result = (Stmt *)xmalloc(sizeof(Stmt));

    result->kind = kind;

    return result;
}

internal_proc Stmt *
stmt_for(Expr *it, Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_FOR);

    result->stmt_for.it = it;
    result->stmt_for.cond = cond;
    result->stmt_for.stmts = stmts;
    result->stmt_for.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_if(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_IF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = stmts;
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_elseif(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSEIF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = stmts;
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_else(Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSE);

    result->stmt_if.cond = 0;
    result->stmt_if.stmts = stmts;
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_block(char *name, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_BLOCK);

    result->stmt_block.name = name;
    result->stmt_block.stmts = stmts;
    result->stmt_block.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_end() {
    Stmt *result = stmt_new(STMT_END);

    return result;
}

internal_proc Stmt *
stmt_var(Item *item) {
    Stmt *result = stmt_new(STMT_VAR);

    result->stmt_var.item = item;

    return result;
}

internal_proc Stmt *
stmt_lit(Item *item) {
    Stmt *result = stmt_new(STMT_LIT);

    result->stmt_lit.item = item;

    return result;
}
