#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "pandos_const.h"
#include "pandos_types.h"
#include "initial.h"
#include "initProc.h"
#include "sysSupport.h"
#include <umps/libumps.h>

void initSwapStructs();
void uTLB_Pager();
int replacement_algorithm();
void atom(int);
int RW_flash_device(int flash_device_num, int frame_addr, int device_block_number, int op);

#endif