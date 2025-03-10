@echo off
cls

set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PS3DEV=%PS3SDK%/ps3dev2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%PS3DEV%/ppu/bin;
set CYGWIN=nodosfilewarning

cd lv2gen
make -f Makefile clean

cd ../nocfw_kern_plugin/payload
make -f Makefile_492C clean
make -f Makefile_492D clean

cd ../../ps2emu_gen
make -f Makefile clean

cd ../ps2emu_stage1_file
make -f Makefile clean

cd ../ps2emu_stage2/gxemu
make -f Makefile clean

cd ../hwemu
make -f Makefile clean

cd ../netemu
make -f Makefile clean

cd ../ps2netemu_gen
make -f Makefile clean

cd ../../stage0_file
make -f Makefile_492C clean
make -f Makefile_492D clean

cd ../stage2
make -f Makefile_492C clean
make -f Makefile_492D clean

cd ../test_payload/payload
make -f Makefile clean

echo Done!

pause