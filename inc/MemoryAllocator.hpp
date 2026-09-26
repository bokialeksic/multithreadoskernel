#pragma once
#include "../lib/hw.h"

struct MemBlock{
    MemBlock* next;
    MemBlock* prev;
    size_t size;
};

class MemoryAllocator{
public:
    static void init();

    static void* mem_alloc(size_t s); // s je broj blokova

    static int mem_free(void* ptr);

private:
    static MemBlock* head;
    static MemBlock* tail;

    MemoryAllocator() = delete;
};

