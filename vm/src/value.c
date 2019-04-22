#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "string_.h"
#include "value.h"
#include "dict.h"
#include "array_obj.h"

void value_int(struct value *val, int64_t data) {
    val->type = TYPE_INT;
    val->as.integer = data;
}
void value_float(struct value *val, double data) {
    val->type = TYPE_FLOAT;
    val->as.floatp = data;
}
void value_str(struct value *val, const char *data) {
    val->type = TYPE_STR;
    val->as.str = malloc(string_size(data));
    string_init(val->as.str, data);
}
void value_strmov(struct value *val, struct string_header *str) {
    val->type = TYPE_STR;
    val->as.str = str;
}
void value_native(struct value *val, value_fn fn) {
    val->type = TYPE_NATIVE_FN;
    val->as.fn = fn;
}
void value_function(struct value *val, uint32_t ip, int nargs) {
    val->type = TYPE_FN;
    val->as.ifn.ip = ip;
    val->as.ifn.nargs = nargs;
}
void value_dict(struct value *val) {
    val->type = TYPE_DICT;
    val->as.dict = malloc(sizeof(struct dict));
    dict_init(val->as.dict);
}
void value_dict_copy(struct value *val, struct dict *dict) {
    val->type = TYPE_DICT;
    val->as.dict = dict;
    dict->refs++;
}
void value_array(struct value *val) {
    val->type = TYPE_ARRAY;
    val->as.array = malloc(sizeof(struct array_obj));
    array_obj_init(val->as.array);
}
void value_array_n(struct value *val, size_t n) {
    val->type = TYPE_ARRAY;
    val->as.array = malloc(sizeof(struct array_obj));
    array_obj_init_n(val->as.array, n);
}
void value_native_obj(struct value *val, void *data, native_obj_free_fn free) {
    val->type = TYPE_NATIVE_OBJ;
    val->as.native = malloc(sizeof(struct native_obj));
    native_obj_init(val->as.native, data, free);
}

struct _rc_struct { uint32_t refs; };
void value_free(struct value *val) {
#define X(x) if(val->as.x->refs == 0) {free(val->as.x);}
    switch(val->type) {
        case TYPE_STR:        ((struct _rc_struct*)val->as.str)->refs++;    X(str) break;
        case TYPE_DICT:       ((struct _rc_struct*)val->as.dict)->refs++;   X(dict) break;
        case TYPE_ARRAY:      ((struct _rc_struct*)val->as.array)->refs++;  X(array) break;
        case TYPE_NATIVE_OBJ: ((struct _rc_struct*)val->as.native)->refs++; X(native) break;
        default: break;
    }
    val->type = TYPE_NIL;
}

void value_print(struct value *val) {
    if(val->type == TYPE_INT)
        printf("%ld", val->as.integer);
    else if(val->type == TYPE_FLOAT)
        printf("%f", val->as.floatp);
    else if(val->type == TYPE_STR)
        printf("\"%s\"", string_data(val->as.str));
    else if(val->type == TYPE_NATIVE_FN)
        printf("[native fn %lx]", (intptr_t)val->as.fn);
    else if(val->type == TYPE_FN)
        printf("[fn %d]", (uint32_t)val->as.ifn.ip);
    else if(val->type == TYPE_DICT)
        printf("[dict %ld]", (intptr_t)val->as.dict);
    else if(val->type == TYPE_ARRAY)
        printf("[array %ld]", (intptr_t)val->as.array);
    else if(val->type == TYPE_NATIVE_OBJ)
        printf("[native obj %ld]", (intptr_t)val->as.native);
    else {
        printf("nil");
    }
}

void value_copy(struct value *dst, struct value *src) {
    dst->type = src->type;
    switch(src->type) {
        case TYPE_STR:        ((struct _rc_struct*)src->as.str)->refs++; break;
        case TYPE_DICT:       ((struct _rc_struct*)src->as.dict)->refs++; break;
        case TYPE_ARRAY:      ((struct _rc_struct*)src->as.array)->refs++; break;
        case TYPE_NATIVE_OBJ: ((struct _rc_struct*)src->as.native)->refs++; break;
        default: break;
    }
    dst->as = src->as;
}

