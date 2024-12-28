.PHONY: build

CC=clang
APP=voir

C_ERRS += -Wall -Wextra -Wpedantic \
		-Wformat=2 -Wno-unused-parameter -Wshadow \
		-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
		-Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
		-Wno-unused

hash = $(shell git log --pretty=format:'%h' -n 1)

clean:
	rm -rf build

build:
	mkdir -p ./build

	$(CC) $(C_ERRS) -ggdb -O2 -std=c99 \
		./src/main.c \
		-I./vendor \
		-I./src \
		-o ./build/$(APP).debug -lm

release_cli:
	mkdir -p ./build

	$(CC) $(C_ERRS) -O2 -std=c99 \
		./src/main.c \
		-I./vendor \
		-I./src \
		-o ./build/$(APP) -lm
