OUTPUT 		= ps2netemu_gen
OBJS 		= main.o 
CFLAGS 		= -Wall $(INC) --std=gnu99
LDFLAGS 	= -L. -lstdc++
INC 		= -I. -I../../ps2emu/include

all: $(OUTPUT)

clean:
	@rm -f *.exe *.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
	@rm -f *.o
