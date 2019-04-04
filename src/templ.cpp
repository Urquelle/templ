internal_proc void
init() {
    init_resolver();
}

internal_proc void
templ_main(int argc, char **argv) {
    if ( argc < 2 ) {
        printf("templ.exe <filename>\n");
    }

    init();
    Doc *doc = parse_file(argv[1]);
    resolve(doc);
    eval(doc);
    gen();
    file_write("test.html", gen_result, strlen(gen_result));
}

