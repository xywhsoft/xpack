..\..\tcc\tcc -m32 -shared xPack.c ../xCore/xCore.c ../xFile/xFile.c ../mmu/mmu.c ../xxhash32/release/x86/xxhash32.o ../lz4/release/x86/lz4.o ../lzma/release/x86/lzma.o -DBUILD_DLL -o release/x86/xPack.dll

@echo;
@echo off

pause
