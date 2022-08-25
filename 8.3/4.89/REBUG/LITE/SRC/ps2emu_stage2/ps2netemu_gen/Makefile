OUTPUT 		= ps2netemu_gen
OBJS 		= main.o 
CFLAGS 		= -Wall $(INC) --std=gnu99
LDFLAGS 	= -L. -lstdc++
INC 		= -I. -I../../ps2emu/include

all: $(OUTPUT)

clean:
	@echo -e "\e[1;32mCleaning ps2netemu_gen...\e[0m"
	@rm -f *.exe *.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
	@rm -f *.o
