#include "../h/SCB.hpp"
#include "../h/TCB.hpp"
#include "../h/scheduler.hpp"
#include "../lib/console.h"
SCB::SCB(unsigned init) {
    this->value = init;
    this->waitHead = nullptr;
    this->waitTail = nullptr;
}

SCB::~SCB() {
    while (waitHead != nullptr) {
        WaitNode* node = dequeueFront();
        delete node;
    }
}

SCB* SCB::createSemaphore(unsigned init) {
    return new SCB(init);
}

int SCB::wait(unsigned n) {
    if (value >= n) {
        value -= n;
        return 0;
    }

    TCB* self = TCB::running;

    WaitNode* myNode = enqueue(self, n);
    if (myNode == nullptr) {

        return -1;
    }
    self->setStatus(TCB::BLOCKED);
    TCB::dispatch();
    int result = myNode->result;
    delete myNode;
    return result;
}

int SCB::signal(unsigned n) {
    value += n;

    while (waitHead != nullptr && waitHead->requested <= value) {
        WaitNode* node = dequeueFront();
        value -= node->requested;
        node->result = 0;
        node->thread->setStatus(TCB::READY);
        Scheduler::put(node->thread);
    }

    return 0;
}

int SCB::closeSemaphore(SCB* sem) {
    if (sem == nullptr) return -1;

    WaitNode* node;
    while ((node = sem->dequeueFront()) != nullptr) {
        node->result = -1;
        node->thread->setStatus(TCB::READY);
        Scheduler::put(node->thread);
    }

    delete sem;
    return 0;
}
