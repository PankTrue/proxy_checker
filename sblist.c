#include "sblist.h"



sblist *sblist_create(size_t itemsize, size_t blockitems)
{
    sblist *ret;
    if((ret = (sblist *)malloc(sizeof(sblist))) == 0) { log_fatal("Out of memory!"); }
    memset(ret,0,sizeof(ret));


    ret->max_count  = blockitems;
    ret->blockitems = blockitems;
    ret->itemsize   = itemsize;

    if((ret->items = malloc(itemsize * blockitems)) == 0) { log_fatal("Out of memory!"); }

return ret;
}

void sblist_free(sblist *l)
{
    if(l->items == NULL)
        free(l->items);
    memset(l,0,sizeof(l));
}

void *sblist_get(sblist *l, size_t n)
{
    if(n > l->count) { log_fatal("out of list"); }
return (l->items + (n * l->itemsize));
}

void sblist_set(sblist *l, void *item, size_t pos)
{
    memcpy(sblist_get(l,pos),item,l->itemsize);
}

void sblist_add(sblist *l, void *item)
{
    char *tmp;
    if(l->count == l->max_count)
    {
        if((tmp = realloc(l->items,(l->max_count + l->blockitems) * l->itemsize)) == 0) { log_fatal("Out of memory!"); }
        l->max_count += l->blockitems;
        l->items       = tmp;
    }
    l->count++;
    sblist_set(l,item,l->count - 1);
}



void sblist_delete(sblist *l, size_t n)
{
    memset(sblist_get(l,n),0,l->itemsize);
    l->count--;
}
