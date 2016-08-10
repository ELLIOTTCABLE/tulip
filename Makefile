cflags = --std=c11 -Isrc/ -g -lm -gdwarf-2 -g3 -lLLVM -l"lua5.2"
libs   = ./src/*/*.c
main   = ./src/main.c

scaffold = ./test/scaffold.c

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

$(BINARY): $(libs) $(main)
	mkdir -p build
	clang $(cflags) $^ -o $@

.PHONY: test-build test
test-build: $(TEST)
test: $(TEST)
	$(TEST)

$(TEST): $(libs) $(scaffold)
	mkdir -p build
	clang $(cflags) $^ -o $@

# library:
