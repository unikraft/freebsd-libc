#include <uk/tcb_impl.h>
#include <uk/alloc.h>
#include <uk/thread.h>

/* Stub implementations for FreeBSD libc Thread Control Block (TCB) */

int uk_thread_uktcb_init(struct uk_thread *thread, void *tcb) {
    /* Avoid unused parameter warnings */
    (void)thread;
    (void)tcb;
    return 0;  /* Success - minimal implementation */
}

void uk_thread_uktcb_fini(struct uk_thread *thread, void *tcb) {
    /* Avoid unused parameter warnings */
    (void)thread;
    (void)tcb;
    /* Nothing to do in minimal implementation */
}

void ukarch_tls_tcb_init(void *tcb) {
    /* Avoid unused parameter warning */
    (void)tcb;
    /* Nothing to do in minimal implementation */  
}