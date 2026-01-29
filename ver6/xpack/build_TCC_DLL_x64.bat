tcc -m64 -shared xPack.c ../xCore/xCore.c ../xFile/xFile.c ../mmu/mmu.c ../xxhash32/release/x64/xxhash32.o ../lz4/release/x64/lz4.o ../lzma/release/x64/lzma.o -DBUILD_DLL -o release/x64/xPack.dll

@echo;
@echo off

pause
