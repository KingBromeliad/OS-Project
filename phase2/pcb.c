#include "../header/pcb.h"
#include "../header/pandos_const.h"
#include "../header/listx.h"

/* ------------------------------------------------------------------------------------------------

 -------------------------------------------------------------------------------------------------*/
/* __Allocazione dei Pcb__ */

/*
•pcbFree: lista dei PCB che sono liberi o inutilizzati.
•pcbfree_h: elemento sentinella della lista pcbFree.
•pcbFree_table[MAX_PROC]: array di PCB con dimensione massima di MAX_PROC.
*/

/* Macro da 'listx.h' che dichiara e inizializza una nuova lista.
   LIST_HEAD(name) name: nome che si vuole dare alla variabile lista */

HIDDEN LIST_HEAD(pcbFree_h);
HIDDEN struct pcb_t pcbFree_table[MAXPROC];

/* Inizializza la pcbFree in modo da contenere tutti gli elementi della pcbFree_table. Questo metodo deve essere chiamato una volta sola in fase di inizializzazione della struttura dati.*/
void initPcbs(void)
{
    for (int i = 0; i < MAXPROC; i++)
    {
        list_add_tail(&pcbFree_table[i].p_list, &pcbFree_h);
    }
}

/* Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree)*/
void freePcb(pcb_t *p)
{
    if (p != NULL)
        list_add_tail(&p->p_list, &pcbFree_h);
}

/* -- Implementazione di memset rudimentale; la funzione viene utilizzata per impostare a 0 (conseguentemente NULL) i campi della struttura pcb_t -- */
HIDDEN void *_memset(void *s, int c, int n)
{

    unsigned int index;
    unsigned char *memory = s, value = c;

    for (index = 0; index < n; index++)
        memory[index] = value;

    return (memory);
}

/* Restituisce NULL se la pcbFree è vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void)
{
    if (!list_empty(&pcbFree_h))
    {

        struct pcb_t *free_pcb;

        free_pcb = container_of(pcbFree_h.next, struct pcb_t, p_list);

        list_del(pcbFree_h.next);
        /* chiamata di _memset per impostare a 0 campi di free_pcb */
        _memset(free_pcb, 0, sizeof(pcb_t));

        /* chiamata di _memset per inizializzare i campi di free_pcb */
        INIT_LIST_HEAD(&(free_pcb->p_list));
        INIT_LIST_HEAD(&(free_pcb->p_child));
        INIT_LIST_HEAD(&(free_pcb->p_sib));

        return free_pcb;
    }
    else
        return NULL;
}

/* __Gestione Lista dei Pcb__ */

/* Inizializza la lista dei PCB, inizializzando l’elemento sentinella.*/
void mkEmptyProcQ(struct list_head *head)
{

    INIT_LIST_HEAD(head);
}

/* Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.*/
int emptyProcQ(struct list_head *head)
{
    return head->next == head;
}

/* inserisce l’elemento puntato da p nella coda dei processi puntata da head.*/
void insertProcQ(struct list_head *head, pcb_t *p)
{
    if (head != NULL && p != NULL)
    {

        list_add_tail(&(p->p_list), head);
    }
    else
        return;
}

/* Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO. Ritorna NULL se la coda non ha elementi.*/
pcb_t *headProcQ(struct list_head *head)
{
    if (head != NULL && !emptyProcQ(head))
    {

        return container_of(head->next, struct pcb_t, p_list);
    }
    else
        return NULL;
}

/* Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda è vuota. Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.*/
struct pcb_t *removeProcQ(struct list_head *head)
{
    if (head != NULL && !emptyProcQ(head))
    {

        struct pcb_t *first_Proc;
        first_Proc = container_of(head->next, struct pcb_t, p_list);
        list_del(head->next);

        return first_Proc;
    }
    else
        return NULL;
}

/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non è presente nella coda, restituisce NULL. */
pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    if (head != NULL && p != NULL)
    {

        /* variabili ausiliarie per iterazione lista*/
        struct list_head *next_P;
        struct pcb_t *temp;

        next_P = head->next;

        /* ricerca eventuale posizione di p */
        while ((next_P != head) && (container_of(next_P, struct pcb_t, p_list) != p))
            next_P = next_P->next;

        temp = container_of(next_P, struct pcb_t, p_list);

        /* verifica che la condizione di uscita dal ciclo sia effettivamente temp == p */
        if (temp == p)
        {

            list_del(&(temp->p_list));
            return temp;

            /* caso p non presente*/
        }
        else
            return NULL;
    }
    else
        return NULL;
}

/* __Gestione Alberi dei Pcb__ */

// Controllo se esiste il padre e se la lista è inizializzata, se non lo è ritorno true altrimenti false
int emptyChild(pcb_t *this)
{
    if (this != NULL)
    {
        return list_empty(&this->p_child);
    }
    else
        return 0;
}
// Controllo se esistono padre e figlio,se è la prima volta che inserisco un figlio inizializzo la lista e lo inserisco, altrimenti lo inserisco e basta
void insertChild(pcb_t *prnt, pcb_t *p)
{
    if (prnt != NULL && p != NULL)
    {
        if (prnt->p_child.next == NULL)
        {
            INIT_LIST_HEAD(&(prnt->p_child));
        }
        list_add_tail(&(p->p_sib), &(prnt->p_child));
        p->p_parent = prnt;
    }
    else
    {
        return;
    }
}

// Controllo se esiste il padre e il suo primo figlio, se si salvo quest'ultimo con il puntatore "u" e lo restituisco
struct pcb_t *removeChild(pcb_t *p)
{

    if (p != NULL && !emptyChild(p))
    {
        struct pcb_t *u;
        struct list_head *q = &(p->p_child);
        if (q != NULL || !list_empty(q))
        {
            u = container_of(q->next, typeof(*u), p_sib);
            list_del(q->next);
            u->p_parent = NULL;
            return u;
        }
    }
    return NULL;
}

// Controllo se il figlio esiste e ha effettivamente il padre, se si procedo all'eliminazione e alla restituzione del figlio da cancellare
struct pcb_t *outChild(pcb_t *p)
{
    if (p->p_parent != NULL && p != NULL)
    {
        list_del(&(p->p_sib));
        p->p_parent = NULL;
        return p;
    }
    return NULL;
}
