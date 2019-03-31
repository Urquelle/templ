internal_proc char * eval_item(Item *item);
internal_proc char * eval_var(Item *item);

internal_proc char *
eval_stmt(Stmt *stmt) {
    switch ( stmt->kind ) {
        case STMT_LIT: {
            return eval_item(stmt->stmt_lit.item);
        } break;

        case STMT_VAR: {
            return eval_var(stmt->stmt_var.item);
        } break;

        case STMT_IF: {
            return "<if>";
        } break;

        case STMT_EXTENDS: {
            return "<extends>";
        } break;

        case STMT_SET: {
            return "<set>";
        } break;

        case STMT_BLOCK: {
            return "<block>";
        } break;

        case STMT_FILTER: {
            return "<filter>";
        } break;

        case STMT_FOR: {
            return "<for>";
        } break;

        default: {
            assert(0);
            return "";
        } break;
    }
}

internal_proc char *
eval_var(Item *item) {
    assert(item->kind == ITEM_VAR);

    // Val val = extract_val(item->item_var.expr);
    for ( int i = 0; i < item->item_var.num_filter; ++i ) {
        Var_Filter filter = item->item_var.filter[i];
    }

    return "";
}

internal_proc char *
eval_item(Item *item) {
    switch ( item->kind ) {
        case ITEM_LIT: {
            return item->item_lit.lit;
        } break;

        case ITEM_VAR: {
            return eval_var(item);
        } break;

        case ITEM_CODE: {
            return eval_stmt(item->item_code.stmt);
        } break;

        default: {
            assert(0);
            return "";
        } break;
    }
}

internal_proc void
eval(Doc *doc) {
    char *result = "";
    for ( int i = 0; i < doc->num_items; ++i ) {
        result = strf("%s%s", result, eval_item(doc->items[i]));
    }
}
