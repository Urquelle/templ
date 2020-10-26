struct Json_Node;

global_var char *json_keyword_true = intern_str("true");
global_var char *json_keyword_false = intern_str("false");
global_var char *json_keyword_null = intern_str("null");

#define JSON_ALLOCATOR(name) void * name(size_t size)
typedef JSON_ALLOCATOR(Json_Alloc);
#define JSON_ALLOC_STRUCT(name) (name *)json_alloc(sizeof(name))

JSON_ALLOCATOR(json_alloc_default) {
    void *result = malloc(size);

    return result;
}

Json_Alloc *json_alloc = json_alloc_default;

struct Json_Pair {
    char *name;
    Json_Node *value;
};

internal_proc Json_Pair *
json_pair(char *name, Json_Node *value) {
    Json_Pair *result = JSON_ALLOC_STRUCT(Json_Pair);

    result->name = intern_str(name);
    result->value = value;

    return result;
}

struct Json {
    Json_Node *node;
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
            Json_Node **nodes;
            size_t num_nodes;
        } json_array;

        struct {
            Json_Pair **pairs;
            size_t num_pairs;
            Map syms;
        } json_object;
    };
};

internal_proc Json_Node *
json_new(Json_Node_Kind kind) {
    Json_Node *result = JSON_ALLOC_STRUCT(Json_Node);

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
json_array(Json_Node **nodes, size_t num_nodes) {
    Json_Node *result = json_new(JSON_ARRAY);

    result->json_array.nodes = nodes;
    result->json_array.num_nodes = num_nodes;

    return result;
}

internal_proc Json_Node *
json_object(Json_Pair **pairs, size_t num_pairs) {
    Json_Node *result = json_new(JSON_OBJECT);

    result->json_object.syms = {};
    result->json_object.pairs = pairs;
    result->json_object.num_pairs = num_pairs;

    for ( int i = 0; i < num_pairs; ++i ) {
        Json_Pair *pair = pairs[i];
        map_put(&result->json_object.syms, pair->name, pair->value);
    }

    return result;
}

internal_proc void *
json_sym(Json_Node *node, char *name) {
    ASSERT(node->kind == JSON_OBJECT);

    void *result = map_get(&node->json_object.syms, name);

    return result;
}

internal_proc Json_Node *
json_null() {
    Json_Node *result = json_new(JSON_NULL);

    return result;
}

internal_proc Json_Node *
json_parse_node(char **str) {
#define NEXT() ((*str)++)
#define VAL() (**str)
#define SKIP_WHITESPACE() do { while ( VAL() && (VAL() == ' ' || VAL() == '\t' || VAL() == '\n') ) { NEXT(); } } while(false)
#define EXPECT(C) do { if ( VAL() != C ) { fatal(0, 0, "expected '%c', got %1s\n", C, VAL()); } else { NEXT(); } } while(false)

    Json_Node *result = 0;
    SKIP_WHITESPACE();

    erstes_if ( VAL() == '{' ) {
        NEXT();
        Json_Pair **pairs = 0;
        if ( VAL() != '}' ) {
            do {
                Json_Node *name = json_parse_node(str);
                ASSERT(name->kind == JSON_STR);
                SKIP_WHITESPACE();
                EXPECT(':');

                Json_Node *value = json_parse_node(str);
                buf_push(pairs, json_pair(name->json_str.value, value));
                SKIP_WHITESPACE();
            } while ( (VAL() == ',') ? NEXT(), true : false );
        }

        EXPECT('}');
        result = json_object(pairs, buf_len(pairs));
    } else if ( VAL() == '[' ) {
        NEXT();
        Json_Node **nodes = 0;

        if ( VAL() != ']' ) {
            do {
                Json_Node *node = json_parse_node(str);
                buf_push(nodes, node);
                SKIP_WHITESPACE();
            } while ( (VAL() == ',') ? NEXT(), true : false );
        }

        EXPECT(']');
        result = json_array(nodes, buf_len(nodes));
    } else if ( VAL() == '"' ) {
        NEXT();
        char *start = *str;

        while ( VAL() != '"' ) {
            if ( VAL() == '\\' ) {
                NEXT();
            }
            NEXT();
        }

        result = json_str(start, *str);
        NEXT();
    } else if ( VAL() == '-' || VAL() >= '0' && VAL() <= '9' ) {
        b32 is_float = false;
        s32 mult = 1;

        if ( VAL() == '-' ) {
            mult = -1;
            NEXT();
        }

        char *base = *str;

        s32 int_value = 0;
        while ( VAL() >= '0' && VAL() <= '9' ) {
            int_value *= 10;
            int_value += VAL() - '0';
            NEXT();
        }

        f32 float_value = 0;
        if ( VAL() == '.' ) {
            NEXT();
            is_float = true;

            while ( VAL() >= '0' && VAL() <= '9' ) {
                NEXT();
            }

            float_value = strtof(base, NULL);
        }

        s32 exp = 1;
        if ( VAL() == 'e' || VAL() == 'E' ) {
            exp = 0;

            NEXT();
            if ( VAL() == '+' ) {
                NEXT();
            } else if ( VAL() == '-' ) {
                NEXT();
            }

            while ( VAL() >= '0' && VAL() <= '9' ) {
                exp *= 10;
                exp += VAL() - '0';
                NEXT();
            }
        }

        if ( is_float ) {
            result = json_float(float_value*mult, exp);
        } else {
            result = json_int(int_value*mult, exp);
        }
    } else if ( VAL() != 0 ) {
        char *start = *str;
        while ( VAL() >= 'a' && VAL() <= 'z' ) {
            NEXT();
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

#undef VAL
#undef NEXT
#undef EXPECT
#undef SKIP_WHITESPACE
}

internal_proc Json
json_parse(char *str) {
    char *ptr = str;

    Json_Node *node = json_parse_node(&ptr);
    Json result = { node };

    return result;
}
