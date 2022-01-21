CC=clang
CFLAGS=-Wall -Wextra -pedantic -lpng -lnsbmp -fms-extensions -Wno-microsoft-anon-tag
OPTFLAGS=-O3
DEBUGFLAGS=-g -fsanitize=address
NAME=circlefit
IMAGES=out.png test.png test.bmp

CFLAGS += $(OPTFLAGS)

.PHONY: all
all: $(NAME)

$(NAME): $(NAME).c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(NAME)
	rm -f $(IMAGES)

.PHONY: install
install: $(NAME)
	install $(NAME) ~/bin/$(NAME)

.PHONY: lock
lock: $(NAME)
	maim -u | ./$(NAME) | i3lock --raw 3840x1130:rgb --image /dev/stdin

.PHONY: pngstdio
pngstdio: $(NAME)
	maim -u | ./$(NAME) | convert -size 3840x1130 -depth 8 RGB:- out.png

.PHONY: pngfile
pngfile: $(NAME)
	maim -u test.png
	./$(NAME) | convert -size 3840x1130 -depth 8 RGB:- out.png

.PHONY: bmpstdio
bmpstdio: $(NAME)
	maim -u -f bmp | ./$(NAME) | convert -size 3840x1130 -depth 8 RGB:- out.png

.PHONY: bmpfile
bmpfile: $(NAME)
	maim -u test.bmp
	./$(NAME) | convert -size 3840x1130 -depth 8 RGB:- out.png

