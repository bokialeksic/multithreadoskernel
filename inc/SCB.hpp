#ifndef SCB_HPP
#define SCB_HPP

#include "../lib/hw.h"
#include "MemoryAllocator.hpp"

class TCB;
class SCB {
public:
    ~SCB();

    static SCB* createSemaphore(unsigned init);

    int wait(unsigned n = 1);

    int signal(unsigned n = 1);

    static int closeSemaphore(SCB* sem);

    void* operator new(size_t size) {
        size_t blocks = size / MEM_BLOCK_SIZE;
        if (size % MEM_BLOCK_SIZE != 0) blocks++;
        return MemoryAllocator::mem_alloc(blocks);
    }
    void operator delete(void* ptr) {
        MemoryAllocator::mem_free(ptr);
    }

private:
    explicit SCB(unsigned init);

    struct WaitNode {
        TCB* thread;
        unsigned requested;
        int result;
        WaitNode* next;

        WaitNode(TCB* t, unsigned r) : thread(t), requested(r), result(0), next(nullptr) {}

        void* operator new(size_t size) {
            size_t blocks = size / MEM_BLOCK_SIZE;
            if (size % MEM_BLOCK_SIZE != 0) blocks++;
            return MemoryAllocator::mem_alloc(blocks);
        }
        void operator delete(void* ptr) {
            MemoryAllocator::mem_free(ptr);
        }
    };

    unsigned value;
    WaitNode* waitHead;
    WaitNode* waitTail;

    WaitNode* enqueue(TCB* thread, unsigned requested) {
        WaitNode* node = new WaitNode(thread, requested);
        if (node == nullptr) {
            return nullptr;
        }
        if (waitTail == nullptr) {
            waitHead = waitTail = node;
        } else {
            waitTail->next = node;
            waitTail = node;
        }
        return node;
    }

    WaitNode* dequeueFront() {
        if (waitHead == nullptr) return nullptr;
        WaitNode* node = waitHead;
        waitHead = waitHead->next;
        if (waitHead == nullptr) waitTail = nullptr;
        node->next = nullptr;
        return node;
    }
};

#endif // SCB_HPP
