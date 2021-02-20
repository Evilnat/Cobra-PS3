Feel free to download / modify.

Thanks to Cobra team, Joonie, Habib, Haxxxen, Alexander, Dean, KW, Nzv, Bguerville, Aldo, LightningMods and all who helped updating Cobra source.

# COBRA 8.3 Source

WIP of Cobra 8.3 from @aldostools.

reActPSN are not added yet due to freezes.

This repository will have constant changes.

###### New features in 8.3:
    . Failsafe Cobra stage2 (by bguerville/aldo)
    . Restore disabled CFW Syscalls without Reboot just entering to Settings > System Update on XMB (by aldo)
    . Integrated fan controller (to control fan when webMAN is unloaded) (by aldo)
    . Support Photo GUI integration with webMAN MOD (mount games from Photo column) (by aldo/DeViL303)
    . Get / Set fan speed (by aldo)
    . Enable/disable features: Photo GUI, Restore Syscalls (by aldo)
    . Opcode to create CFW Syscalls (6, 7, 8, 9, 10, 11, 15, 389, 409) (by aldo)
    . Updated ps3mapi_load_process_modules to load custom modules and system modules (by haxxxen)
    . Added ps3mapi_get_process_module_info
    . Increased from 24 to 32 the max number of map paths (by aldo)
    . Added sm_get_temperature patch in kernel (by Evilnat)
    . Added sm_get_fan_policy patch in kernel (by Evilnat)
    . Added sm_set_fan_policy patch in kernel (by Evilnat)
    . Fixed Control FAN payload, avoids loading previous mode (by Evilnat)
    . Disable stage2.bin while Recovery Menu is loaded (by haxxxen)
    . Fixed max FAN speed after shutdown (by Evilnat)
    . Improved Habib's QA flags code imported by aldostools (by Evilnat)
	. Fixed black screen in CFW2OFW converted games (by Evilnat)
	. Added sm_ring_buzzer with single_beep, double_beep and triple_beep (by Evilnat)
	. Skip the creation of rif if already exists (by aldo)
    
###### To do:
    . reActPSN
	
