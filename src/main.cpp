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
#include "gen_html.cpp"

int
main(int argc, char **argv) {
    Doc *doc = parse_file("test.tpl");
    init_resolver();
    resolve_doc(doc);
    gen_doc(doc);

    return 0;
}
