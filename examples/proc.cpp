#include "templ.cpp"

PROC_CALLBACK(custom_hello) {
    using namespace templ::devapi;

    return val_str("hello, world");
}

int
main(int argc, char **argv) {
    using namespace templ::api;
    using namespace templ::devapi;

    templ_init(MB(100), MB(100), MB(100));
    templ_register_proc("hello", custom_hello, 0, 0, type_str);

    Templ *templ = templ_compile_string(R"END(
        {{ hello() }}
    )END");

    char *result = templ_render(templ, 0);

    return 0;
}

