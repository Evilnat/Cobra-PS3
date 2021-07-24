Feel free to download / modify.

Thanks to Cobra team, Joonie, Habib, Haxxxen, Alexander, Dean, KW, Nzv, Bguerville, Aldo, LightningMods and all who helped updating Cobra source.

# COBRA 8.3 Source

Ported from Cobra 8.3 from @aldostools.

###### New features in 8.3:
    • Failsafe Cobra stage2 (by bguerville/aldo)
    • Restore CFW Syscalls without reboot just entering to 'Settings > System Update' on XMB (by aldo)
    • Integrated Dynamic Control FAN (to control fan when webMAN is unloaded) (by aldo/Evilnat)
    • Support Photo GUI integration with webMAN MOD (mount games from 'Photo' column) (by aldo/DeViL303)
    • Get/Set FAN speed (by aldo)
    • Enable/disable features: Photo GUI, Restore Syscalls (by aldo)
    • Opcode to create CFW Syscalls (6, 7, 8, 9, 10, 11, 15, 389, 409) (by aldo)
    • Opcode to set fake accountID (by Evilnat)
    • Opcode to activate account (act.dat) (by Evilnat)
    • Opcode to create license (RIF) (by Evilnat)
    • Updated ps3mapi_load_process_modules to load custom modules and system modules (by haxxxen)
    • Added ps3mapi_get_process_module_info
    • Increased from 24 to 32 the max number of map paths (by aldo)
    • Added sm_get_temperature patch in kernel (by Evilnat)
    • Added sm_get_fan_policy patch in kernel (by Evilnat)
    • Added sm_set_fan_policy patch in kernel (by Evilnat)
    • Fixed Control FAN payload, avoids loading previous mode (by Evilnat)
    • Disable stage2.bin while Recovery Menu is loaded (by haxxxen)
    • Fixed max FAN speed after shutdown (by Evilnat)
    • Improve Habib's QA flags code imported by aldostools (by Evilnat)
    • Fixed black screen in CFW2OFW converted games (by Evilnat)
    • Added sm_ring_buzzer with single_beep, double_beep and triple_beep (by Evilnat)
    • Skip license creation (rif) if it already exists (by aldo)
    • Added cellFsMkdir symbol (by Evilnat)
    • Added constant FAN Speed while a PS2 ISO is launched (by Evilnat)
    • Check/Disable/Enable QA Flags (by Evilnat)
    • Creation of act.dat while launching a PSN game (by Evilnat)
    • Convert someones's else savedata to your own savedata (by Evilnat)
    
###### To do:
    All done

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
