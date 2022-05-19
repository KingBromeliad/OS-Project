#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "initial.h"
#include "interrupts.h"
#include "exceptions.h"
#include <umps/libumps.h>


extern int processCount;
extern int sftBlockCount;
extern pcb_t * currentProcess;
extern struct list_head lowReadyQueue;
extern struct list_head highReadyQueue;
extern pcb_t * newProcess;
extern int semD[SEMNUM];
extern cpu_t start_clock;

extern void scheduler();
/*Funzione per effettuare il loadstate */
extern void contextSwitch(pcb_t *process);

#endif
