#include "../h/syscall_c.hpp"
#include "../h/Console.hpp"
#include "../h/processor.hpp"
#include "../lib/console.h"

size_t general_syscall(size_t op, size_t arg1, size_t arg2, size_t arg3, size_t arg4){
    register size_t a0 __asm__("a0") = op;
    register size_t a1 __asm__("a1") = arg1;
    register size_t a2 __asm__("a2") = arg2;
    register size_t a3 __asm__("a3") = arg3;
    register size_t a4 __asm__("a4") = arg4;
    __asm__ volatile(
        "ecall"
        : "+r"(a0), "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4)
        :
        : "memory"
    );
    return a0;
}

void* mem_alloc (size_t size){
    size_t blocks = size / MEM_BLOCK_SIZE;
    if (size%MEM_BLOCK_SIZE != 0){
        blocks++;
    }
    return (void*) general_syscall(0x01, blocks, 0, 0);
}

int mem_free(void* ptr){
    return (int) general_syscall(0x02, (size_t)ptr, 0, 0);
}

int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    void* stack_space = mem_alloc(DEFAULT_STACK_SIZE);
    if (stack_space == nullptr) {
        return -1;
    }

    void* stack_top = (char*)stack_space + DEFAULT_STACK_SIZE;

    int result = (int)general_syscall(0x11, (long)start_routine, (long)arg, (long)handle, (long)stack_top);
    if (result != 0) {
        mem_free(stack_space);
    }
    return result;
}

int thread_exit() {
    return (int)general_syscall(0x12, 0, 0, 0);
}

void thread_dispatch() {
    general_syscall(0x13, 0, 0, 0);
}

int sem_open(sem_t* handle, unsigned init) {
    return (int) general_syscall(0x21, (size_t)handle, (size_t)init, 0);
}

int sem_close(sem_t handle) {
    return (int) general_syscall(0x22, (size_t)handle, 0, 0);
}

int sem_wait(sem_t id) {
    return (int) general_syscall(0x23, (size_t)id, 0, 0);
}

int sem_signal(sem_t id) {
    return (int) general_syscall(0x24, (size_t)id, 0, 0);
}

int sem_wait_n(sem_t id, unsigned n) {
    return (int) general_syscall(0x25, (size_t)id, (size_t)n, 0);
}

int sem_signal_n(sem_t id, unsigned n) {
    return (int) general_syscall(0x26, (size_t)id, (size_t)n, 0);
}

int time_sleep(time_t ticks) {
    return (int) general_syscall(0x31, (size_t)ticks, 0, 0);
}

char getc() {
    sem_wait(ConsoleDriver::rxFullSlots);
    char c = (char) general_syscall(0x41, 0, 0, 0);   // ABI radi citanje
    sem_signal(ConsoleDriver::rxEmptySlots);
    return c;
}

void putc(char c) {
    sem_wait(ConsoleDriver::txEmptySlots);
    general_syscall(0x42, (size_t)c, 0, 0);
}