#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string_.h"
#include "value.h"
#include "vm.h"
#include "hmap.h"
#include "array_obj.h"
#include "function.h"

// non-primitives
struct value value_int(int64_t n) {
    return (struct value){
        .type = TYPE_INT,
        .as.integer = n,
    };
}
struct value value_float(double n) {
    return (struct value){
        .type = TYPE_FLOAT,
        .as.floatp = n,
    };
}
struct value value_str(const char *data, const struct vm *vm) {
    return value_pointer(TYPE_STR, string_malloc(data, vm));
}
struct value value_function(uint32_t ip, uint16_t nargs, struct env *env, const struct vm *vm) {
    return value_pointer(TYPE_FN, function_malloc(ip, nargs, env, vm));
}
struct value value_dict(const struct vm *vm) {
    return value_pointer(TYPE_DICT, dict_malloc(vm));
}
struct value value_dict_n(size_t n, const struct vm *vm) {
    return value_pointer(TYPE_DICT, dict_malloc_n(vm, n));
}
struct value value_array(const struct vm *vm) {
    return value_pointer(TYPE_ARRAY, array_obj_malloc(vm));
}
struct value value_array_n(size_t n, const struct vm *vm) {
    return value_pointer(TYPE_ARRAY, array_obj_malloc_n(n, vm));
}

struct value value_interpreter_error() {
    return value_pointer(TYPE_INTERPRETER_ERROR, 0);
}

// arith
#define arith_op(name, op, custom)                                                                      \
    struct value value_##name(const struct value left, const struct value right, const struct vm *vm) { \
        switch (left.type) {                                                                            \
            case TYPE_INT: {                                                                            \
                switch (right.type) {                                                                   \
                    case TYPE_FLOAT:                                                                    \
                        return value_float((double)value_get_int(left) op right.as.floatp);             \
                    case TYPE_INT:                                                                      \
                        return value_int(value_get_int(left) op value_get_int(right));                  \
                    default:                                                                            \
                        return value_interpreter_error();                                               \
                }                                                                                       \
            }                                                                                           \
            case TYPE_FLOAT: {                                                                          \
                switch (right.type) {                                                                   \
                    case TYPE_INT:                                                                      \
                        return value_int(left.as.floatp op(double) value_get_int(right));               \
                    case TYPE_FLOAT:                                                                    \
                        return value_float(left.as.floatp op right.as.floatp);                          \
                    default:                                                                            \
                        return value_interpreter_error();                                               \
                }                                                                                       \
            }                                                                                           \
                custom                                                                                  \
        }                                                                                               \
        return value_interpreter_error();                                                               \
    }
arith_op(add, +,
    case TYPE_STR: {
        if(right.type != TYPE_STR) {
            return value_interpreter_error();
        }
        return value_pointer(TYPE_STR, string_append(value_get_pointer(left), value_get_pointer(right), vm));
    }
)
arith_op(sub, -,)
arith_op(mul, *,
    case TYPE_STR: {
        if(right.type == TYPE_INT) {
            if(value_get_int(right) == 0) {
                return value_interpreter_error();
            }
            return value_pointer(TYPE_STR, string_repeat(value_get_pointer(left), value_get_int(right), vm));
        }
        return value_interpreter_error();
    }
    case TYPE_ARRAY: {
        if(right.type == TYPE_INT) {
            return value_pointer(TYPE_ARRAY, array_obj_repeat(value_get_pointer(left), (size_t)value_get_int(right), vm));
        }
        return value_interpreter_error();
    }
)

