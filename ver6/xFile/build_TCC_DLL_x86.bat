..\..\tcc\tcc -m32 -shared xFile.c ../xCore/xCore.c ../disphelper/tcc/uuid.c -DBUILD_DLL -o release/x86/xFile.dll

@echo;
@echo off

pause
