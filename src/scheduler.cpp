#include "../h/scheduler.hpp"
#include "../h/TCB.hpp"

List<TCB> Scheduler::readyQueue;

void Scheduler::put(TCB* tcb) {
    if (tcb != nullptr) {
        Scheduler::readyQueue.putBack(tcb);
    }
}

TCB* Scheduler::get() {
    return Scheduler::readyQueue.removeFirst();
}
extern "C" {
    void* __dso_handle = nullptr;


    int __cxa_atexit(void (*destructor)(void*), void* arg, void* dso) {
        return 0;
    }
}
