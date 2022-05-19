#include "../header/interrupts.h"

state_t *oldState;
void interruptsHandler(state_t *oldState)
{
    /* Gestione tempo trascorso negli interrupt*/
    STCK(stop_clock);

    /* Gestione interrupts sollevati da Processor Local Timer */
    if ((oldState->cause & LOCALTIMERINT) != 0)
    {
        /* Acknowledge dell'interrupt */
        setTIMER(TIMESLICE);
        if (currentProcess != NULL)
        {
            CopyPaste(oldState, &(currentProcess->p_s));
            currentProcess->p_time = (currentProcess->p_time) + (stop_clock - start_clock);
            /* Processi sottoposti a preemption */
            if (currentProcess->p_prio == 0)
            {
                /* Salvo lo stato del processore all'interno del processo*/
                /* Inserisco il processo 'scaduto' nella ready queue*/
                if (currentProcess->p_prio == PROCESS_PRIO_LOW)
                {
                    insertProcQ(&lowReadyQueue, currentProcess);
                }
                else
                {
                    insertProcQ(&highReadyQueue, currentProcess);
                }
                /* Chiamo lo scheduler per cambiare processo */
                scheduler();
            }
            LDST(oldState);
        }
        else
        {
            scheduler();
        }
    }
    /* Gestione interrupts sollevati da Interval Timer */
    if ((oldState->cause & TIMERINTERRUPT) != 0)
    {

        /* Acknowledge dell'interrupt */
        LDIT(PSECOND);

        /* Sblocca tutti i pcb bloccati sul semaforo controllato dallo pseudo clock */
        pcb_t *tempProcess;
        tempProcess = removeBlocked(&CLOCKSEM);
        while (tempProcess != NULL)
        {
            tempProcess->p_semAdd = NULL;
            if (tempProcess->p_prio == PROCESS_PRIO_LOW)
            {
                insertProcQ(&lowReadyQueue, tempProcess);
            }
            else
            {
                insertProcQ(&highReadyQueue, tempProcess);
            }
            tempProcess = removeBlocked(&CLOCKSEM);
            /* Aggiorno il contatore */
            sftBlockCount -= 1;
        }

        /* Reset a 0 del semaforo controllato dallo pseudo clock */
        CLOCKSEM = 0;

        /* Ritorno il controllo al processo bloccato */
        if (currentProcess != NULL)
        {
            LDST(oldState);
        }
        else
        {
            scheduler();
        }
    }
    /* Gestione interrupts sollevati da Disk device */
    if ((oldState->cause & DISKINTERRUPT) != 0)
    {
        handleDeviceInterrupt(DISKINT);
    }
    /* Gestione interrupts sollevati da Flash device */
    if ((oldState->cause & FLASHINTERRUPT) != 0)
    {
        handleDeviceInterrupt(FLASHINT);
    }
    /* Gestione interrupts sollevati da Printer device */
    if ((oldState->cause & PRINTINTERRUPT) != 0)
    {
        handleDeviceInterrupt(PRNTINT);
    }
    /* Interrupt schede di rete non implementato */

    /* Gestione interrupts sollevati da Terminali */
    if ((oldState->cause & TERMINTERRUPT) != 0)
    {
        int deviceSemaphore;
        pcb_t *unblockedProcess;
        unsigned int status;
        unsigned int bitMap;
        volatile int deviceNumber;
        volatile devregarea_t *deviceRegister;

        /* Bitmap del dispositivo ottenuta dal device register */
        deviceRegister = ((devregarea_t *)RAMBASEADDR);
        bitMap = deviceRegister->interrupt_dev[TERMINT - 3];
        /* Calcolo l'indirizzo del device */
        deviceNumber = whichDevice(bitMap);
        /* Calcolo l'indirizzo del semaforo associato */
        deviceSemaphore = ((TERMINT - 3) * DEVPERINT) + deviceNumber;

        /* Priorità alla scrittura */
        if ((deviceRegister->devreg[4][deviceNumber].term.transm_status & 0x0F) != READY)
        { /* Caso scrittura */
            status = deviceRegister->devreg[4][deviceNumber].term.transm_status;
            deviceRegister->devreg[4][deviceNumber].term.transm_command = ACK;
        }
        else
        { /* Caso lettura */
            status = deviceRegister->devreg[5][deviceNumber].term.recv_status;
            deviceRegister->devreg[5][deviceNumber].term.recv_command = ACK;
            /* Non implementata in questa fase */
            deviceSemaphore = deviceSemaphore + DEVPERINT;
            PANIC();
        }
        /* Performo una v operation sul semaforo */
        unblockedProcess = removeBlocked(&(semD[deviceSemaphore]));
        /* (IP) Controllo che la V ritorni effettivamente un pcb */
        if (unblockedProcess != NULL)
        {
            /* Inserisco lo stato salvato nel pcb sbloccato*/
            unblockedProcess->p_s.reg_v0 = status;
            unblockedProcess->p_semAdd = NULL;
            /* Inserisco il processo ready queue */
            if (unblockedProcess->p_prio == PROCESS_PRIO_LOW)
            {
                insertProcQ(&lowReadyQueue, unblockedProcess);
            }
            else
            {
                insertProcQ(&highReadyQueue, unblockedProcess);
            }
            /* Aggiorno il contatore */
            sftBlockCount -= 1;
        }
        /* Ritorno il controllo al processo bloccato */
        if (currentProcess != NULL)
        {
            LDST(oldState);
        }
        else
        {
            /* (IP) Nessun processo a cui ritornare controllo */
            scheduler();
        }
    }
}

