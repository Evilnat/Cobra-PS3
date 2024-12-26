all:
	@echo -e "\e[1;33mCompiling lv2gen...\e[0m"
	@cd lv2gen && make all --no-print-directory

	@echo -e "\n\e[1;33mCompiling nocfw_kern_plugin...\e[0m"
	@cd nocfw_kern_plugin/payload && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling ps2emu_gen...\e[0m"
	@cd ps2emu_gen && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling ps2netemu_gen...\e[0m"
	@cd ps2emu_stage2/ps2netemu_gen && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling ps2emu_stage1_file...\e[0m"
	@cd ps2emu_stage1_file && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling ps2emu_stage2/gxemu...\e[0m"
	@cd ps2emu_stage2/gxemu && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling ps2emu_stage2/hwemu...\e[0m"
	@cd ps2emu_stage2/hwemu && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling ps2emu_stage2/netemu...\e[0m"
	@cd ps2emu_stage2/netemu && make all --no-print-directory

	@echo -e "\n\e[1;33mCompiling stage0...\e[0m"
	@cd stage0_file && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling stage2...\e[0m"
	@cd stage2 && make all --no-print-directory
	
	@echo -e "\n\e[1;33mCompiling test_payload...\e[0m"
	@cd test_payload/payload && make all --no-print-directory

	@echo -e "\n\e[1;32mDONE! Files copied to BIN folder\e[0m"