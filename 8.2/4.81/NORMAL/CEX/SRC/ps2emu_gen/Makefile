OUTPUT		= ps2emu_gen
OBJS		= main.o 
CFLAGS		= -Wall -I. --std=gnu99
LDFLAGS		= -L. -lstdc++

all: $(OUTPUT)

clean:
	@rm -f *.exe *.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
	@rm -f *.o
