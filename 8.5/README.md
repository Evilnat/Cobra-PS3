Feel free to download / modify.

Thanks to Cobra team, Joonie, Habib, Haxxxen, Alexander, Dean, KW, Nzv, Bguerville, Aldo, LightningMods, Chattrapat Sangmanee and all who helped updating Cobra source.

# COBRA 8.5 Source

###### New features in 8.5:
    • Overclock RSX's core and memory frequencies at boot from USB/HDD
    • TTYWRITE support in socat (Thanks to haxxxen and PS3HEN team)
    • Fixed controller infinite loop (Thanks to aldostools)
    • Added asm code embedding to display temperature when compiling ps2_netemu.self (Thanks to @3141card for his research)
    • Support for 3k3y/Redump ISOs on-the-fly  
    • Opcode to set SYSCALLS mode at boot
    • Opcode to set GAMEBOOT mode
    • Opcode to set COLDBOOT mode at boot
    • Opcode to set HIDDEN TROPHIES mode
    
###### To do:
    • Patch to allow universal DVD region

## RSX Overclock
Create a file called **overclock.txt**, and put the frequencies you want to use

For example, I want to overclock RSX's core to 600MHz and memory to 750MHz, this is how it would be in the file, first line is for **Core** and the second one for **Memory**:

<img src="https://raw.githubusercontent.com/Evilnat/Cobra-PS3/master/overclock.png" width="500">

This will overclock RSX's **Core** to **600MHz** and **Memory** to **750MHz**

Once you have completed the previous step, you just need to copy the "overclock.txt" file to the root of a USB device in **FAT32** format or to the internal hard drive of the PS3

This is what the routes would be like:

- **PC USB:**  X:\overclock.txt
- **PS3 USB:** /dev_usbXXX/overclock.txt
- **PS3 HDD:** /dev_hdd0/overclock.txt

> [!CAUTION]
> **Just remember to only use numbers, do not use letters or other characters**

Start or restart the PS3 so that Cobra loads the file, once the XMB has finished loading, go to the Network column and choose the option [Custom Firmware Tools > Overclock Tools > Check GPU/VRAM clock speed]

> [!NOTE]
> **Cobra will check if /dev_usb000 is mounted and it will overclock the PS3. If you turn on the PS3 without a USB device and  you connect one after a while, Cobra will overclock**

>[!NOTE]
> **Overclocking from USB has priority over HDD, if the PS3 crashes with the file on the internal HDD, you can use the USB with smaller values ​​to boot the PS3 and delete the file**

Default values are 500MHz for Core and 650MHz for Memory

## Data that need to be ported to every CFW version

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
