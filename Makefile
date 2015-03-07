all: bin/threads

run: all
	bin/threads

clean:
	rm -r bin/*

bin/threads: threads.c
	gcc --std=c99 -o $@ $<

.PHONY: all run clean
