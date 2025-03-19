@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
