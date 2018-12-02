
clean:
	rm -f *DEBUG *.o *.elf *.self *.453 *.bin *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o

all:
	rm -f *.o *.elf *.self *.453 *.bin *.453c *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o
	make -f Makefile_453C --no-print-directory

	rm -f *.o *.elf *.453 *.453d *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o
	make -f Makefile_453D --no-print-directory

	#rm -f *.o *.elf  *.453 *.bin *.453c *.453d *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o
