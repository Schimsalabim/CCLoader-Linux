#!/bin/sh

CC=gcc
#CC=arm-linux-gnueabihf-gcc
CC=arm-buildroot-linux-gnueabihf-gcc

$CC -g -L. -o main main.c -lgpiod

(cd CCLoader_Firmware && ./build.sh)
