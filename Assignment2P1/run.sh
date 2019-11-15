#!/bin/bash
clear
gcc -o $1 $1.c -lpthread
./$1
