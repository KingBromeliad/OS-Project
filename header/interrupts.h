#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "initial.h"
#include "exceptions.h"
#include "scheduler.h"
#include "initProc.h"
#include "scheduler.h"
#include <umps/libumps.h>

extern int processCount;
extern int sftBlockCount;
extern pcb_t *currentProcess;
extern struct list_head lowReadyQueue;
extern struct list_head highReadyQueue;
extern pcb_t *newProcess;
extern int semD[SEMNUM];
extern cpu_t start_clock;

extern void interruptsHandler(state_t *oldState);
extern void handleDeviceInterrupt(int device);
extern int whichDevice(unsigned int map);

#endif