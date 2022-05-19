#include "../header/scheduler.h"


void scheduler()
{
    /*Gestione processi alta priorità*/
    pcb_t *incomingHiProcess;
    incomingHiProcess = removeProcQ(&highReadyQueue);
    if (incomingHiProcess != NULL)
    {
        /* Caricamento di start_clock */
        STCK(start_clock);
        contextSwitch(incomingHiProcess);
    }
    else
    {
        /*Gestione processi a bassa priorità*/
        pcb_t *incomingProcess;
        incomingProcess = removeProcQ(&lowReadyQueue);
        if (incomingProcess != NULL)
        {
            /* PLT timer impostato con timeslice di 5ms */
            STCK(start_clock);
            setTIMER(TIMESLICE);
            contextSwitch(incomingProcess);
        }
        /*Controllo l'eventuale presenza di processi da eseguire*/
        if (processCount == 0)
        {
            /*Processi terminati*/
            HALT();
        }
        else
        {
            if (processCount > 0)
            {
                if (sftBlockCount > 0)
                {
                    /* Attesa di processi bloccati*/                    
                    /* Attendo con interrupt abilitati */
                    unsigned int sendingStatus = ( IECON | IMON );
                    /* Disabilito il timer caricando un valore elevato */
                    setTIMER(MAXTIME);
                    setSTATUS(sendingStatus);
                    WAIT();
                }
                /*Caso di Errore */
                if (sftBlockCount == 0)
                {
                    PANIC();
                }
            }
        }
    }
}

/* Funzione di context switch */
void contextSwitch(pcb_t *process)
{
    currentProcess = process;
    LDST(&(process->p_s));
}