#include "../header/initial.h"

/* Dichiarazione variabili globali*/
int processCount;
int sftBlockCount;
pcb_t *currentProcess;
struct list_head lowReadyQueue;
struct list_head highReadyQueue;
pcb_t *newProcess;
int semD[SEMNUM];
cpu_t start_clock;

int main()
{
    /* Passup vector popolato*/
    passupvector_t *passupvector = (passupvector_t *)PASSUPVECTOR;
    passupvector->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    passupvector->tlb_refill_stackPtr = (memaddr)KERNELSTACK;
    passupvector->exception_handler = (memaddr)exceptionHandler;
    passupvector->exception_stackPtr = (memaddr)KERNELSTACK;

    /* Inizializzazione strutture dati*/
    initPcbs();
    initASL();

    /*Inizializzazione variabili nucleo*/
    processCount = 0;
    sftBlockCount = 0;
    INIT_LIST_HEAD(&lowReadyQueue);
    INIT_LIST_HEAD(&highReadyQueue);
    mkEmptyProcQ(&lowReadyQueue);
    mkEmptyProcQ(&highReadyQueue);
    currentProcess = NULL;

    for (int i = 0; i < SEMNUM; i++)
    {
        semD[i] = 0;
    }
    /* Impostazione Interval Timer*/
    LDIT(PSECOND);

    /* Inizializzazione nuovo processo:
        Interrupts e Timer attivati
        Funzione test puntata dal program counter
    */
    newProcess = allocPcb();
    newProcess->p_prio = 0;
    newProcess->p_s.status = (IEPON | IMON | TEBITON);
    RAMTOP(newProcess->p_s.reg_sp);
    newProcess->p_s.pc_epc = (memaddr)test;
    newProcess->p_s.reg_t9 = (memaddr)test;
    newProcess->p_parent = NULL;
    newProcess->p_time = 0;
    newProcess->p_semAdd = NULL;
    newProcess->p_supportStruct = NULL;
    /* Inserimento ready queue e incremento pc*/
    insertProcQ(&lowReadyQueue, newProcess);
    processCount += 1;
    /* Chiamata Scheduler*/
    scheduler();
    return 0;
}