struct value value_div(const struct value left, const struct value right, const struct vm *_) {
    switch(left.type) {
    case TYPE_INT: {
        switch(right.type) {
            case TYPE_FLOAT:
                return value_float((double)value_get_int(left) / right.as.floatp);
            case TYPE_INT:
                return value_float((double)value_get_int(left) / (double)value_get_int(right));
        }
        return value_interpreter_error();
    }
    case TYPE_FLOAT: {
        switch(right.type) {
            case TYPE_INT:
                return value_float(left.as.floatp / (double)value_get_int(right));
            case TYPE_FLOAT:
                return value_float(left.as.floatp / right.as.floatp);
            }
        return value_interpreter_error();
    }
    default: return value_interpreter_error(); }
}
struct value value_mod(const struct value left, const struct value right, const struct vm *_) {
    if(left.type == TYPE_INT && right.type == TYPE_INT) {
        return value_int(value_get_int(left) % value_get_int(right));
    } else
        return value_interpreter_error();
}
struct value value_bitwise_and(const struct value left, const struct value right, const struct vm *_) {
    if (left.type == TYPE_INT && right.type == TYPE_INT) {
        return value_int(value_get_int(left) & value_get_int(right));
    } else
        return value_interpreter_error();
}
struct value value_bitwise_or(const struct value left, const struct value right, const struct vm *_) {
    if (left.type == TYPE_INT && right.type == TYPE_INT) {
        return value_int(value_get_int(left) | value_get_int(right));
    } else
        return value_interpreter_error();
}
struct value value_bitwise_xor(const struct value left, const struct value right, const struct vm *_) {
    if (left.type == TYPE_INT && right.type == TYPE_INT) {
        return value_int(value_get_int(left) ^ value_get_int(right));
    } else
        return value_interpreter_error();
}

// in place
// returns 1 if it CAN do it in place
int value_iadd(struct value left, const struct value right) {
    switch (left.type) {
        case TYPE_STR: {
            switch (right.type) {
                case TYPE_STR: {
                    string_append_in_place(value_get_pointer(left), value_get_pointer(right));
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

int value_imul(struct value left, const struct value right) {
    switch (left.type) {
        case TYPE_STR: {
            switch (right.type) {
                case TYPE_INT: {
                    string_repeat_in_place(value_get_pointer(left), value_get_int(right));
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

// comparison
#define strcmp_op(cond) \
    case TYPE_STR: \
        return value_int(right.type == TYPE_STR && string_cmp(value_get_pointer(left), value_get_pointer(right)) cond); \
        break;
arith_op(eq, ==,
    strcmp_op(== 0)
    case TYPE_NATIVE_FN:
    case TYPE_FN:
    case TYPE_DICT:
        return value_int(value_get_int(left) == value_get_int(right));
    case TYPE_NIL:
        return value_int(right.type == TYPE_NIL);
)
arith_op(neq, !=,
    strcmp_op(!= 0)
    case TYPE_NATIVE_FN:
    case TYPE_FN:
    case TYPE_DICT:
        return value_int(value_get_int(left) != value_get_int(right));
    case TYPE_NIL:
        return value_int(right.type != TYPE_NIL);
)
arith_op(lt, <,
    strcmp_op(< 0)
)
arith_op(leq, <=,
    strcmp_op(<= 0)
)
arith_op(gt, >,
    strcmp_op(> 0)
)
arith_op(geq, >=,
    strcmp_op(>= 0)
)

// boolean
bool value_is_true(const struct value val) {
    switch (val.type) {
        case TYPE_INT:
            return value_get_int(val) > 0;
        case TYPE_FLOAT:
            return val.as.floatp > 0;
        case TYPE_STR:
            return !string_is_empty(value_get_pointer(val));
        default:
            return 0;
    }
}

struct dict *value_get_prototype(const struct vm *vm, const struct value val) {
    switch (val.type) {
        case TYPE_STR:
            return vm->dstr;
        case TYPE_INT:
            return vm->dint;
        case TYPE_FLOAT:
            return vm->dfloat;
        case TYPE_ARRAY:
            return vm->darray;
        case TYPE_DICT: {
            const struct value *p = dict_get(value_get_pointer(val), "prototype");
            if (p == NULL) return NULL;
            return value_get_pointer(*p);
        }
    }
    return NULL;
}
