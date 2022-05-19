#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pandos_const.h"
#include "pandos_types.h"
#include "initProc.h"
#include "vmSupport.h"
#include <umps/libumps.h>

void generalExceptionHandler();
void userSyscallHandler(support_t *supportPtr);
void trapHandler(support_t *supportPtr);

void getTOD(support_t *supportPtr);
void terminate(support_t *supportPtr);
void writeToPrinter(support_t *supportPtr);
void writeToTerminal(support_t *supportPtr);
void readFromTerminal(support_t *supportPtr);
#endif