#include "../header/exceptions.h"

state_t *old_state; /* variabile per la BIOSDATAPAGE */
int pid = 1; /* variabile per l'assegnazione dei pid */

void exceptionHandler()
{
    unsigned int exceptionCode; /*  exception_code: 0 : Interrupts; 1-3 : TLB exceptions; 4-7 : Program Traps; 8 : SYSCALL; 9-12: Program Traps */

    old_state = (state_t *)BIOSDATAPAGE;
    /* Prendo l'excepion code */
    exceptionCode = CAUSE_GET_EXCCODE(old_state->cause);
    if (exceptionCode == 0)
    {
        interruptsHandler(old_state);
    }
    else if (exceptionCode >= 1 && exceptionCode <= 3)
    {
        TLBHandler();
    }
    else if (((exceptionCode >= 4) && (exceptionCode <= 7)) || ((exceptionCode >= 9) && (exceptionCode <= 12)))
    {
        programTrapHandler();
    }
    else if (exceptionCode == 8)
    {
        SYSCALLHandler();
    }
}

void SYSCALLHandler()
{
    old_state->pc_epc += WORDLEN; /* Devo far avanzare il Program Counter */
    int syscallNumber, a1, a2, a3, usermode;
    syscallNumber = old_state->reg_a0;
    a1 = old_state->reg_a1;
    a2 = old_state->reg_a2;
    a3 = old_state->reg_a3;
    /* usermode 0 = kernel, 1 = user */
    usermode = currentProcess->p_s.status & USERPON; /* Prendo lo usermode */
    /* Per fare le syscall devo essere in kernel mode */
    if (usermode == 0)
    {
        switch (syscallNumber)
        {
        case CREATEPROCESS:
            old_state->reg_v0 = CreateProcess((state_t *)a1, a2, (support_t *)a3);
            LDST(old_state);
            break;
        case TERMPROCESS:
            TerminateProcess(a1);
            break;
        case PASSEREN:
            Passeren((int *)a1);
            break;
        case VERHOGEN:
            Verhogen((int *)a1);
            break;
        case DOIO:
            doIO((int *)a1, a2);
            break;
        case GETTIME:
            old_state->reg_v0 = getTime();
            LDST(old_state);
            break;
        case CLOCKWAIT:
            clockWait();
            break;
        case GETSUPPORTPTR:
            old_state->reg_v0 = (int)getSupportPTR();
            LDST(old_state);
            break;
        case GETPROCESSID:
            old_state->reg_v0 = getProcessID(a1);
            LDST(old_state);
            break;
        case YIELD:
            yield();
            break;
        default: /* Se dentro reg_a0 ho il valore di una syscall non esistente, chiamo il program trap handler */
            programTrapHandler();
            break;
        }
    }
    else
    {
        int old_cause = getCAUSE() & !CAUSE_EXCCODE_MASK; /* Se viene chiamata una syscall in user mode imposto RI (Reserved Instrucion) sul registro cause */
        setCAUSE(old_cause | 40);
        programTrapHandler(); /* e chiamo il program trap handler */
    }
    scheduler();
}
/* Crea un nuovo processo e ritorna il suo */
int CreateProcess(state_t *statep, int prio, support_t *supportp)
{
    pcb_t *newChild = allocPcb();
    int ret = -1; /* se fallisce ritorna -1 se no ritorna l'id del processo creato */
    if (newChild != NULL)
    { /* Inizializzo i campi del nuovo processo */
        CopyPaste(statep, &(newChild->p_s));
        newChild->p_prio = prio;
        newChild->p_supportStruct = supportp;
        newChild->p_pid = pid++; /* id assegnato come valore intero che viene incrementato ogni volta che viene creato un processo */
        if (prio == PROCESS_PRIO_LOW) 
        {
            insertProcQ(&lowReadyQueue, newChild);
        }
        else
        {
            insertProcQ(&highReadyQueue, newChild);
        }
        newChild->p_time = 0;
        newChild->p_semAdd = NULL;
        ret = newChild->p_pid;
        insertChild(currentProcess, newChild);
        processCount += 1; /* Incremento il contatore dei processi */
    }
    return ret;
}
/* Uccide un processo e i suoi figli */
void TerminateProcess(int pid)
{
    int tpid;
    if (pid == 0)
    { /* Con 0 in reg_a1 si uccide il processo corrente */
        outChild(currentProcess);      
        TerminateTree(currentProcess); 
        currentProcess = NULL;
        scheduler();
    }
    else
    { /* Con reg_a1 != 0 si uccide il processo indicato dal valore di reg_a1 */
        tpid = pid;
        outChild((pcb_t *)tpid); 
        TerminateTree((pcb_t *)tpid);
        LDST(old_state);
    }
}
/* Esegue una P sul semaforo binario */
void Passeren(int *semaddr){
    pcb_t *tmp = NULL;
    if(*semaddr == 0){
        block(semaddr);
    }else if((tmp = removeBlocked(semaddr)) != NULL){     
        tmp->p_semAdd = NULL;
        if (tmp->p_prio == PROCESS_PRIO_LOW)
        {
            insertProcQ(&lowReadyQueue, tmp);
        }
        else
        {
            insertProcQ(&highReadyQueue, tmp);
        }
        if((semaddr >= (&semD[0])) && (semaddr <= (&semD[SEMNUM - 1]))){
            sftBlockCount -= 1;
        }
    }
    else{
        *semaddr-= 1;
    }
    LDST(old_state);
} 

