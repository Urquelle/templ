#include "templ.cpp"

int
main(int argc, char **argv) {
    using namespace templ::api;

    templ_init();

    char *data;
    os_file_read("data.json", &data);
    Json json = json_parse(data);

    Templ *templ = templ_compile_file("main.tpl");

    Templ_Var *users = templ_var("users", json);
    Templ_Vars vars = templ_vars();

    templ_vars_add(&vars, users);

    char *result = "";

    int num_iterations = 2;
    for (int i = 0; i < num_iterations; ++i) {
        result = templ_render(templ, &vars);
        os_file_write("out.html", result, utf8_str_size(result));
        templ_reset();
    }

    return 0;
}
