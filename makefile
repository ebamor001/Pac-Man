CC=gcc
CFLAGS=-Wall -Werror -std=c99 $(shell pkg-config --cflags sdl2)
LDLIBS=$(shell pkg-config --libs sdl2 SDL2_image)

SRCS=pacman.c movement.c map.c window_utils.c textures.c editor.c
OBJS=$(SRCS:.c=.o)

.PHONY: all clean


all: pacman

pacman: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS) -lm

clean:
	rm -f pacman $(OBJS)

