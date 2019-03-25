struct Type;

internal_proc void resolve_item(Item *item);
internal_proc void resolve_expr(Expr *expr);

struct Type_Field {
    char *name;
    Type *type;
};

enum Type_Kind {
    TYPE_NONE,
};

struct Type {
    Type_Kind kind;
    Type_Field **fields;
    size_t num_fields;
};

enum Sym_Kind {
    SYM_NONE,
};

struct Sym {
    Sym_Kind kind;
    char *name;
    Type *type;
};

global_var Sym **doc_syms;

internal_proc Sym *
get_sym(char *name) {
    for ( int i = 0; i < buf_len(doc_syms); ++i ) {
        Sym *sym = doc_syms[i];
        if ( sym->name == name ) {
            return sym;
        }
    }

    return 0;
}

internal_proc Sym *
resolve_name(char *name) {
    Sym *result = get_sym(name);

    return result;
}

internal_proc void
resolve_stmt(Stmt *stmt) {
    switch (stmt->kind) {
        case STMT_VAR: {
            resolve_item(stmt->stmt_var.item);
        } break;

        case STMT_FOR: {
            resolve_expr(stmt->stmt_for.it);
            resolve_expr(stmt->stmt_for.cond);
        } break;

        case STMT_IF: {
        } break;

        case STMT_BLOCK: {
        } break;

        case STMT_END: {
        } break;

        case STMT_LIT: {
            resolve_item(stmt->stmt_lit.item);
        } break;

        case STMT_EXTENDS: {
        } break;

        case STMT_FILTER: {
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
resolve_expr(Expr *expr) {
    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = resolve_name(expr->expr_name.value);
        } break;

        case EXPR_STR: {
        } break;

        case EXPR_INT: {
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
resolve_filter(Var_Filter filter) {
    resolve_name(filter.name);
    for ( int i = 0; i < filter.num_params; ++i ) {
        resolve_expr(filter.params[i]);
    }
}

internal_proc void
resolve_var(Item *item) {
    assert(item->kind == ITEM_VAR);

    resolve_expr(item->item_var.expr);
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
resolve(Doc *d) {
    for ( int i = 0; i < d->num_items; ++i ) {
        resolve_item(d->items[i]);
    }
}
