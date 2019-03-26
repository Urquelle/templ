struct Var_Filter {
    char *name;
    Expr **params;
    size_t    num_params;
};

enum Item_Kind {
    ITEM_NONE,
    ITEM_CODE,
    ITEM_VAR,
    ITEM_LIT,
};

struct Item {
    Item_Kind kind;

    union {
        struct {
            Expr *expr;
            Var_Filter *filter;
            size_t num_filter;
        } item_var;

        struct {
            char *lit;
        } item_lit;

        struct {
            Stmt *stmt;
        } item_code;
    };
};

struct Doc {
    Item **items;
    size_t num_items;
};

internal_proc Var_Filter
var_filter(char *name, Expr **params, size_t num_params) {
    Var_Filter result = {};

    result.name = name;
    result.params = params;
    result.num_params = num_params;

    return result;
}

internal_proc Item *
item_new(Item_Kind kind) {
    Item *result = (Item *)xmalloc(sizeof(Item));

    result->kind = kind;

    return result;
}

internal_proc Item *
item_code(Stmt *stmt) {
    Item *result = item_new(ITEM_CODE);

    result->item_code.stmt = stmt;

    return result;
}

internal_proc Item *
item_var(Expr *expr, Var_Filter *filter, size_t num_filter) {
    Item *result = item_new(ITEM_VAR);

    result->item_var.expr = expr;
    result->item_var.filter = filter;
    result->item_var.num_filter = num_filter;

    return result;
}

internal_proc Item *
item_lit(char *lit, size_t size) {
    Item *result = item_new(ITEM_LIT);

    result->item_lit.lit = (char *)memdup(lit, size);
    result->item_lit.lit[size] = 0;

    return result;
}
