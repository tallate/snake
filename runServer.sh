#! /bin/sh

mkdir bin
gcc snake.c socklib.c utils.c server.c -o bin/server -l pthread
bin/server