STD:=gnu99 # use gnu99 so we can use gnu extensions (like asm)
CC:=gcc -m32 --std=$(STD) -g

all: TAGS bin/threads

run: all
	bin/threads

debug: all
	gdb bin/threads

clean:
	rm -r bin/*

bin/threads: bin/threads.c.o bin/threads.s.o bin/main.c.o
	$(CC) -o $@ $^

bin/main.c.o: main.c threads.h utility.h
	$(CC) -o $@ -c $<

bin/threads.c.o: threads.c threads.h utility.h
	$(CC) -o $@ -c $<

bin/threads.s.o: threads.s
	nasm -f elf32 -o $@ $^

TAGS: *.c
	etags $<

.PHONY: all run clean
