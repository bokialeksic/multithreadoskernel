#include "../lib/mem.h"
#include "../h/MemoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"

void* operator new (size_t size) {
    return mem_alloc(size);
}

void operator delete (void* ptr)  {
    mem_free(ptr);
}

void* operator new[] (size_t size) {
    return mem_alloc(size);
}

void operator delete[] (void* ptr)  {
    mem_free(ptr);
}

void Thread::runnerWrapper(void* threadObj) {
    if (threadObj == nullptr) return;
    Thread* t = (Thread*)threadObj;

    if (t->body != nullptr) {
        t->body(t->arg);
    } else {
        t->run();
    }
}

Thread::Thread(void (*body)(void*), void* arg) {
    this->body = body;
    this->arg = arg;
    this->myHandle = nullptr;
}

Thread::Thread() {
    this->body = nullptr;
    this->arg = nullptr;
    this->myHandle = nullptr;
}

Thread::~Thread() {
}
int Thread::start() {
    return thread_create(&this->myHandle, &Thread::runnerWrapper, this);
}
void Thread::dispatch() {
    thread_dispatch();
}
int Thread::sleep(time_t time) {
    return time_sleep(time);
}

Semaphore::Semaphore(unsigned init) {
    sem_open(&this->myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(this->myHandle);
}

int Semaphore::wait() {
    return sem_wait(this->myHandle);
}

int Semaphore::signal() {
    return sem_signal(this->myHandle);
}

struct TerminateNode {
    PeriodicThread* thread;
    TerminateNode* next;
    TerminateNode(PeriodicThread* t) : thread(t), next(nullptr) {}
};

static TerminateNode* terminatedHead = nullptr;

static bool isTerminated(PeriodicThread* p) {
    for (TerminateNode* node = terminatedHead; node != nullptr; node = node->next) {
        if (node->thread == p) return true;
    }
    return false;
}

static void markTerminated(PeriodicThread* p) {
    TerminateNode* node = new TerminateNode(p);
    node->next = terminatedHead;
    terminatedHead = node;
}

static void clearTerminated(PeriodicThread* p) {
    TerminateNode* prev = nullptr;
    TerminateNode* curr = terminatedHead;
    while (curr != nullptr) {
        if (curr->thread == p) {
            if (prev == nullptr) {
                terminatedHead = curr->next;
            } else {
                prev->next = curr->next;
            }
            delete curr;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

PeriodicThread::PeriodicThread(time_t period) : Thread() {
    this->period = period;
}

void PeriodicThread::run() {
    while (!isTerminated(this)) {
        periodicActivation();
        Thread::sleep(this->period);
    }
    clearTerminated(this);
}

void PeriodicThread::terminate() {
    markTerminated(this);
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}
