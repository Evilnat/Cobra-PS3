
clean:
	rm -f *DEBUG *.o *.elf *.self *.480 *.bin *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o

all:
	rm -f *.o *.elf *.self *.480 *.bin *.480c *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o
	make -f Makefile_480C --no-print-directory

	rm -f *.o *.elf *.480 *.480d *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o
	make -f Makefile_480D --no-print-directory

	#rm -f *.o *.elf  *.480 *.bin *.480c *.480d *.map ../lv1/src/*.o ../debug/src/*.o ../lv2/src/*.o
