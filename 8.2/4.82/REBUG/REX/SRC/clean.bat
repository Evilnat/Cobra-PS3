@echo off
cls

set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PS3DEV=%PS3SDK%/ps3dev2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%PS3DEV%/ppu/bin;
set CYGWIN=nodosfilewarning

echo Cleaning lv2gen...
cd lv2gen
make -f Makefile clean

echo Cleaning nocfw_kern_plugin...
cd ../nocfw_kern_plugin/payload
make -f Makefile clean

echo Cleaning ps2emu_gen...
cd ../../ps2emu_gen
make -f Makefile clean

echo Cleaning ps2emu_stage1_file...
cd ../ps2emu_stage1_file
make -f Makefile clean

echo Cleaning gxemu...
cd ../ps2emu_stage2/gxemu
make -f Makefile clean

echo Cleaning hwemu...
cd ../hwemu
make -f Makefile clean

echo Cleaning netemu...
cd ../netemu
make -f Makefile clean

echo Cleaning ps2netemu_gen...
cd ../ps2netemu_gen
make -f Makefile clean

echo Cleaning stage0_file...
cd ../../stage0_file
make -f Makefile clean

echo Cleaning stage2...
cd ../stage2
make -f Makefile clean

echo Cleaning test_payload...
cd ../test_payload/payload
make -f Makefile clean

echo Done!

pause