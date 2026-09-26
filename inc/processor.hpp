#pragma once
#include "../lib/hw.h"
class Processor
{
public:
    // Inicijalizuje stvec (adresu prekidne rutine). Poziva se jednom, iz
    // main(), pre nego sto se omoguce prekidi.
    static void init();

    // Omogucava softverske i spoljasnje (hardverske) prekide i postavlja
    // SIE bit. Poziva se iz main() nakon sto je sve potrebno (konzola,
    // niti...) inicijalizovano.
    static void enableInterrupts();

    static void shutdown();

private:

    static uint64 r_sstatus();
    static uint64 r_sepc();
    static uint64 r_scause();
    static uint64 r_stval();
    static uint64 r_sip();
    static uint64 r_sie();

    static void w_sstatus(uint64 sstatus);
    static void w_stvec(uint64 stvec);
    static void w_sepc(uint64 sepc);
    static void w_sip(uint64 sip);
    static void w_sie(uint64 sie);

    static void supervisorTrap();
    static void handle_trap();

};


inline uint64 Processor::r_sstatus() {
    uint64 volatile sstatus;
    __asm__ volatile("csrr %[sstatus], sstatus":[sstatus]"=r"(sstatus));
    return sstatus;
}

inline uint64 Processor::r_scause() {
    uint64 volatile scause;
    __asm__ volatile("csrr %[scause], scause":[scause]"=r"(scause));
    return scause;
}
inline uint64 Processor::r_stval() {
    uint64 volatile stval;
    __asm__ volatile("csrr %[stval], stval":[stval]"=r"(stval));
    return stval;
}
inline uint64 Processor::r_sip() {
    uint64 volatile sip;
    __asm__ volatile("csrr %[sip], sip":[sip]"=r"(sip));
    return sip;
}
inline void Processor::w_sip(uint64 sip) {
    __asm__ volatile("csrw sip,%[sip]"::[sip]"r"(sip));
}
inline uint64 Processor::r_sie() {
    uint64 volatile sie;
    __asm__ volatile("csrr %[sie], sie":[sie]"=r"(sie));
    return sie;
}
inline void Processor::w_sie(uint64 sie) {
    __asm__ volatile("csrw sie,%[sie]"::[sie]"r"(sie));
}
inline void Processor::w_stvec(uint64 stvec)
{
    __asm__ volatile("csrw stvec,%[stvec]"::[stvec]"r"(stvec));
}
inline uint64 Processor::r_sepc() {
    uint64 volatile sepc;
    __asm__ volatile("csrr %[sepc], sepc":[sepc]"=r"(sepc));
    return sepc;
}
inline void Processor::w_sepc(uint64 sepc) {
    __asm__ volatile("csrw sepc,%[sepc]"::[sepc]"r"(sepc));
}
inline void Processor::w_sstatus(uint64 sstatus) {
    __asm__ volatile("csrw sstatus,%[sstatus]"::[sstatus]"r"(sstatus));
}
inline void Processor::shutdown() {
    volatile uint32* shutdown_addr = (volatile uint32*)0x100000;
    *shutdown_addr = 0x5555;
    while (1);
}

inline void Processor::init() {
    w_stvec((uint64)&Processor::supervisorTrap);
}

inline void Processor::enableInterrupts() {
    w_sie((1ULL << 1) | (1ULL << 9)); // SSIE | SEIE
    w_sstatus(r_sstatus() | (1ULL << 1)); // SIE bit
}