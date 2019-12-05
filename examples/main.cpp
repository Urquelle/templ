#include "templ.cpp"

int
main(int argc, char **argv) {
    using namespace templ::api;

    templ_init(MB(100), MB(100), MB(100));

    char *data;
    os_file_read("data.json", &data);
    Json json = json_parse(data);

    Templ *templ = templ_compile_file("main.tpl");

    Templ_Var *users = templ_var("users", json);
    Templ_Vars vars = templ_vars();

    templ_vars_add(&vars, users);

    char *result = templ_render(templ, &vars);
    os_file_write("out.html", result, utf8_str_size(result));

    return 0;
}
