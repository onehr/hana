#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "value.h"
#include "array.h"

struct hmap;
struct string;

const struct value *hmap_get(struct hmap *, const char *);
void hmap_set(struct hmap *, const char *, struct value);

#ifdef __cplusplus
}
#endif