// arith
#define arith_op(name, op, custom) \
void value_ ## name (struct value *result, const struct value *left, const struct value *right) { \
    if(left->type == TYPE_INT) { \
        if(right->type == TYPE_FLOAT) \
            value_float(result, (float)left->as.integer op right->as.floatp); \
        else if(right->type == TYPE_INT) \
            value_int(result, left->as.integer op right->as.integer); \
    } else if(left->type == TYPE_FLOAT) { \
        if(right->type == TYPE_INT) \
            value_float(result, left->as.floatp op (float)right->as.integer); \
        else if(right->type == TYPE_FLOAT) \
            value_int(result, left->as.floatp op right->as.floatp); \
    } custom \
    else { value_int(result, 0); }\
}
arith_op(add, +,
    else if(left->type == TYPE_STR && right->type == TYPE_STR) {
        struct string_header *s = string_alloc(string_len(left->as.str)+string_len(right->as.str));
        char *ss = string_data(s); ss[0] = 0;
        strcpy(ss, string_data(left->as.str));
        strcpy(ss+string_len(left->as.str), string_data(right->as.str));
        result->type = TYPE_STR;
        result->as.str = s;
    }
)
arith_op(sub, -,)
arith_op(mul, *,
    else if(left->type == TYPE_STR && right->type == TYPE_INT) {
        if(right->as.integer == 0) {
            value_str(result, "");
        } else {
            size_t n = string_len(left->as.str)*right->as.integer;
            struct string_header *s = string_alloc(n);
            char *ss = string_data(s); ss[0] = 0;
            for(size_t i = 0; i < right->as.integer; i++)
                strcat(ss, string_data(left->as.str));
            result->type = TYPE_STR;
            result->as.str = s;
        }
    } else if(left->type == TYPE_ARRAY && right->type == TYPE_INT) {
        size_t length = left->as.array->data.length*(size_t)right->as.integer;
        value_array_n(result, length);
        for(size_t i = 0; i < right->as.integer; i++) {
            for(size_t j = 0; j < left->as.array->data.length; j++) {
                size_t index = left->as.array->data.length*i+j;
                value_copy(&result->as.array->data.data[index], &left->as.array->data.data[j]);
            }
        }
    }
)
void value_div(struct value *result, const struct value *left, const struct value *right) {
    if( (left->type == TYPE_FLOAT && right->type == TYPE_INT) ||
        (left->type == TYPE_INT && right->type == TYPE_FLOAT) ) {
        const struct value *floatv = left->type == TYPE_FLOAT ? left : right;
        const struct value *intv = left->type == TYPE_INT ? left : right;
        value_float(result, floatv->as.floatp / (double)intv->as.integer);
    } else if(left->type == TYPE_INT && right->type == TYPE_INT) {
        value_float(result, (double)left->as.integer / (double)right->as.integer);
    } else if(left->type == TYPE_FLOAT && right->type == TYPE_FLOAT) {
        value_float(result, left->as.floatp / right->as.floatp);
    }
}
void value_mod(struct value *result, const struct value *left, const struct value *right) {
    assert(left->type == right->type);
    if(left->type == TYPE_INT) {
        value_int(result, left->as.integer % right->as.integer);
    }
}

// logic
#define logic_op(name, op) \
void value_ ## name (struct value *result, const struct value *left, const struct value *right) { \
    value_int(result, value_is_true(left) op value_is_true(right)); \
}
logic_op(and, &&)
logic_op(or, ||)

// comparison
#define strcmp_op(cond) \
    else if(left->type == TYPE_STR && right->type == TYPE_STR) { \
        value_int(result, string_cmp(left->as.str, right->as.str) cond); \
    }
arith_op(eq, ==,
    strcmp_op(== 0)
    else if(left->type == TYPE_NATIVE_FN && right->type == TYPE_NATIVE_FN)
        value_int(result, left->as.fn == right->as.fn);
    else if(left->type == TYPE_FN && right->type == TYPE_FN)
        value_int(result, left->as.ifn.ip == right->as.ifn.ip);
    else if(left->type == TYPE_DICT && right->type == TYPE_DICT)
        value_int(result, left->as.dict == right->as.dict);
    else if(left->type == TYPE_NIL && right->type == TYPE_NIL)
        value_int(result, 1);
)
arith_op(neq, !=,
    strcmp_op(!= 0)
    else if(left->type == TYPE_NATIVE_FN && right->type == TYPE_NATIVE_FN)
        value_int(result, left->as.fn != right->as.fn);
    else if(left->type == TYPE_FN && right->type == TYPE_FN)
        value_int(result, left->as.ifn.ip != right->as.ifn.ip);
    else if(left->type == TYPE_DICT && right->type == TYPE_DICT)
        value_int(result, left->as.dict != right->as.dict);
    else if(left->type == TYPE_NIL && right->type == TYPE_NIL)
        value_int(result, 0);
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
int value_is_true(const struct value *val) {
    if(val->type == TYPE_INT)
        return val->as.integer > 0;
    else if(val->type == TYPE_FLOAT)
        return val->as.floatp > 0;
    else if(val->type == TYPE_STR)
        return string_len(val->as.str) > 0;
    else
        return 0;
}
