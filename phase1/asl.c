
#include "../header/asl.h"
#include "../header/pcb.h"
#include "../header/listx.h"
#include "../header/pandos_const.h"
#include "../header/pandos_types.h"

//sentinella lista dei semafori liberi
HIDDEN LIST_HEAD(semdFree_h);
//sentinella della lista della ASL (semafori attivi)
HIDDEN LIST_HEAD(semd_h);
//lista dei semafori
struct semd_t semdTable[MAXPROC];
//ritorna la key dell elemento puntato dal puntatore list_head
int* key_of(struct list_head* h){
    return ((container_of(h, struct semd_t , s_link))->s_key);
}

void initASL(){
	for (int i = 0; i < MAXPROC; i++){
		INIT_LIST_HEAD(&semdTable[i].s_procq);/*inizializzo la s_procq*/
		list_add(&semdTable[i].s_link, &semdFree_h);/*inizializzo la lista s_link*/
	}
}
//ritorna il puntatore al semaforo data lasua chiave key, se non viene trovato ritorna null
struct semd_t* getSemd(int* key){
	struct list_head *tmp;
	tmp = semd_h.next;
	while (tmp != &semd_h && (key_of(tmp) < key)){//ciclo finchè la key del semaforo diventa maggiore di key dato che sono ordinati in ordine crescente
		tmp = tmp->next;
	}
	if(key_of(tmp) == key) return container_of(tmp, struct semd_t, s_link);//ritorno il puntatore al semaforo se la chiave è uguale a key
	else return NULL;
}


int insertBlocked(int *key, struct pcb_t *p){
	if( p == NULL ) return 2;//controllo se il pcb esiste
	struct list_head *tmp;
	struct semd_t* semdtmp;
	tmp = semd_h.next;
	semdtmp = getSemd(key);//prendo il semaforo corretto
	if (semdtmp == NULL){ //se il semaforo è presente inserisco il pcb nella lista dei processi del semaforo corretto
		if(list_empty(&semdFree_h)) return TRUE;
		while(tmp != &semd_h && (key_of(tmp) < key)){
			tmp = tmp->next;
		}
		struct list_head* h;
		h = semdFree_h.next;
		list_del(h);
		list_add(h, tmp->prev);
		semdtmp = container_of(h, struct semd_t, s_link);
		semdtmp->s_key = key;//inizializzo la s_key
	}
	p->p_semAdd = semdtmp->s_key;
	list_add_tail(&p->p_list, &semdtmp->s_procq);
	return 0;
}


void remove_if_empty(semd_t* semd){
	if (emptyProcQ(&semd->s_procq) == TRUE){/*se la lista dei processi bloccati del semaforo con chiave key è vuota il semaforo viene tolto dalla asl e messo nella semdFree*/
		list_del(&semd->s_link);
		list_add(&semd->s_link, &semdFree_h);
        }
}



struct pcb_t* removeBlocked(int *key){
        struct semd_t* semd;
        semd = getSemd(key);
        if (semd != NULL){//controllo se è presente un semaforo con chiave key
		pcb_t* pcb_tmp;
                pcb_tmp = removeProcQ(&(semd->s_procq));//rimuovo il processo dalla lista dei processi bloccati
                remove_if_empty(semd);//rimuovo il semaforo se la s_procq è vuota
                return pcb_tmp;
        }else{
                return NULL;//se il semaforo con chiave key non esiste ritorno NULL
        }
}



struct pcb_t* outBlocked(pcb_t *p){
	semd_t* semd;
	semd = getSemd(p->p_semAdd); /*viene cercato il semaforo con key uguale alla semkey del pcb*/
	if(semd != NULL){
		return outProcQ(&semd->s_procq, p); /*viene rimosso e ritornato il pcb che stavamo cercando, ritorna NULL se non è presente nella coda*/
	}else{
		return NULL;
	}
		
}


struct pcb_t* headBlocked(int *key){
	semd_t* semd;
	semd = getSemd(key);
	if(semd != NULL && headProcQ(&semd->s_procq) != NULL){ /*se semd non è vuoto e s_procq non è vuoto viene ritornato l elemento in testa con la funzione headProcQ()*/
		return(headProcQ(&semd->s_procq));
	}
	return NULL;
}

void outChildBlocked(struct pcb_t *p){
	if(p == NULL) return;/*se il pcb è uguale a NULL la funzione ritorna e viene scansioinata un'altra parte di albero*/
	else{
		struct list_head *tmp;
		tmp = p->p_child.next;
		while (tmp != &(p->p_child) && tmp != NULL) {/*elimina tutti i processi dell’albero radicato in p (ossia tutti i processi che hanno come avo p) dalle eventuali code dei semafori su cui sono bloccati*/
        		outChildBlocked(container_of(tmp, pcb_t, p_sib));
        		outBlocked(container_of(tmp, pcb_t, p_sib));
        		tmp = tmp->next;
   		}
    		outBlocked(p);//Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato
	}
}


