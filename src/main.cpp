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
#include "template.cpp"
#include "parser.cpp"

int
main(int argc, char **argv) {
    char *content = 0;
    file_read("test.tpl", &content);

    Parser parser = {};
    Parser *p = &parser;
    init_parser(p, content);

    Item **items = 0;
    while (p->lex.token.kind != T_EOF) {
        if ( match_token(p, T_VAR_BEGIN) ) {
            Item *item = parse_var(p);
            buf_push(items, item);
        } else if ( is_token(p, T_CODE_BEGIN) ) {
            Item *item = parse_code(p);
            buf_push(items, item);
        } else {
            Item *item = parse_lit(p);
            buf_push(items, item);
        }
    }

    return 0;
}
