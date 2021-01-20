CC			:= ppu-gcc
CXX			:= ppu-g++
LD			:= ppu-ld 
OBJCOPY		:= ppu-objcopy
AR 			:= ppu-ar
STRIP		:= ppu-strip
payload 	=  payload.bin
INCLUDE		=  ../../lv2/include ../../lv1/include ../../debug/include

INCLUDE		+= $(PS3DEV)/ppu/ppu/include
LIBSDIR		=
LIBS 		=
TARGET		= payload


CFLAGS = -ffunction-sections -fdata-sections -Os -m64 -fno-builtin -fno-exceptions \
		 -Os -Wall $(foreach dir,$(INCLUDE),-I$(dir)) \
		 -DLV2 -DFIRMWARE_CEX -ffreestanding -Wno-strict-aliasing -DUSE_LV1_PEEK_POKE 
CFLAGS += --std=gnu99
LDFLAGS=-nostartfiles -nostdlib -mbig-endian
LDFLAGS= -T $(TARGET).ld -nostartfiles -nostdlib -nodefaultlibs -Wl,-static -Wl,-s -L. $(foreach dir,$(LIBSDIR),-L$(dir)) $(LIBS) -Wl,--gc-sections -Wl

OBJS = start.o main.o \
	../../lv2/src/usb.o ../../lv2/src/patch.o ../../lv2/src/interrupt.o ../../lv2/src/interrupt_c.o ../../lv2/src/io.o ../../lv2/src/libc.o \
	../../lv2/src/libc_c.o ../../lv2/src/memory.o ../../lv2/src/memory_c.o ../../lv2/src/thread.o ../../lv2/src/thread_c.o ../../lv2/src/process.o \
	../../lv2/src/synchronization.o ../../lv2/src/modules.o ../../lv2/src/modules_c.o ../../lv2/src/storage.o ../../lv2/src/object.o \
	../../lv2/src/security.o ../../lv2/src/time.o ../../lv2/src/hid.o ../../lv2/src/pad.o ../../lv2/src/syscall.o \
	../../lv1/src/hvcall.o ../../lv1/src/stor.o ../../lv1/src/device.o 

all: $(TARGET).bin

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@	
	@rm -f *.o *.elf ../../lv1/src/*.o ../../lv2/src/*.o
	@cp $(TARGET).bin ../../../BIN

$(TARGET).elf: $(OBJS) $(TARGET).ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f *.o *.elf *.self *.bin *.486c *.map ../../lv1/src/*.o ../../debug/src/*.o ../../lv2/src/*.o
