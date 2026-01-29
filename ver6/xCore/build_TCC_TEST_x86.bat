..\..\tcc\tcc -m32 test.c xCore.c ../disphelper/tcc/uuid.c -o release/x86/test.exe

@echo;
@echo off

cd release
cd x86
test.exe

pause
