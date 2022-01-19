CC=clang
CFLAGS=-Wall -Wextra -pedantic -lpng -fms-extensions -Wno-microsoft-anon-tag
OPTFLAGS=-O3
DEBUGFLAGS=-g -fsanitize=address
NAME=circlefit

CFLAGS += $(OPTFLAGS)

.PHONY: all
all: $(NAME)

$(NAME): $(NAME).c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(NAME)

.PHONY: run
run: $(NAME)
	maim -u | ./$(NAME) | i3lock --raw 3840x1130:rgb --image /dev/stdin

.PHONY: png
png: $(NAME)
	maim -u | ./$(NAME) | convert -size 3840x1130 -depth 8 RGB:- test.png

