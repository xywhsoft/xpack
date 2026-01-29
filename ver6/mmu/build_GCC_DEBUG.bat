gcc test.c mmu.c -g -o release/x64/test.exe

@echo;
@echo off

cd release
cd x64
pause
test.exe
