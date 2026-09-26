#include "../h/processor.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/TCB.hpp"
#include "../h/SCB.hpp"
#include "../h/Console.hpp"
#include "../lib/console.h"
#include "../test/printing.hpp"



void Processor::handle_trap(){
    unsigned long arg1;
    unsigned long arg2;
    unsigned long arg3;
    unsigned long arg4;

    __asm__ volatile("mv %0, a1" : "=r"(arg1));
    __asm__ volatile("mv %0, a2" : "=r"(arg2));
    __asm__ volatile("mv %0, a3" : "=r"(arg3));
    __asm__ volatile("mv %0, a4" : "=r"(arg4));

    uint64 volatile sepc;
    sepc = r_sepc();
    uint64 volatile scause;
    scause = r_scause();
    uint64 volatile sstatus;
    sstatus = r_sstatus();


    if (scause == 8 || scause == 9){
        
        sepc = sepc + 4;
        unsigned long id;
        __asm__ volatile("mv %0, a0" : "=r"(id));

        unsigned long return_val = -1;
        switch (id){
            case 0x01: {
                void* allocated_ptr = MemoryAllocator::mem_alloc(arg1);
                return_val = (unsigned long) allocated_ptr;
                break;
            }
            case 0x02:{
                int status = MemoryAllocator::mem_free((void*)arg1);
                return_val = (unsigned long) status;
                break;
            }
            case 0x11: { // thread_create
                    TCB* new_thread = TCB::createThread((void(*)(void*))arg1, (void*)arg2, (void*)arg4);
                    if (new_thread != nullptr) {
                        *(TCB**)arg3 = new_thread;
                        return_val = 0;
                    } else {
                        return_val = -1;
                    }
                    break;
            }
            case 0x12: { // thread_exit
                    TCB::exit();
                    return_val = 0;
                    break;
            }
            case 0x13: { // thread_dispatch
                    TCB::dispatch();
                    return_val = 0;
                    break;
            }
            case 0x21: { // sem_open(sem_t* handle, unsigned init)
                    SCB* new_sem = SCB::createSemaphore((unsigned)arg2);
                    if (new_sem != nullptr) {
                        *(SCB**)arg1 = new_sem;
                        return_val = 0;
                    } else {
                        return_val = -1;
                    }
                    break;
            }
            case 0x22: { // sem_close(sem_t handle)
                    SCB* sem = (SCB*)arg1;
                    if (sem != nullptr) {
                        return_val = (unsigned long)(long) SCB::closeSemaphore(sem);
                    } else {
                        return_val = (unsigned long)(long) -1;
                    }
                    break;
            }
            case 0x23: { // sem_wait(sem_t id)
                    SCB* sem = (SCB*)arg1;
                    if (sem != nullptr) {
                        return_val = (unsigned long)(long) sem->wait();
                    } else {
                        return_val = (unsigned long)(long) -1;
                    }
                    break;
            }
            case 0x24: { // sem_signal(sem_t id)
                    SCB* sem = (SCB*)arg1;
                    if (sem != nullptr) {
                        return_val = (unsigned long)(long) sem->signal();
                    } else {
                        return_val = (unsigned long)(long) -1;
                    }
                    break;
            }
            case 0x25: { // sem_wait_n(sem_t id, unsigned n)
                    SCB* sem = (SCB*)arg1;
                    if (sem != nullptr) {
                        return_val = (unsigned long)(long) sem->wait((unsigned)arg2);
                    } else {
                        return_val = (unsigned long)(long) -1;
                    }
                    break;
            }
            case 0x26: { // sem_signal_n(sem_t id, unsigned n)
                    SCB* sem = (SCB*)arg1;
                    if (sem != nullptr) {
                        return_val = (unsigned long)(long) sem->signal((unsigned)arg2);
                    } else {
                        return_val = (unsigned long)(long) -1;
                    }
                    break;
            }
            case 0x31: { // time_sleep(time_t ticks)
                    TCB::sleep((time_t)arg1);
                    return_val = 0;
                    break;
            }
            case 0x41: { // getc()
                    return_val = (unsigned long)(long) ConsoleDriver::readFromRxBuffer();
                    break;
            }
            case 0x42: { // putc(char)
                    ConsoleDriver::writeToTxBuffer((char)arg1);
                    return_val = 0;
                    break;
            }
            default:{
                    break;
            }
        }

        __asm__ volatile ("mv a0, %0" : : "r"(return_val));
    }
    else if (scause == 0x8000000000000001ULL){
        // prekid od tajmera

        TCB::tick();

        if (TCB::running->getRemainingSlice() > 0) {
            TCB::running->setRemainingSlice(TCB::running->getRemainingSlice() - 1);
        }
        if (TCB::running->getRemainingSlice() == 0) {
            TCB::dispatch();
        }

        uint64 sip = r_sip();
        sip &= ~(uint64)(1ULL << 1);
        w_sip(sip);

    }
    else if (scause == 0x8000000000000009ULL){
        // prekid od konzole

        int irq = plic_claim();

        if (irq == (int)CONSOLE_IRQ) {
            ConsoleDriver::handleInterrupt();
        }
        plic_complete(irq);

    }
    else
    {
        printString("Katastrofa ");
        while (ConsoleDriver::pendingCount() > 0) {
            thread_dispatch();
        }
        Processor::shutdown();
    }

    w_sstatus(sstatus);
    w_sepc(sepc);
}