/* Funzione per la gestione di interrupt sollevati dai dispositivi non terminali*/
void handleDeviceInterrupt(int device)
{
    int deviceSemaphore;
    volatile int deviceNumber;
    unsigned int status;
    unsigned int bitMap;
    volatile devregarea_t *deviceRegister;
    pcb_t *unblockedProcess;
    /* Calcolo indirizzi device come da manuale */
    deviceRegister = ((devregarea_t *)RAMBASEADDR);
    bitMap = deviceRegister->interrupt_dev[device - 3];
    /* Calcolo l'indirizzo del device */
    deviceNumber = whichDevice(bitMap);
    /* Calcolo l'indirizzo del semaforo associato */
    device = (device - 3) * DEVPERINT;
    deviceSemaphore = device + deviceNumber;
    /* Salvo il codice di stato dal device register*/
    status = ((deviceRegister->devreg[device][deviceNumber]).dtp.status);
    /* Acknowledge dell'interrupt */
    (deviceRegister->devreg[device][deviceNumber]).dtp.command = ACK;
    /* Performo una v operation sul semaforo */
    unblockedProcess = removeBlocked(&(semD[deviceSemaphore]));
    /* (IP) Controllo che la V ritorni effettivamente un pcb */
    if (unblockedProcess != NULL)
    {
        /* Inserisco lo stato salvato nel pcb sbloccato*/
        unblockedProcess->p_s.reg_v0 = status;
        /* Inserisco il processo ready queue */
        if (unblockedProcess->p_prio == 0)
        {
            insertProcQ(&lowReadyQueue, unblockedProcess);
        }
        else
        {
            insertProcQ(&highReadyQueue, unblockedProcess);
        }
        /* Aggiorno il contatore */
        sftBlockCount -= 1;
    }
    /* Ritorno il controllo al processo bloccato */
    if (currentProcess != NULL)
    {
        LDST(oldState);
    }
    else
    {
        /* (IP) Nessun processo a cui ritornare controllo */
        scheduler();
    }
}

/* Funzione per ottenere la linea del device */
int whichDevice(unsigned int map)
{
    if ((map & DEV0ON) != 0)
    {
        return 0;
    }
    else if ((map & DEV1ON) != 0)
    {
        return 1;
    }
    else if ((map & DEV2ON) != 0)
    {
        return 2;
    }
    else if ((map & DEV3ON) != 0)
    {
        return 3;
    }
    else if ((map & DEV4ON) != 0)
    {
        return 4;
    }
    else if ((map & DEV5ON) != 0)
    {
        return 5;
    }
    else if ((map & DEV6ON) != 0)
    {
        return 6;
    }
    else
    {
        return 7;
    }
};