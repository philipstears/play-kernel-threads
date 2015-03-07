STD:=gnu99 # use gnu99 so we can use gnu extensions (like asm)
CC:=gcc -m32 --std=$(STD) -g

all: TAGS bin/threads

run: all
	bin/threads

debug: all
	gdb bin/threads

clean:
	rm -r bin/*

bin/threads: threads.c
	$(CC) -o $@ $<

TAGS: *.c
	etags $<

.PHONY: all run clean
