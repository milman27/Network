#ifndef ARENA_H
#define ARENA_H
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define assert(x) if(!(x)){printf("assertion is incorrect!(%s,%d)\n", __FILE__, __LINE__);}
#define PAGESIZE 4096
#define MEMUNIT 128 * 1024

typedef uint8_t arena_id;

struct arena_head{
    size_t size;
    size_t usage;
    void* next;
};

void* arenaAlloc(arena_id arena, size_t size);
void arenaFree(arena_id arena);
void viewMem(arena_id arena);

#endif
