.PHONY: build

CC=gcc
APP=voir

PLATFORM:=$(shell uname -s)
CPU:=$(shell uname -m)

C_ERRS += -Wall -Wextra -Wpedantic \
		-Wformat=2 -Wno-unused-parameter -Wshadow \
		-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
		-Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
		-Wno-unused
STD:=c99

hash = $(shell git log --pretty=format:'%h' -n 1)

clean:
	rm -rf build

build:
	mkdir -p ./build/$(PLATFORM)/$(CPU)/

	$(CC) $(CUSTOM_CFLAGS) $(C_ERRS) -ggdb -O2 -std=$(STD) \
		./src/main.c \
		-I./vendor \
		-I./src \
		-o ./build/$(PLATFORM)/$(CPU)/$(APP).debug -lm

release_cli:
	mkdir -p ./build/$(PLATFORM)/$(CPU)/

	$(CC) $(CUSTOM_CFLAGS) $(C_ERRS) -O2 -std=$(STD) \
		./src/main.c \
		-I./vendor \
		-I./src \
		-o ./build/$(PLATFORM)/$(CPU)/$(APP) -lm
