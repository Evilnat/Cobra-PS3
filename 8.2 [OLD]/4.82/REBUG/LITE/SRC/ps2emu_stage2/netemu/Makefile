CC			:= ppu-gcc
CXX			:= ppu-g++
LD			:= ppu-ld 
OBJCOPY		:= ppu-objcopy
OBJDUMP		:= ppu-objdump
AR 			:= ppu-ar
STRIP		:= ppu-strip
INCLUDE		=  ../common ../../ps2emu/include ../../lv1/include ../../debug/include
INCLUDE		+= $(PS3DEV)/ppu/ppu/include
LIBSDIR     =
LIBS        =
BUILD_TYPE  = release

GEN			= ../ps2netemu_gen/ps2netemu_gen
SRC_SELF	= ../ps2netemu_gen/ps2_netemu.self
OUT_SELF	= ps2_netemu.self


CFLAGS = -ffunction-sections -fdata-sections -Os -m64 -fno-builtin -fno-exceptions \
			   -Os -Wall -Wno-strict-aliasing $(foreach dir,$(INCLUDE),-I$(dir)) \
			   -DPS2EMU -DPS2NETEMU -DFIRMWARE_CEX -ffreestanding 
CFLAGS += --std=gnu99

ifeq ($(BUILD_TYPE), debug)
CFLAGS += -DDEBUG
endif

ifeq ($(BUILD_TYPE), release)
endif

LDFLAGS= -T ldscript.ld -nostartfiles -nostdlib -nodefaultlibs -Wl,-static -Wl,-s -L. $(foreach dir,$(LIBSDIR),-L$(dir)) $(LIBS) \
	-Wl,--gc-sections -Wl,-Map=ps2netemu_stage2.map

OBJS = start.o main.o ../../ps2emu/src/libc.o ../../ps2emu/src/libc_c.o ../../ps2emu/src/memory.o ../../ps2emu/src/io.o \
	../../ps2emu/src/patch.o 

ifeq ($(BUILD_TYPE), debug)
OBJS += 
endif

all: ps2netemu_stage2.bin
	@cp $(OUT_SELF) ../../../BIN
	@rm -f *.o *.elf *.map temp.self ../../ps2emu/src/*.o ../../lv1/src/*.o ../../debug/src/*.o
	
objdump: ps2netemu_stage2.bin
	$(OBJDUMP) -D -EB -b binary -m powerpc:common64 ps2netemu_stage2.bin

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	$(GEN) $@ $(SRC_SELF) $(OUT_SELF) ps2netemu_stage2.map
	
ps2netemu_stage2.elf: $(OBJS) ldscript.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f *.o *.elf *.self *.bin *.map temp.self ../../ps2emu/src/*.o ../../lv1/src/*.o ../../debug/src/*.o
