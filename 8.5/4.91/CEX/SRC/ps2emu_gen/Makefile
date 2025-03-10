OUTPUT		= ps2emu_gen
OBJS		= main.o 
CFLAGS		= -Wall -I. --std=gnu99
LDFLAGS		= -L. -lstdc++

all: $(OUTPUT)

clean:
	@echo -e "\e[1;32mCleaning ps2emu_gen...\e[0m"
	@rm -f *.exe *.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
	@rm -f *.o
