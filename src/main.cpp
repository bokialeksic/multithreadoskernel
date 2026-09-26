#include "../lib/console.h"
#include "../h/syscall_c.hpp"
#include "../h/processor.hpp"
#include "../lib/mem.h"
#include "../h/MemoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"
#include "../h/Console.hpp"
#include "../test/printing.hpp"

extern void  userMain();

SCB* waitUser = nullptr;


void userWrapper(void* arg)
{
    printString("user main start:\n");
    userMain();
    printString("user main end:\n");
    waitUser->signal();
}


int main() {
    MemoryAllocator::init();
    TCB::running = TCB::createThreadWithoutStarting(nullptr, nullptr);
    TCB::initIdle();

    Processor::init();

    ConsoleDriver::init();

    Processor::enableInterrupts();

    sem_open(&waitUser, 0);

    thread_t user;
    thread_create(&user, userWrapper, nullptr);

    waitUser->wait();

    while (ConsoleDriver::pendingCount() > 0) {
        thread_dispatch();
    }

    Processor::shutdown();

    return 0;
}
/*int main() {
    MemoryAllocator::init();
    TCB::running = TCB::createThreadWithoutStarting(nullptr, nullptr);
    TCB::initIdle();

    Processor::init();

    ConsoleDriver::init();
    ConsoleDriver::startTxThread();
    Processor::enableInterrupts();

    //putc('\n');
    putc('?'); putc('\n'); // signal da je program spreman za unos

    while (true) {
        int key = getc();
        //int recv = ConsoleDriver::rxReceivedCount;
        //int cons = ConsoleDriver::rxConsumedCount;
        //putc('R'); putc('0' + (recv % 10));
        //putc('C'); putc('0' + (cons % 10));
        //putc('[');
        // ispisujemo hex vrednost pritisnutog tastera, da vidimo TACNO
        // koji bajt(ovi) stignu
        const char* hexdigits = "0123456789ABCDEF";
        putc(hexdigits[(key >> 4) & 0xF]);
        putc(hexdigits[key & 0xF]);
        putc(']');
        putc('!');
        if (key == 0x7e) {
            //__putc('\n');
            putc('E'); putc('S'); putc('C'); putc('!');
            putc('\n');
            break;
        }
    }

    putc('K'); putc('O');
    putc('\n');

    Processor::shutdown();

    return 0;
}*/
/*
int main() {
    MemoryAllocator::init();
    TCB::running = TCB::createThreadWithoutStarting(nullptr, nullptr);
    TCB::initIdle();
    Processor::init();
    ConsoleDriver::init();
    Processor::enableInterrupts();
    for (int i = 0; i < 100; i++) {
        thread_dispatch();
        putc('0' + (i % 10));
    }

    putc('\n');
    putc('K'); putc('O');
    putc('\n');
    while (ConsoleDriver::pendingCount() > 0) {
        thread_dispatch();
    }
    for (int i = 0; i < 1000; i++) {
        thread_dispatch();  // dodatno cekanje da QEMU flush-uje izlaz
    }
    Processor::shutdown();
    return 0;
}*/
/*void taskA(void* arg) {
    uint64 limit = 50;
    for (uint64 i = 0; i <= limit; i += 1) {
        if (i % 5 == 0)
        {
            printString("A: ");
            printInt(i);
            printString("\n");
        }
    }
}
void taskB(void* arg) {
    uint64 limit = 100;
    for (uint64 i = 0; i <= limit; i += 1) {
        if (i % 5 == 0)
        {
            printString("B: ");
            printInt(i);
            printString("\n");
        }
    }
}
int main() {
    MemoryAllocator::init();
    TCB::running = TCB::createThreadWithoutStarting(nullptr, nullptr);
    TCB::initIdle();
    ConsoleDriver::init();
    ConsoleDriver::startTxThread();
    Processor::init();
    Processor::enableInterrupts();
    uint64* anum = (uint64*) mem_alloc(sizeof(uint64));
    uint64* bnum = (uint64*) mem_alloc(sizeof(uint64));
    *anum = 100;
    *bnum = 50;


    thread_t a, b;
    thread_create(&a, taskA, anum);
    thread_create(&b, taskB, bnum);
    for (int i = 0; i < 1000; i++) {
        thread_dispatch();
    }
    Processor::shutdown();
    return 0;
}*/



