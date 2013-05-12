#ifndef __HEAP_H
#define __HEAP_H

struct heap {
    void **heap;
    int (*compare)(void*, void*);
    void (*destruct)(void*);
    unsigned count, max;
};

typedef struct heap heap;

struct heap *create_heap(int (*)(void*, void*), void (*)(void*));
void insert(struct heap*, void*);
void *peek(struct heap*);
void *pop(struct heap*);
void remove_if(struct heap*, int (*)(void*)); 
void traverse(struct heap*, void (*)(void*));
unsigned size(struct heap*);

#endif

