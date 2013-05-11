#ifndef __HEAP_H
#define __HEAP_H

struct heap {
    void **heap;
    int (*compare)(void*, void*);
    unsigned count, max;
};

typedef struct heap heap;


struct heap *create_heap(int (*)(void*, void*));
void insert(struct heap*, void*);
void *peek(struct heap*);
void *pop(struct heap*);
unsigned size(struct heap*);

#endif

