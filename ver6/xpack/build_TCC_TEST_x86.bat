..\..\tcc\tcc -m32 test.c xPack.c ../xCore/release/x86/xcore.o ../xFile/release/x86/xFile.o ../smm/release/x86/smm.o ../xxhash32/release/x86/xxhash32.o ../lz4/release/x86/lz4.o ../lzma/release/x86/lzma.o -o release/x86/test.exe

@echo;
@echo off

cd release
cd x86
test.exe

pause
