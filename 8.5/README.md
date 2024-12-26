Feel free to download / modify.

Thanks to Cobra team, Joonie, Habib, Haxxxen, Alexander, Dean, KW, Nzv, Bguerville, Aldo, LightningMods and all who helped updating Cobra source.

# COBRA 8.5 Source

###### New features in 8.5:
    • TTYWRITE support in socat (Thanks to haxxxen and PS3HEN team)
    • Fixed controller infinite loop (Thanks to aldostools)
    • Added asm code embedding to display temperature when compiling ps2_netemu.self (Thanks to @3141card for his research)
    • Support for 3k3y/Redump ISOs on-the-fly  
    • Changed base load to 0x8000000000001000 in stage0
    • Opcode to set SYSCALLS mode at boot
    • Opcode to set GAMEBOOT mode
    • Opcode to set EPILEPSY WARNING mode
    • Opcode to set COLDBOOT mode at boot
    • Opcode to set HIDDEN TROPHIES mode    
    • Sorry if I forgot something else...
    
###### To do:
    • rap2bin - bin2rap support by @esc0rtd3w
    • Patch to allow universal DVD region
    • Patch PS2CLASSICS to be able to play it on DEX

### Data that need to be ported to every CFW version

The following files have data that must be ported to each firmware version:

    • lv2\include\lv2\symbols.h
    • ps2emu\include\ps2emu\symbols.h
    • ps2emu_stage1_file\Makefile.gxemu
    • ps2emu_stage1_file\Makefile.hwemu
    • ps2emu_stage2\gxemu\main.c
    • ps2emu_stage2\hwemu\main.c
    • ps2emu_stage2\netemu\main.c
    • stage0_file\Makefile
    • stage0_file\start.S
    • stage2\main.c
    • stage2\modulespatch.h
    • stage2\ps3mapi_core.h
    • stage2\qa.h
