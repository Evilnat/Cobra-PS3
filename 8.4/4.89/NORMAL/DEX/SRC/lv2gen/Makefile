CC 			= gcc
OUTPUT 		= lv2gen
OBJS 		= main.o 
CFLAGS 		= -Wall -I. --std=gnu99
LDFLAGS 	= -L. -lstdc++

all: $(OUTPUT)

clean:
	@echo -e "\e[1;32mCleaning lv2gen...\e[0m"
	@rm -f *.o *.exe

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
	@rm -f *.o
