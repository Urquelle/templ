internal_proc Sym *
extract_sym(Expr *expr) {
    switch (expr->kind) {
        case EXPR_NAME: {
            return expr->expr_name.sym;
        } break;

        case EXPR_FIELD: {
            return extract_sym(expr->expr_field.field);
        } break;

        case EXPR_CALL: {
            return extract_sym(expr->expr_call.expr);
        } break;

        default: {
            assert(0);
            return 0;
        } break;
    }
}

internal_proc void
eval_stmt(Stmt *stmt) {
    switch ( stmt->kind ) {
        case STMT_LIT: {
            //
        } break;

        case STMT_IF: {
            //
        } break;

        case STMT_EXTENDS: {
            //
        } break;

        case STMT_SET: {
            //
        } break;

        case STMT_BLOCK: {
            //
        } break;

        case STMT_FILTER: {
            //
        } break;

        case STMT_FOR: {
            //
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
exec_filter(Sym *sym, Var_Filter *filter) {
    Sym *sym_filter = filter->sym;
    assert(sym_filter->type->kind == TYPE_FILTER);
    sym_filter->type->type_proc.callback(&sym->val, filter->params, filter->num_params);
}

internal_proc void
eval_var(Item *item) {
    assert(item->kind == ITEM_VAR);
#if 0
    char *mem = resolve_mem(item->item_var.expr);

    for ( int i = 0; i < item->item_var.num_filter; ++i ) {
        Var_Filter filter = item->item_var.filter[i];
        exec_filter(sym, &filter);
    }
#endif
}

internal_proc void
eval_item(Item *item) {
    switch ( item->kind ) {
        case ITEM_LIT: {
            //
        } break;

        case ITEM_VAR: {
            eval_var(item);
        } break;

        case ITEM_CODE: {
            eval_stmt(item->item_code.stmt);
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
eval(Doc *doc) {
    for ( int i = 0; i < doc->num_items; ++i ) {
        eval_item(doc->items[i]);
    }
}