/* esegue una V sul semaforo binario */
void Verhogen(int *semaddr){
    pcb_t *tmp = NULL;
    if(*semaddr == 1){
        block(semaddr);
    }else if((tmp = removeBlocked(semaddr)) != NULL){
        tmp->p_semAdd = NULL;
        if (tmp->p_prio == PROCESS_PRIO_LOW)
        {
            insertProcQ(&lowReadyQueue, tmp);
        }
        else
        {
            insertProcQ(&highReadyQueue, tmp);
        }
        if((semaddr >= (&semD[0])) && (semaddr <= (&semD[SEMNUM - 1]))){
            sftBlockCount -= 1;
        }
    }else{
        *semaddr+= 1;
    }
    LDST(old_state);
}
/* Svolge un operazione di I/O, dentro interrupts.c ritorna lo stato del dispositivo */
void doIO(int *cmdAddr, int cmdValue) /* inidizzo di memoria campo command del device, valore */
{

    int addr = (int)cmdAddr;  
    int value = cmdValue; 

    device_data device = findDevice(addr);

    /* Scrivo il valore in command per inziare I/O */
    volatile devregarea_t *deviceRegister;
    deviceRegister = ((devregarea_t *)RAMBASEADDR);
    deviceRegister->devreg[device.d_type][device.d_number].term.transm_command = value;
    Passeren(&semD[device.d_semaphore]);
    CopyPaste(old_state, &currentProcess->p_s);
    scheduler();
}
/* Ritorna il tempo che il procceso ha usato il processore */
cpu_t getTime()
{
    cpu_t now;
    STCK(now);
    now = (now - start_clock) + currentProcess->p_time;
    return now;
}
/* Fa una P sul semaforo dello pseudo-clock */
void clockWait()
{
    Passeren(&CLOCKSEM);
}
/* Ritorna la struttura di supporto del processo corrente */
support_t *getSupportPTR()
{

    return currentProcess->p_supportStruct;
}
/* Ritorna il pid del processo indicato in reg_a1 */
int getProcessID(int parent)
{
    int ret = currentProcess->p_pid; /* Se è 0 ritorna il pid del processo corrente */
    if (parent != 0)
    {
        ret = currentProcess->p_parent->p_pid;
    }
    return ret;
}
/* Mette il processo corrente in fondo alla sua readyQueue */
void yield()
{
    CopyPaste(old_state, &(currentProcess->p_s));
    if (currentProcess->p_prio == PROCESS_PRIO_LOW)
    {
        insertProcQ(&lowReadyQueue, currentProcess);
    }
    else
    {
        insertProcQ(&highReadyQueue, currentProcess);
    }
    scheduler();
}

