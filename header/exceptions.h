#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "initial.h"
#include "interrupts.h"
#include "scheduler.h"
#include <umps/libumps.h>
#include <umps/cp0.h>
#include <umps/arch.h>

/* Prendo le variabili da initial */
extern int processCount;
extern int sftBlockCount;
extern pcb_t *currentProcess;
extern struct list_head lowReadyQueue;
extern struct list_head highReadyQueue;
extern pcb_t *newProcess;
extern int semD[SEMNUM];
extern cpu_t start_clock;

void uTLB_RefillHandler();

void exceptionHandler();
void TLBHandler();
void programTrapHandler();
void SYSCALLHandler();

int CreateProcess(state_t *statep, int prio, support_t *supportp);
void TerminateProcess(int pid);
void Passeren(int *semaddr);
void Verhogen(int *semaddr);
void doIO(int *cmdAddr, int cmdValue);
int getTime();
void clockWait();
support_t *getSupportPTR();
int getProcessID(int parent);
void yield();

void PassUpOrDie(int Excepttrigger);

void TerminateSingleProcess(pcb_t* to_terminate);
void TerminateTree(pcb_t* to_terminate);

void block(int *);       /* funzione ausialiaria per bloccare i processi */
void CopyPaste(state_t *copy, state_t *paste);
device_data findDevice(memaddr addr);

#endif