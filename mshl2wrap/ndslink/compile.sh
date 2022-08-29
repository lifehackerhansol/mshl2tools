#!/bin/sh
gcc -s -O2 -o ndslink main.c
i586-mingw32msvc-gcc -s -O2 -o ndslink.exe main.c
