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

enum { MAX_LOCAL_SYMS = 1024 };
global_var Sym global_scope_pointer[MAX_LOCAL_SYMS];
global_var Sym *current_scope_marker = global_scope_pointer;

internal_proc Sym *
scope_enter() {
    return global_scope_pointer;
}

internal_proc void
scope_leave(Sym *scope_marker) {
    current_scope_marker = scope_marker;
}

global_var Sym **doc_syms;

internal_proc Sym *
sym_get(char *name) {
    for ( int i = 0; i < buf_len(doc_syms); ++i ) {
        Sym *sym = doc_syms[i];
        if ( sym->name == name ) {
            return sym;
        }
    }

    return 0;
}

internal_proc void
sym_scoped_push(char *name, Type *type) {
    //
}

internal_proc Sym *
resolve_name(char *name) {
    Sym *result = sym_get(name);

    return result;
}

internal_proc void
resolve_stmt(Stmt *stmt) {
    switch (stmt->kind) {
        case STMT_VAR: {
            resolve_item(stmt->stmt_var.item);
        } break;

        case STMT_FOR: {
            Sym *scope_marker = scope_enter();

            // sym_scoped_push(stmt->stmt_for.it)
            resolve_expr(stmt->stmt_for.cond);
            for ( int i = 0; i < stmt->stmt_for.num_stmts; ++i ) {
                resolve_stmt(stmt->stmt_for.stmts[i]);
            }

            scope_leave(scope_marker);
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
            if ( !sym ) {
                assert(!"konnte symbol nicht auflösen");
            }
        } break;

        case EXPR_STR: {
            // ...
        } break;

        case EXPR_INT: {
            // ...
        } break;

        case EXPR_UNARY: {
            resolve_expr(expr->expr_unary.expr);
        } break;

        case EXPR_BINARY: {
            resolve_expr(expr->expr_binary.left);
            resolve_expr(expr->expr_binary.right);
        } break;

        case EXPR_TERNARY: {
            resolve_expr(expr->expr_ternary.left);
            resolve_expr(expr->expr_ternary.middle);
            resolve_expr(expr->expr_ternary.right);
        } break;

        case EXPR_FIELD: {
            resolve_expr(expr->expr_field.expr);
        } break;

        case EXPR_RANGE: {
            resolve_expr(expr->expr_range.left);
            resolve_expr(expr->expr_range.right);
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
resolve_filter(Var_Filter filter) {
    Sym *sym = resolve_name(filter.name);

    if ( !sym ) {
        assert(!"symbol konnte nicht gefunden werden!");
    }

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
resolve_doc(Doc *d) {
    for ( int i = 0; i < d->num_items; ++i ) {
        resolve_item(d->items[i]);
    }
}
