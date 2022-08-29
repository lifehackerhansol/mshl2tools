#!/bin/sh
DSI_SD=0 make && make clean
mv -f boot.bin boot_nodsisd.bin
DSI_SD=1 make && make clean
