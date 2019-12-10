#include "templ.cpp"

PROC_CALLBACK(custom_test_xxx) {
    using namespace templ::devapi;

    if ( val_str(operand) == intern_str("xxx") ) {
        return val_bool(true);
    }

    return val_bool(false);
}

int
main(int argc, char **argv) {
    using namespace templ::api;
    using namespace templ::devapi;

    templ_init(MB(100), MB(100), MB(100));
    templ_register_test("xxx", custom_test_xxx, 0, 0);

    Templ *templ = templ_compile_string(R"END(
        {% if "xxx" is xxx %}
            -- match --
        {% else %}
            -- no match --
        {% endif %}
    )END");

    char *result = templ_render(templ, 0);

    return 0;
}

