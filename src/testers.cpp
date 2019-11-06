internal_proc TEST_CALLBACK(test_callable) {
    b32 result = type_is_callable(type);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_defined) {
    b32 result = !val_is_undefined(operand);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_divisibleby) {
    Resolved_Arg *arg = narg("s");
    s32 s = val_int(arg->val);

    b32 result = (val_int(operand) % s) == 0;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_eq) {
    Resolved_Arg *arg = narg("s");

    Val *result = val_bool(*operand == *arg->val);

    return result;
}

internal_proc TEST_CALLBACK(test_escaped) {
    implement_me();

    return val_bool(false);
}

internal_proc TEST_CALLBACK(test_even) {
    b32 result = (val_int(operand) & 0x1) != 0x1;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_ge) {
    Resolved_Arg *arg = narg("s");
    s32 s = val_int(arg->val);

    b32 result = val_int(operand) >= s;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_gt) {
    Resolved_Arg *arg = narg("s");
    s32 s = val_int(arg->val);

    b32 result = val_int(operand) > s;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_in) {
    Resolved_Arg *arg = narg("s");
    Val *set = arg->val;

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

internal_proc TEST_CALLBACK(test_iterable) {
    b32 result = type->flags & TYPE_FLAGS_CALLABLE;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_le) {
    Resolved_Arg *arg = narg("s");
    s32 s = val_int(arg->val);

    b32 result = val_int(operand) <= s;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_lt) {
    Resolved_Arg *arg = narg("s");
    s32 s = val_int(arg->val);

    b32 result = val_int(operand) < s;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_ne) {
    Resolved_Arg *arg = narg("s");
    s32 s = val_int(arg->val);

    b32 result = val_int(operand) != s;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_none) {
    b32 result = val_bool(operand) == -1;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_number) {
    b32 result = type_is_arithmetic(type);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_odd) {
    b32 result = (val_int(operand) & 0x1) == 0x1;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_sameas) {
    Resolved_Arg *arg = narg("s");

    b32 result = operand->ptr == arg->val->ptr;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_sequence) {
    b32 result = test_iterable(operand, type, nargs, kwargs, varargs, num_varargs);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_string) {
    b32 result = operand->kind == VAL_STR;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_undefined) {
    b32 result = val_is_undefined(operand);

    return val_bool(result);
}

