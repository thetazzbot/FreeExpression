# Freecut: open firmware for Cricut Expression 1 devices.
#
# 
# MCU name
MCU = atmega1281

# Output format. (can be srec, ihex)
FORMAT = ihex

# Target file name (without extension).
TARGET = freecut

# Speed
FCLK = 16000000UL

# List C source files here. (C dependencies are automatically generated.)
SRC = main.c usb.c lcd.c keypad.c timer.c stepper.c cli.c flash.c dial.c hpgl.c shvars.c scale.c

# Assembler sources 
ASRC = 

# Optional compiler flags.
CFLAGS  = -D$(MCU) $(TEST) $(CE) -Os -DFCLK=$(FCLK) -fpack-struct 
CFLAGS += -fshort-enums -Wall -Werror -Wstrict-prototypes 

# Optional assembler flags.
ASFLAGS = -Wa,-ahlms=$(<:.S=.lst),-gstabs 

# Optional linker flags.
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref

# Additional libraries
#
# Minimalistic printf version
#LDFLAGS += -Wl,-u,vfprintf -lprintf_min
#
# Floating point printf version (requires -lm below)
#LDFLAGS +=  -Wl,-u,vfprintf -lprintf_flt
#
# -lm = math library
LDFLAGS += -lm

# ---------------------------------------------------------------------------

# Define directories, if needed.

# Define programs and commands.
SHELL = sh

CC = avr-gcc

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

# Define all object files.
OBJ = $(ASRC:.S=.o) $(SRC:.c=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(SRC:.c=.lst)

# Combine all necessary flags and optional flags. Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) 
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

# Default target.
all: $(TARGET).elf $(TARGET).hex $(TARGET).lss tags

.PHONY: prog 

#you may need to change this line to program your device
prog: all 
	avrdude -pm128 -cstk500v2 -b115200 -P/dev/ttyUSB0 -u -V -U flash:w:$(TARGET).hex:i 

# Create final output files (.hex) from ELF output file.
%.hex: %.elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

# Disassembly listing
%.dis: %.elf
	$(OBJDUMP) -d $< > $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)


# Compile: create object files from C source files.
%.o : %.c
	$(CC) -c $(ALL_CFLAGS) $< -o $@


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.S
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.

clean :
	rm -f *.hex
	rm -f *.obj
	rm -f *.elf
	rm -f *.map
	rm -f *.obj
	rm -f *.sym
	rm -f *.ssm
	rm -f *.lnk
	rm -f *.lss
	rm -f *.lst
	rm -f $(OBJ)
	rm -f $(LST)
	rm -f $(SRC:.c=.s)
	rm -f $(SRC:.c=.d)
	rm -f logfile


# Automatically generate C source code dependencies. 
# (Code originally taken from the GNU make user manual and modified (See README.txt Credits).)
# Note that this will work with sh (bash) and sed that is shipped with WinAVR (see the SHELL variable defined above).
# This may not work with other shells or other seds.
%.d: %.c
	set -e; $(CC) -MM $(ALL_CFLAGS) $< \
	| sed  -e 's,\(.*\)\.o[ :]*,\1.o \1.d : ,g' \
	> $@; [ -s $@ ] || rm -f $@


# Remove the '-' if you want to see the dependency files generated.
-include $(SRC:.c=.d)


# Listing of phony targets.
.PHONY : all clean 

tags: *.[hc]
	ctags *.[hc]


