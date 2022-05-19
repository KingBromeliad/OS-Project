#ifndef ASL_H
#define ASL_H

#include "pandos_types.h"

/* ASL handling functions */
extern semd_t* getSemd(int *key);
extern void initASL();

extern int insertBlocked(int *key,pcb_t* p);
extern pcb_t* removeBlocked(int *key);
extern pcb_t* outBlocked(pcb_t *p);
extern pcb_t* headBlocked(int *key);
extern void outChildBlocked(pcb_t *p);

#endif
