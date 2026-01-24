@echo off
mkdir build 2>nul
cl /std:c++20 /EHsc src\main.cpp /Fo:build\ /Fe:build\program.exe