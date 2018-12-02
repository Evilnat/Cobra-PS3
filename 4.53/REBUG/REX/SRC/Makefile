all:
	@echo -e "Compiling lv2gen...\n"
	@cd lv2gen && make all --no-print-directory
	@echo -e "\nCompiling ps2emu_gen..."
	@cd ps2emu_gen && make all --no-print-directory
	@echo -e "\nCompiling ps2netemu_gen..."
	@cd ps2emu_stage2/ps2netemu_gen && make all --no-print-directory
	@echo -e "\nCompiling ps2emu_stage1_file..."
	@cd ps2emu_stage1_file && make all --no-print-directory
	@echo -e "\nCompiling ps2emu_stage2/gxemu..."
	@cd ps2emu_stage2/gxemu && make all --no-print-directory
	@echo -e "\nCompiling ps2emu_stage2/hwemu..."
	@cd ps2emu_stage2/hwemu && make all --no-print-directory
	@echo -e "\nCompiling ps2emu_stage2/netemu..."
	@cd ps2emu_stage2/netemu && make all --no-print-directory
	@echo -e "\nCompiling stage0_file..."
	@cd stage0_file && make all --no-print-directory
	@echo -e "\nCompiling stage1_file..."
	@cd stage1_file && make all --no-print-directory
	@echo -e "\nCompiling stage2..."
	@cd stage2 && make all --no-print-directory

	@echo -e "\nDONE! Files copied to BIN folder"