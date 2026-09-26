#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "list.hpp"

class TCB; // Preddeklaracija da ne bismo kruzno inkludovali tcb.hpp

class Scheduler {
private:
    static List<TCB> readyQueue; // Statika: deljena lista za ceo sistem

public:
    static void put(TCB* tcb);
    static TCB* get();
};

#endif