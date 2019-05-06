#include "function.h"
#include <stdlib.h>

void function_init(struct function *fn, uint32_t ip, uint16_t nargs, struct env *parent) {
    fn->ip = ip;
    fn->nargs = nargs;
    if(parent != NULL) {
        env_copy(&fn->bound, parent);
    } else {
        fn->bound.nslots = 0;
    }
}

void function_free(struct function *fn, __attribute__((unused)) void *p)
{
    if(fn->bound.nslots)
        env_free(&fn->bound);
}