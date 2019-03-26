#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "common.cpp"
#include "os.cpp"
#include "lex.cpp"
#include "ast.cpp"
#include "doc.cpp"
#include "parser.cpp"
#include "resolve.cpp"

int
main(int argc, char **argv) {
    char *content = 0;
    file_read("test.tpl", &content);

    Parser parser = {};
    Parser *p = &parser;
    init_parser(p, content);

    Doc doc = {};
    while (p->lex.token.kind != T_EOF) {
        if ( match_token(p, T_VAR_BEGIN) ) {
            buf_push(doc.items, parse_var(p));
        } else if ( is_token(p, T_CODE_BEGIN) ) {
            buf_push(doc.items, parse_code(p));
        } else {
            buf_push(doc.items, parse_lit(p));
        }
    }

    doc.num_items = buf_len(doc.items);
    init_resolver();
    resolve_doc(&doc);

    return 0;
}
