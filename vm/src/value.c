#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "value.h"

void value_int(struct value *val, int data) {
    val->type = TYPE_INT;
    val->as.integer = data;
}
void value_float(struct value *val, float data) {
    val->type = TYPE_FLOAT;
    val->as.floatp = data;
}
void value_str(struct value *val, const char *data) {
    val->type = TYPE_STR;
    val->as.str = strdup(data);
}
void value_function(struct value *val, value_fn fn) {
    val->type = TYPE_NATIVE_FN;
    val->as.fn = fn;
}

void value_free(struct value *val) {
    if(val->type == TYPE_STR)
        free(val->as.str);
    val->type = TYPE_NIL;
}

void value_print(struct value *val) {
    if(val->type == TYPE_INT)
        printf("%d", val->as.integer);
    else if(val->type == TYPE_FLOAT)
        printf("%f", val->as.floatp);
    else if(val->type == TYPE_STR)
        printf("\"%s\"", val->as.str);
    else if(val->type == TYPE_NATIVE_FN)
        printf("[fn %lx]", (intptr_t)val->as.fn);
    else {
        printf("nil");
    }
}

void value_copy(struct value *left, struct value *right) {
    memcpy(left, right, sizeof(struct value));
}

// arith
#define arith_op(name, op, custom) \
void value_ ## name (struct value *result, struct value *left, struct value *right) { \
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
}
arith_op(add, +,
    else if(left->type == TYPE_STR) {
        char s[strlen(left->as.str)+strlen(right->as.str)+1];
        strcpy(s, left->as.str);
        strcat(s, right->as.str);
        s[sizeof(s)] = 0;
        value_str(result, s);
        value_free(left);
        value_free(right);
    }
)
arith_op(sub, -,)
arith_op(mul, *,
    else if( (left->type == TYPE_STR && right->type == TYPE_INT) ||
             (left->type == TYPE_INT && right->type == TYPE_STR) ) {
        struct value *strv = left->type == TYPE_STR ? left : right;
        struct value *intv = left->type == TYPE_INT ? left : right;
        if(intv->as.integer == 0) {
            value_free(left);
            value_free(right);
            value_str(result, "");
        } else {
            char s[strlen(strv->as.str)*intv->as.integer+1];
            s[0] = 0; s[sizeof(s)] = 0;
            for(int i = 0; i < intv->as.integer; i++)
                strcat(s, strv->as.str);
            value_free(left);
            value_free(right);
            value_str(result, s);
        }
    }
)
arith_op(div, /,)
void value_mod(struct value *result, struct value *left, struct value *right) {
    assert(left->type == right->type);
    if(left->type == TYPE_INT) {
        value_int(result, left->as.integer % right->as.integer);
    }
}

// logic
#define logic_op(name, op) \
void value_ ## name (struct value *result, struct value *left, struct value *right) { \
    value_int(result, value_is_true(left) op value_is_true(right)); \
}
logic_op(and, &&)
logic_op(or, ||)

// comparison
arith_op(lt, <,)
arith_op(leq, <=,)
arith_op(gt, >=,)
arith_op(geq, >=,)
arith_op(eq, ==,)
arith_op(neq, !=,)

// boolean
int value_is_true(const struct value *val) {
    if(val->type == TYPE_INT)
        return val->as.integer > 0;
    else if(val->type == TYPE_FLOAT)
        return val->as.floatp > 0;
    else if(val->type == TYPE_STR)
        return strlen(val->as.str) > 0;
    else
        return 0;
}
