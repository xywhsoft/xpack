..\..\tcc\tcc -m64 -shared xFile.c ../xCore/xCore.c ../disphelper/tcc/uuid.c -DBUILD_DLL -o release/x64/xFile.dll

@echo;
@echo off

pause
