#ifndef INITIAL_H
#define INITIAL_H

#include "pandos_types.h"
#include "pandos_const.h"
#include "pcb.h"
#include "asl.h"
#include "exceptions.h"
#include "scheduler.h"
#include "interrupts.h"
#include <umps/libumps.h>
#include "klog.h"


#define CLOCKSEM semD[SEMNUM - 1] /* pseudo-clock semaphore */

extern int processCount;
extern int sftBlockCount;
extern pcb_t * currentProcess;
extern struct list_head lowReadyQueue;
extern struct list_head highReadyQueue;
extern pcb_t * newProcess;
extern int semD[SEMNUM];
extern cpu_t start_clock;

extern int main();

extern void test();
extern void uTLB_RefillHandler();

#endif