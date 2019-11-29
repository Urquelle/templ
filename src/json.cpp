struct Json_Node;

global_var char *json_keyword_true = intern_str("true");
global_var char *json_keyword_false = intern_str("false");
global_var char *json_keyword_null = intern_str("null");

struct Json_Pair {
    char *name;
    Json_Node *value;
};

internal_proc Json_Pair *
json_pair(char *name, Json_Node *value) {
    Json_Pair *result = (Json_Pair *)xmalloc(sizeof(Json_Pair));

    result->name = name;
    result->value = value;

    return result;
}

struct Json {
    Json_Node **nodes;
    size_t num_nodes;
};

enum Json_Node_Kind {
    JSON_STR,
    JSON_INT,
    JSON_FLOAT,
    JSON_BOOL,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_NULL,
};
struct Json_Node {
    Json_Node_Kind kind;

    union {
        struct {
            char *value;
        } json_str;

        struct {
            s32 value;
            s32 exp;
        } json_int;

        struct {
            f32 value;
            s32 exp;
        } json_float;

        struct {
            b32 value;
        } json_bool;

        struct {
            Json_Node **vals;
            size_t num_vals;
        } json_array;

        struct {
            Json_Pair **pairs;
            size_t num_pairs;
        } json_object;
    };
};

internal_proc Json_Node *
json_new(Json_Node_Kind kind) {
    Json_Node *result = (Json_Node *)xmalloc(sizeof(Json_Node));

    result->kind = kind;

    return result;
}

internal_proc Json_Node *
json_int(s32 val, s32 exp) {
    Json_Node *result = json_new(JSON_INT);

    result->json_int.value = val;
    result->json_int.exp = exp;

    return result;
}

internal_proc Json_Node *
json_float(f32 val, s32 exp) {
    Json_Node *result = json_new(JSON_FLOAT);

    result->json_float.value = val;
    result->json_float.exp = exp;

    return result;
}

internal_proc Json_Node *
json_str(char *str, char *end) {
    Json_Node *result = json_new(JSON_STR);

    result->json_str.value = intern_str(str, end);

    return result;
}

internal_proc Json_Node *
json_bool(b32 value) {
    Json_Node *result = json_new(JSON_BOOL);

    result->json_bool.value = value;

    return result;
}

internal_proc Json_Node *
json_array(Json_Node **vals, size_t num_vals) {
    Json_Node *result = json_new(JSON_ARRAY);

    result->json_array.vals = vals;
    result->json_array.num_vals = num_vals;

    return result;
}

internal_proc Json_Node *
json_object(Json_Pair **pairs, size_t num_pairs) {
    Json_Node *result = json_new(JSON_OBJECT);

    result->json_object.pairs = pairs;
    result->json_object.num_pairs = num_pairs;

    return result;
}

internal_proc Json_Node *
json_null() {
    Json_Node *result = json_new(JSON_NULL);

    return result;
}

internal_proc Json_Node *
json_parse_node(char **str) {
#define SKIP_WHITESPACE() do { while ( **str == ' ' || **str == '\t' ) { (*str)++; } } while(false)

    Json_Node *result = 0;
    SKIP_WHITESPACE();

    erstes_if ( **str == '{' ) {
        (*str)++;

        Json_Pair **pairs = 0;
        if ( **str != '}' ) {
            do {
                Json_Node *name = json_parse_node(str);
                assert(name->kind == JSON_STR);
                SKIP_WHITESPACE();

                if ( **str != ':' ) {
                    fatal(0, 0, "':' expected, got '%1s'", **str);
                } else {
                    (*str)++;
                }

                Json_Node *value = json_parse_node(str);

                buf_push(pairs, json_pair(name->json_str.value, value));

                SKIP_WHITESPACE();
            } while ( (**str == ',') ? (*str)++, true : false );
        }

        if ( **str != '}' ) {
            fatal(0, 0, "expected '}', got %1s", **str);
        } else {
            (*str)++;
        }

        result = json_object(pairs, buf_len(pairs));
    } else if ( **str == '[' ) {
        (*str)++;
        Json_Node **nodes = 0;

        if ( **str != ']' ) {
            do {
                Json_Node *node = json_parse_node(str);
                SKIP_WHITESPACE();
            } while ( (**str == ',') ? (*str)++, true : false );
        }

        if ( **str != ']' ) {
            fatal(0, 0, "expected ']', got %1s", **str);
        } else {
            (*str)++;
        }

        result = json_array(nodes, buf_len(nodes));
    } else if ( **str == '"' ) {
        (*str)++;
        char *start = *str;

        while ( **str != '"' ) {
            if ( **str == '\\' ) {
                (*str)++;
            }
            (*str)++;
        }

        result = json_str(start, *str);
        (*str)++;
    } else if ( **str == '-' || **str >= '0' && **str <= '9' ) {
        b32 is_float = false;
        s32 mult = 1;

        if ( **str == '-' ) {
            mult = -1;
            (*str)++;
        }

        char *base = *str;

        s32 int_value = 0;
        while ( **str >= '0' && **str <= '9' ) {
            int_value *= 10;
            int_value += **str - '0';
            (*str)++;
        }

        f32 float_value = 0;
        if ( **str == '.' ) {
            (*str)++;
            is_float = true;

            while ( **str >= '0' && **str <= '9' ) {
                (*str)++;
            }

            float_value = strtof(base, NULL);
        }

        s32 exp = 1;
        if ( **str == 'e' || **str == 'E' ) {
            exp = 0;

            (*str)++;
            if ( **str == '+' ) {
                (*str)++;
            } else if ( **str == '-' ) {
                (*str)++;
            }

            while ( **str >= '0' && **str <= '9' ) {
                exp *= 10;
                exp += **str - '0';

                (*str)++;
            }
        }

        if ( is_float ) {
            result = json_float(float_value*mult, exp);
        } else {
            result = json_int(int_value*mult, exp);
        }
    } else {
        char *start = *str;
        while ( **str >= 'a' && **str <= 'z' ) {
            (*str)++;
        }

        char *name = intern_str(start, (*str)-1);
        erstes_if ( name == json_keyword_true ) {
            result = json_bool(true);
        } else if ( name == json_keyword_false ) {
            result = json_bool(false);
        } else if ( name == json_keyword_null ) {
            result = json_null();
        } else {
            fatal(0, 0, "unrecognized value %s", name);
        }
    }

    return result;

#undef SKIP_WHITESPACE
}

internal_proc Json
json_parse(char *str) {
    char *ptr = str;
    Json_Node **nodes = 0;

    while ( *ptr ) {
        Json_Node *node = json_parse_node(&ptr);
        buf_push(nodes, node);
    }

    Json result = { nodes, buf_len(nodes) };

    return result;
}
