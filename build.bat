@echo off
if not exist build mkdir build

pushd build
cl ..\main.c /std:c17 /Fedansk.exe /nologo
popd