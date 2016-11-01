cflags = --std=c11 -Isrc/ -g -lm -gdwarf-2 -g3 -lLLVM -l"lua5.2"
libs   = $(shell find src -name '*.c')
main   = ./src/main.c
BINARY = ./build/tulip

BINARY = ./build/tulip
TEST   = ./build/tulip-test

.PHONY: build run clean help

build: binary
run: $(BINARY)
	./build/tulip

debug: $(BINARY)
	gdb $^

clean:
	rm build/**

help:
	echo "targets: build, run, clean"

.PHONY: binary
binary: $(BINARY)

$(BINARY): $(srcs) $(libs) $(main)
	mkdir -p build
	clang $(cflags) $^ -o $@
