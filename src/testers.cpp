internal_proc PROC_CALLBACK(test_callable) {
    b32 result = operand->kind == VAL_PROC;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_defined) {
    b32 result = !val_is_undefined(operand);

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_divisibleby) {
    s32 s = val_int(arg_val(narg("num")));
    b32 result = (val_int(operand) % s) == 0;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_eq) {
    Val *result = val_bool(*operand == *arg_val(narg("s")));

    return result;
}

internal_proc PROC_CALLBACK(test_escaped) {
    implement_me();

    return val_bool(false);
}

internal_proc PROC_CALLBACK(test_even) {
    b32 result = (val_int(operand) & 0x1) != 0x1;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_ge) {
    s32 s = val_int(arg_val(narg("s")));

    b32 result = val_int(operand) >= s;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_gt) {
    s32 s = val_int(arg_val(narg("s")));

    b32 result = val_int(operand) > s;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_in) {
    Val *set = arg_val(narg("s"));

    if ( !set || set->kind != VAL_LIST ) {
        return val_bool(false);
    }

    b32 found = false;
    for ( int i = 0; i < set->len; ++i ) {
        Val *it = val_elem(set, i);

        if ( *it == *operand ) {
            found = true;
            break;
        }
    }

    return val_bool(found);
}

internal_proc PROC_CALLBACK(test_iterable) {
    b32 result = operand->kind == VAL_LIST || operand->kind == VAL_DICT;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_le) {
    s32 s = val_int(arg_val(narg("s")));

    b32 result = val_int(operand) <= s;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_lt) {
    s32 s = val_int(arg_val(narg("s")));

    b32 result = val_int(operand) < s;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_mapping) {
    b32 result = operand->kind == VAL_DICT || operand->kind == VAL_TUPLE || operand->kind == VAL_LIST;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_ne) {
    s32 s = val_int(arg_val(narg("s")));

    b32 result = val_int(operand) != s;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_none) {
    b32 result = operand->kind == VAL_NONE;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_number) {
    b32 result = operand->kind == VAL_INT || operand->kind == VAL_FLOAT;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_odd) {
    b32 result = (val_int(operand) & 0x1) == 0x1;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_sameas) {
    b32 result = ( operand->ptr == arg_val(narg("s"))->ptr );

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_sequence) {
    b32 result = test_iterable(operand, value, args, num_args, nargs, narg_keys,
            num_narg_keys, kwargs, num_kwargs, varargs, num_varargs);

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_string) {
    b32 result = operand->kind == VAL_STR;

    return val_bool(result);
}

internal_proc PROC_CALLBACK(test_undefined) {
    b32 result = val_is_undefined(operand);

    return val_bool(result);
}

