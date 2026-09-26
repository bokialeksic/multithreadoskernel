.extern tcb_running_is_user_mode
.extern _ZN3TCB18thread_wrapper_cppEv

.align 4
.global _ZN3TCB20thread_wrapper_entryEv
.type _ZN3TCB20thread_wrapper_entryEv, @function


_ZN3TCB20thread_wrapper_entryEv:
    call tcb_running_is_user_mode
    beqz a0, .Lkernel_mode


    la   t0, _ZN3TCB18thread_wrapper_cppEv
    csrw sepc, t0

    csrr t1, sstatus
    li   t2, 0x100          # bit 8 = SPP
    not  t2, t2
    and  t1, t1, t2         # SPP = 0 (vraca se u user rezim)
    li   t2, 0x20           # bit 5 = SPIE
    or   t1, t1, t2         # SPIE = 1 (SIE postaje 1 odmah posle sret-a)
    csrw sstatus, t1

    sret


.Lkernel_mode:
    li   t0, 0x2
    csrs sstatus, t0
    tail _ZN3TCB18thread_wrapper_cppEv