@echo off
del CMakeCache.txt >nul 2>&1
cmake -G "Visual Studio 15 Win64"