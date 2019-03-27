#define genf(...)   gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genlnf(...) gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__); ++gen_row
#define genln()     gen_result = strf("%s\n", gen_result); gen_indentation(); ++gen_row

global_var char *gen_result = "";
global_var int gen_indent   = 0;

internal_proc void
gen_indentation() {
    gen_result = strf("%s%*.s", gen_result, 4 * gen_indent, "         ");
}

internal_proc void
gen_expr(Expr *expr) {
    //
}

internal_proc void
gen_stmt(Stmt *stmt) {
    //
}

internal_proc void
gen_var(Item *item) {
    assert(item->kind == ITEM_VAR);
    for ( int i = 0; i < item->item_var.num_filter; ++i ) {
        Var_Filter filter = item->item_var.filter[i];
    }
}

internal_proc void
gen_item(Item *item) {
    switch ( item->kind ) {
        case ITEM_LIT: {
            genf("%s", item->item_lit.lit);
        } break;

        case ITEM_VAR: {
            gen_var(item);
        } break;

        case ITEM_CODE: {
            gen_stmt(item->item_code.stmt);
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
gen_doc(Doc *doc) {
    if ( doc->parent ) {
        Doc *parent = doc->parent;
        for ( int i = 0; i < parent->num_items; ++i ) {
            gen_item(parent->items[i]);
        }
    }
}
