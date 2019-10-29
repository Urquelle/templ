#include "templ.cpp"

int
main(int argc, char **argv) {
    using namespace templ::api;

    templ_init(MB(100), MB(100), MB(100));

    Templ_Vars vars = templ_vars();

    Templ_Var *address = templ_var("address");
    templ_var_set(address, "city", "berlin");
    templ_var_set(address, "street", "siegerstr. 2");

    Templ_Var *user = templ_var("user");
    templ_var_set(user, "name", "noob");
    templ_var_set(user, "age", 25);
    templ_var_set(user, "address", address);

    templ_vars_add(&vars, user);

    Parsed_Templ *templ1 = templ_compile_string("servus {{ user.name }} in {{ user.address.city }}");
    char *result1 = templ_render(templ1, &vars);

    if ( !status_is_error() ) {
        os_file_write("test1.html", result1, os_strlen(result1));
    } else {
        fprintf(stderr, "fehler aufgetreten in der übergebenen zeichenkette: %s\n", status_message());
        assert(0);
        status_reset();
    }

    templ_reset();

    Parsed_Templ *templ2 = templ_compile_file(argv[1]);
    char *result2 = templ_render(templ2, &vars);

    if ( !status_is_error() ) {
        os_file_write("test2.html", result2, os_strlen(result2));
    } else {
        fprintf(stderr, "fehler im template %s zeile %lld aufgetreten: %s\n", status_filename(), status_line(), status_message());
        assert(0);
        status_reset();
    }

    return 0;
}
