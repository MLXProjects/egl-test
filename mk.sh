#!/bin/sh
rm -f demo
gcc -o demo main.c tools.c -lEGL -lGLESv2