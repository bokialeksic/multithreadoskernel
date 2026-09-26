#ifndef TCB_HPP
#define TCB_HPP

#include "../lib/hw.h"
#include "list.hpp"
#include "MemoryAllocator.hpp"

class TCB {
public:
    struct Context {
        uint64 ra;
        uint64 sp;
    };

    Context context;
    uint64* kernel_stack_space;

    enum Status {
        READY,
        RUNNING,
        BLOCKED,
        FINISHED
    };

    using Body = void (*)(void*);

    ~TCB();

    bool isFinished() const { return status == FINISHED; }
    bool isBlocked() const { return status == BLOCKED; }
    bool isReady() const { return status == READY; }
    bool isRunning() const { return status == RUNNING; }

    void setStatus(Status newStatus) { status = newStatus; }
    bool isMain() const { return main; }


    bool isKernelThread() const { return kernelThread; }


    void setRemainingSlice(time_t slice) { remainingSlice = slice; }
    time_t getRemainingSlice() const { return remainingSlice; }

    static TCB* createThread(Body body, void* arg, void* stack_top = nullptr, bool kernelThread = false);
    static TCB* createThreadWithoutStarting(Body body, void* arg, void* stack_top = nullptr, bool kernelThread = false);
    int start();

    static void dispatch();
    static void exit();
    static TCB* idle;
    static void initIdle();

    static void join(TCB* handle);
    void releaseAll();


    static void sleep(time_t ticks);


    static void tick();

    static TCB *running;

    void* operator new(size_t size) {
        size_t blocks = size / MEM_BLOCK_SIZE;
        if (size % MEM_BLOCK_SIZE != 0) blocks++;
        return MemoryAllocator::mem_alloc(blocks);
    }
    void* operator new[](size_t size) {
        size_t blocks = size / MEM_BLOCK_SIZE;
        if (size % MEM_BLOCK_SIZE != 0) blocks++;
        return MemoryAllocator::mem_alloc(blocks);
    }
    void operator delete(void *ptr) {
        MemoryAllocator::mem_free(ptr);
    }
    void operator delete[](void *ptr) {
        MemoryAllocator::mem_free(ptr);
    }

private:
    static void contSwitch(Context* old_ctx, Context* new_ctx);
    TCB(Body body, void* arg);
    static void idle_body(void*);

    Body body;
    void* arg;
    Status status;
    bool main;
    bool kernelThread;
    time_t remainingSlice; // Preostali broj perioda tajmera do isteka kvantuma

    List<TCB> joined_threads;


    static void thread_wrapper_entry();


    static void thread_wrapper_cpp();

    static uint64 constexpr STACK_SIZE = 1024;


    struct SleepNode {
        TCB* thread;
        time_t remaining;
        SleepNode* next;

        SleepNode(TCB* t, time_t r) : thread(t), remaining(r), next(nullptr) {}

        void* operator new(size_t size) {
            size_t blocks = size / MEM_BLOCK_SIZE;
            if (size % MEM_BLOCK_SIZE != 0) blocks++;
            return MemoryAllocator::mem_alloc(blocks);
        }
        void operator delete(void* ptr) {
            MemoryAllocator::mem_free(ptr);
        }
    };

    static SleepNode* sleepHead;
};


extern "C" int tcb_running_is_user_mode();

#endif // TCB_HPP