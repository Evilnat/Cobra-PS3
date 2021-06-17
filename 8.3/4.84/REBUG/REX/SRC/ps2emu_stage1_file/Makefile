
clean:
	@echo -e "\e[1;32mCleaning ps2emu_stage1_file...\e[0m"
	@rm -f *.o *.elf *.bin *.self *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o

all:
	@rm -f *.o *.elf *.bin *.self  *.map *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o
	@make -f Makefile.hwemu --no-print-directory
	
	@rm -f *.o *.elf *.bin  *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o
	@make -f Makefile.gxemu --no-print-directory
	
	@rm -f *.o *.elf *.bin  *.lzma ../lv2/src/*.o ../lv1/src/*.o ../debug/src/*.o

