struct Env {
    Arena arena;
};

global_var Env global_env;
global_var Map global_env_map;

internal_proc void
init_env() {
    arena_init(&global_env.arena, MB(100));
}

internal_proc void *
env_add(char *key) {
    /* @TODO: 'key' zusammen mit 'scope' hashen und als schlüssel für die map nutzen */
    void *result = map_get(&global_env_map, key);

    if ( !result ) {
        result = ALLOC_SIZE(&global_env.arena, sizeof(Val));
        map_put(&global_env_map, key, result);
    }

    return result;
}

