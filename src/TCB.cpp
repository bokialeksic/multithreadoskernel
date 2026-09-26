#include "../h/TCB.hpp"
#include "../h/scheduler.hpp"
#include "../h/syscall_c.hpp"
#include "../lib/console.h"
#include "../lib/mem.h"

TCB* TCB::running = nullptr;
TCB* TCB::idle = nullptr;
TCB::SleepNode* TCB::sleepHead = nullptr;

TCB::TCB(Body body, void* arg) {
    this->body = body;
    this->arg = arg;
    this->status = READY;
    this->main = false;
    this->kernelThread = false;
    this->kernel_stack_space = nullptr;
    this->context.ra = 0;
    this->context.sp = 0;
    this->remainingSlice = DEFAULT_TIME_SLICE;
}

TCB::~TCB() {
    if (kernel_stack_space != nullptr) {
        MemoryAllocator::mem_free(kernel_stack_space);
    }
}

TCB* TCB::createThread(Body body, void* arg, void* stack_top, bool kernelThread) {
    TCB* new_thread = createThreadWithoutStarting(body, arg, stack_top, kernelThread);
    if (new_thread != nullptr) {
        new_thread->status = READY;
        Scheduler::put(new_thread);
    }
    return new_thread;
}

TCB* TCB::createThreadWithoutStarting(Body body, void* arg, void* stack_top, bool kernelThread) {
    TCB* new_thread = new TCB(body, arg);
    if (new_thread == nullptr) return nullptr;

    new_thread->kernelThread = kernelThread;

    if (body != nullptr) {
        if (stack_top == nullptr) {
            delete new_thread;
            return nullptr;
        }

        size_t stack_elements = DEFAULT_STACK_SIZE / sizeof(uint64);
        new_thread->kernel_stack_space = (uint64*)stack_top - stack_elements;

        new_thread->context.ra = (uint64)&TCB::thread_wrapper_entry;
        new_thread->context.sp = (uint64)stack_top;
    } else {
        new_thread->main = true;
        new_thread->status = RUNNING;
    }

    return new_thread;
}

int TCB::start() {
    if (status == READY && this != idle) {
        Scheduler::put(this);
        return 0;
    }
    return -1;
}

void TCB::dispatch() {
    TCB* old_thread = running;

    if (old_thread != TCB::idle && old_thread->status != FINISHED && old_thread->status != BLOCKED) {
        old_thread->status = READY;
        Scheduler::put(old_thread);
    }

    TCB::running = Scheduler::get();

    if (TCB::running == nullptr) {
        if (old_thread->status != FINISHED && old_thread->status != BLOCKED) {
            TCB::running = old_thread;
        } else {
            TCB::running = TCB::idle;
        }
    }
    TCB::running->status = RUNNING;


    TCB::running->remainingSlice = DEFAULT_TIME_SLICE;



    if (old_thread != TCB::running) {
        contSwitch(&(old_thread->context), &(TCB::running->context));
    }
}


void TCB::thread_wrapper_cpp() {
    if (TCB::running->body != nullptr) {
        TCB::running->body(TCB::running->arg);
    }

    if (tcb_running_is_user_mode()) {
        thread_exit(); // ecall (0x12) - legalan prelaz u S-mod pre dispatch-a
    } else {
        TCB::exit();
    }
}


extern "C" int tcb_running_is_user_mode() {
    return (TCB::running->isMain() || TCB::running->isKernelThread()) ? 0 : 1;
}

void TCB::exit() {
    TCB::running->status = FINISHED;
    TCB::running->releaseAll();
    TCB::dispatch();
}

void TCB::initIdle() {
    if (TCB::idle == nullptr) {
        TCB::idle = new TCB(&TCB::idle_body, nullptr);

        size_t blocks = (STACK_SIZE * sizeof(uint64)) / MEM_BLOCK_SIZE;
        if ((STACK_SIZE * sizeof(uint64)) % MEM_BLOCK_SIZE != 0) {
            blocks++;
        }

        TCB::idle->kernel_stack_space = (uint64*)MemoryAllocator::mem_alloc(blocks);
        uint64* top_of_stack = TCB::idle->kernel_stack_space + STACK_SIZE;

        TCB::idle->context.ra = (uint64)&TCB::thread_wrapper_entry;
        TCB::idle->context.sp = (uint64)top_of_stack;
        TCB::idle->status = READY;
        TCB::idle->kernelThread = true;
    }
}

void TCB::idle_body(void* arg) {
    while (1) {
        //dispatch();
    }
}

void TCB::join(TCB* handle) {
    if (handle != nullptr && !handle->isFinished() && handle != TCB::running) {
        TCB::running->status = BLOCKED;
        handle->joined_threads.putBack(TCB::running);
        TCB::dispatch();
    }
}

void TCB::releaseAll() {
    while (joined_threads.peekFirst() != nullptr) {
        TCB* t = joined_threads.removeFirst();
        t->status = READY;
        Scheduler::put(t);
    }
}


void TCB::sleep(time_t ticks) {
    if (ticks == 0) return;

    time_t target = ticks;
    SleepNode* prev = nullptr;
    SleepNode* curr = sleepHead;

    while (curr != nullptr && target >= curr->remaining) {
        target -= curr->remaining;
        prev = curr;
        curr = curr->next;
    }

    TCB* self = TCB::running;
    SleepNode* node = new SleepNode(self, target);
    if (node == nullptr) {
        return;
    }
    node->next = curr;
    if (curr != nullptr) {
        curr->remaining -= target;
    }
    if (prev == nullptr) {
        sleepHead = node;
    } else {
        prev->next = node;
    }

    self->setStatus(BLOCKED);
    TCB::dispatch();
}


void TCB::tick() {
    while (sleepHead != nullptr && sleepHead->remaining == 0) {
        SleepNode* node = sleepHead;
        sleepHead = sleepHead->next;

        node->thread->setStatus(READY);
        Scheduler::put(node->thread);

        delete node;
    }

    if (sleepHead != nullptr) {
        sleepHead->remaining--;

        while (sleepHead != nullptr && sleepHead->remaining == 0) {
            SleepNode* node = sleepHead;
            sleepHead = sleepHead->next;

            node->thread->setStatus(READY);
            Scheduler::put(node->thread);

            delete node;
        }
    }
}