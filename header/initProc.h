#ifndef INITPROC_H
#define INITPROC_H

#include "pandos_const.h"
#include "pandos_types.h"
#include "sysSupport.h"
#include "vmSupport.h"
#include <umps/libumps.h>

/* each terminal device should have two mutual exclusion semaphore, one for reading and one for writing */
#define DEVSEMNUM DEVICECNT+DEVPERINT

void test();
void initUProc();

#endif