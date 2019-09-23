internal_proc char *
eval_var(Item *item) {
    assert(item->kind == ITEM_VAR);

    for ( int i = 0; i < item->item_var.num_filter; ++i ) {
        Var_Filter filter = item->item_var.filter[i];
    }

    return "";
}

internal_proc Operand *
eval_expr(Expr *expr) {
    switch (expr->kind) {
        case EXPR_NAME: {
//            return fetch_resolved_expr(expr);
        } break;

        case EXPR_INT: {
//            return fetch_resolved_expr(expr);
        } break;

        case EXPR_FLOAT: {
//            return fetch_resolved_expr(expr);
        } break;

        default: {
            //
        } break;
    }

    return 0;
}

internal_proc Instr *
eval_item(Resolved_Item *item) {
    switch ( item->kind ) {
        case RESOLVED_LIT: {
            return instr_print(item->lit.str);
        } break;

        case RESOLVED_VAR: {
            return instr_print(to_char(item->op->val));
        } break;

        case RESOLVED_SET: {
            return instr_set();
        } break;

        default: {
            assert(0);
            return 0;
        } break;
    }
}

internal_proc void
eval() {
    for ( int i = 0; i < buf_len(resolved_items); ++i ) {
        push_instr(eval_item(resolved_items[i]));
    }
}
