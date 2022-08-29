#!/bin/sh
gcc -O2 -o mselink main.c
strip mselink
i586-mingw32msvc-gcc -O2 -o mselink.exe main.c
i586-mingw32msvc-strip mselink.exe