/* Blocca il processo e chiama lo scheduler */
void block(int *blocked)
{
    if((blocked >= (&semD[0])) && (blocked <= (&semD[SEMNUM - 1]))){
        sftBlockCount += 1;
    }
    CopyPaste(old_state, &(currentProcess->p_s));
    cpu_t stopt;
    STCK(stopt);
    currentProcess->p_time = currentProcess->p_time + (stopt - start_clock);
    currentProcess->p_semAdd = blocked;
    insertBlocked(blocked, currentProcess);
    currentProcess = NULL;
    scheduler();
}

void TLBHandler()
{
    PassUpOrDie(PGFAULTEXCEPT);
}

void programTrapHandler()
{
    PassUpOrDie(GENERALEXCEPT);
}
/* Si occupa di gestire le eccezzioni non gestite dalle syscall */
void PassUpOrDie(int Excepttrigger)
{
    if (currentProcess->p_supportStruct == NULL) /* Se il processo non ha una struttura di supporto viene ucciso */
    {
        TerminateProcess(0);
    }
    else /* Se ha una struttura di supporto si occupa di caricare il suo contesto */
    {
        CopyPaste(old_state, &(currentProcess->p_supportStruct->sup_exceptState[Excepttrigger]));
        LDCXT(currentProcess->p_supportStruct->sup_exceptContext[Excepttrigger].stackPtr,
              currentProcess->p_supportStruct->sup_exceptContext[Excepttrigger].status,
              currentProcess->p_supportStruct->sup_exceptContext[Excepttrigger].pc);
    }
}
/* Funzione ausiliaria per copiare tutto stato di un processo */
void CopyPaste(state_t *copy, state_t *paste)
{
    paste->entry_hi = copy->entry_hi;
    paste->cause = copy->cause;
    paste->status = copy->status;
    paste->pc_epc = copy->pc_epc;
    for (int i = 0; i < STATE_GPR_LEN; i++)
    {
        paste->gpr[i] = copy->gpr[i];
    }
    paste->hi = copy->hi;
    paste->lo = copy->lo;
}
/* Funzione ausiliaria per calcolare il dispostivo giusto */
device_data findDevice(memaddr addr)
{
    device_data device;
    if (addr < DEV_REG_ADDR(3, 7))
    {
        device.d_type = 0; /* disk */
    }
    else if (addr < DEV_REG_ADDR(4, 7))
    {
        device.d_type = 1; /* flash */
    }
    else if (addr < DEV_REG_ADDR(5, 7))
    {
        device.d_type = 2; /* network */
    }
    else if (addr < DEV_REG_ADDR(6, 7))
    {
        device.d_type = 3; /* printer */
    }
    else if (addr < DEV_REG_ADDR(7, 7))
    {
        device.d_type = 4; /* terminal transmitter */
    }
    else if (addr < DEV_REG_ADDR(8, 7))
    {
        device.d_type = 5; /* terminal receiver */
    }

    int found = 0;
    device.d_number = 0;
    while (!found && device.d_number < 8)
    {
        if (addr == (DEV_REG_ADDR(device.d_type + 3, device.d_number) + 0xc))
        {
            found = 1;
        }
        else
        {
            device.d_number++;
        }
    }
    device.d_semaphore = device.d_type * N_DEV_PER_IL + device.d_number;
    return device;
}
/* Funzione ausiliaria per selezionare ricorsivamente i figli da uccidere */
void TerminateTree(pcb_t *to_terminate)
{
    while (!(emptyChild(to_terminate)))
    {
        TerminateTree(removeChild(to_terminate));
    }
    TerminateSingleProcess(to_terminate);
}
/* Funzione ausiliaria per uccidere i processi */
void TerminateSingleProcess(pcb_t *to_terminate)
{
    processCount -= 1;
    if (to_terminate->p_prio == 0) /* Tolgo il processo dalla sua coda */
    {
        outProcQ(&lowReadyQueue, to_terminate);
    }
    else
    {
        outProcQ(&highReadyQueue, to_terminate);
    }
    if (to_terminate->p_semAdd != NULL)
    { /* Aggiorno il sftBlockCount per i semafori che erano bloccati su un dispositivo */
        while(outBlocked(to_terminate) != NULL){
            if((to_terminate->p_semAdd >= &semD[0]) && (to_terminate->p_semAdd <= &semD[SEMNUM - 1])){
            sftBlockCount -= 1;
            }
        }
    }
    freePcb(to_terminate); /* Libero il pcb */
}