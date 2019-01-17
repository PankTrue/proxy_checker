#ifndef SBLIST_H
#define SBLIST_H

#include <stdint.h>
#include <memory.h>

#include "log.h"

typedef struct
{
    size_t count;
    size_t max_count;
    size_t itemsize;
    size_t blockitems;
    char *items;
}sblist;


sblist* sblist_create(size_t itemsize, size_t blockitems);
void sblist_free(sblist* l);


void* sblist_get(sblist* l, size_t n);
void sblist_set(sblist* l, void* item, size_t pos);
void sblist_add(sblist* l, void* item);
void sblist_delete(sblist* l, size_t n);


#endif // SBLIST_H
