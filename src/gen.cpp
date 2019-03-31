#define genf(...)   gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genlnf(...) gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__)
#define genln()     gen_result = strf("%s\n", gen_result); gen_indentation()

global_var char *gen_result = "";
global_var int gen_indent   = 0;

internal_proc void gen_item(Item *item);
internal_proc void gen_stmt(Stmt *stmt);

internal_proc void
gen_indentation() {
    gen_result = strf("%s%*.s", gen_result, 4 * gen_indent, "         ");
}

internal_proc void
gen_expr(Expr *expr) {
    //
}

internal_proc void
gen_stmts(Stmt **stmts, size_t num_stmts) {
    for ( int i = 0; i < num_stmts; ++i ) {
        gen_stmt(stmts[i]);
    }
}

internal_proc void
gen_stmt(Stmt *stmt) {
    switch (stmt->kind) {
        case STMT_BLOCK: {
            genf("<!-- %s -->", stmt->stmt_block.name);
            genln();
            gen_stmts(stmt->stmt_block.stmts, stmt->stmt_block.num_stmts);
        } break;

        case STMT_VAR: {
            gen_item(stmt->stmt_var.item);
        } break;

        case STMT_LIT: {
            gen_item(stmt->stmt_lit.item);
        } break;

        case STMT_FOR:
        case STMT_IF:
        case STMT_SET:
        case STMT_FILTER:
        case STMT_EXTENDS: {
            // nix tun
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
gen_var(Item *item) {
    assert(item->kind == ITEM_VAR);
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
gen(Doc *doc) {
    if ( doc->parent ) {
        Doc *parent = doc->parent;
        for ( int i = 0; i < parent->num_items; ++i ) {
            gen_item(parent->items[i]);
        }
        for ( int i = 0; i < doc->num_items; ++i ) {
            gen_item(doc->items[i]);
        }
    }
}
