#include <cassert>
#include <cmath>
#include "hanayo.h"
#include "vm/src/string_.h"

#define fn(name) void hanayo::float_::name(struct vm *vm, int nargs)

fn(constructor) {
    struct value *val = &array_top(vm->stack);
    if(val->type == value::TYPE_FLOAT)
        return;
    else if(val->type == value::TYPE_INT)
        value_float(val, (double)val->as.integer);
    else if(val->type == value::TYPE_STR) {
        value_free(val);
        value_float(val, std::stof(string_data(val->as.str)));
    } else {
        value_free(val);
        value_float(val, 0);
    }
}
fn(round) {
    assert(nargs == 1);
    struct value *val = &array_top(vm->stack);
    value_int(val, (int64_t)::round(val->as.floatp));
}
