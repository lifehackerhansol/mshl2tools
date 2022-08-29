#!/bin/sh
gcc -s -O2 -o mselink main.c
i586-mingw32msvc-gcc -s -O2 -o mselink.exe main.c
