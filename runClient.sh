#! /bin/sh

mkdir bin
gcc snake.c socklib.c utils.c draw.c client.c -o bin/client -l pthread -l ncursesw -l rt
bin/client