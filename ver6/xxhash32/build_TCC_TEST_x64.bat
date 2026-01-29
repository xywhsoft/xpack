..\..\tcc\tcc -m64 test.c -o release/x64/test.exe

@echo;
@echo off

cd release
cd x64
test.exe

pause
