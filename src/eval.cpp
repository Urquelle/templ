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
eval_var(Item *item) {
    assert(item->kind == ITEM_VAR);
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
