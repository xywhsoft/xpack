..\..\tcc\tcc -m64 test.c xPack.c ../xCore/release/x64/xcore.o ../xFile/release/x64/xFile.o ../smm/release/x64/smm.o ../xxhash32/release/x64/xxhash32.o ../lz4/release/x64/lz4.o ../lzma/release/x64/lzma.o -o release/x64/test.exe

@echo;

cd release
cd x64
test.exe

pause
