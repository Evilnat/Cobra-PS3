clean:
	@rm -f *~ *.*~ *.o *.elf *.bin *.cex *.dex *.release *.debug *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o

all:
	@rm -f *.o *.elf *.bin *.release *.debug *.cex *.dex *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o
	@make -f Makefile_490C --no-print-directory
	
	@rm -f *.o *.elf *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o
	@make -f Makefile_490D --no-print-directory
