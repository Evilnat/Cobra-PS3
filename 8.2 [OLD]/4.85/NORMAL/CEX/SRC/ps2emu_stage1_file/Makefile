
clean:
	@rm -f *.o *.elf *.bin *.self *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o

all:
	@rm -f *.o *.elf *.bin *.self  *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o
	@make -f Makefile.hwemu --no-print-directory
	
	@rm -f *.o *.elf *.bin  *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o
	@make -f Makefile.gxemu --no-print-directory
	@rm -f *.o *.elf *.bin  *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o

