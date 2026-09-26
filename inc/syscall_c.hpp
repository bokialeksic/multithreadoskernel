#pragma once

#include "../lib/hw.h"
#include "TCB.hpp"

size_t general_syscall(size_t op, size_t arg1, size_t arg2, size_t arg3,size_t arg4 = 0);

void* mem_alloc (size_t size);
int mem_free (void* ptr);

typedef TCB* thread_t;
int thread_create(thread_t* handle,void(*start_routine)(void*), void* arg);
int thread_exit();
void thread_dispatch();

class SCB;
typedef SCB* sem_t;

int sem_open(sem_t* handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);
int sem_wait_n(sem_t id, unsigned n);
int sem_signal_n(sem_t id, unsigned n);

int time_sleep(time_t ticks);

const int EOF = -1;

char getc();
void putc(char c);