# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-linux-gnu-
# On fedora make XT_PRG_PREFIX=mips64-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

# uMPS3-related paths

# Simplistic search for the umps3 installation prefix.
# If you have umps3 installed on some weird location, set UMPS3_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3/

DEFS = ../header/asl.h ../header/exceptions.h ../header/initial.h ../header/interrupts.h ../header/listx.h ../header/pandos_types.h ../header/pandos_const.h ../header/pcb.h ../header/scheduler.h ../header/klog.h $(UMPS3_INCLUDE_DIR)umps/libumps.h Makefile

# Compiler options
CFLAGS_LANG = -ffreestanding
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -EL -G 0 -mno-abicalls -fno-pic -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS3_INCLUDE_DIR) -Wall -O0

# Linker options
LDFLAGS = -G 0 -T $(UMPS3_DATA_DIR)/umpscore.ldscript -m elf32ltsmip

# Add the location of crt*.S to the search path
VPATH = $(UMPS3_DATA_DIR)

.PHONY : all clean

all : kernel.core.umps

kernel.core.umps : kernel
	umps3-elf2umps -k $<

kernel : p2test.o initial.o interrupts.o scheduler.o exceptions.o asl.o pcb.o klog.o libumps.o crtso.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean :
	-rm -f *.o kernel kernel.*.umps

# Pattern rule for assembly modules
%.o : %.S $(DEFS) 
	$(CC) $(CFLAGS) -c -o $@ $<