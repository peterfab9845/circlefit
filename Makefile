CC=clang
CFLAGS=-Wall -Wextra -pedantic -lpng -lnsbmp -fms-extensions -Wno-microsoft-anon-tag
OPTFLAGS=-O3
DEBUGFLAGS=-g -fsanitize=address
NAME=circlefit
IMAGES=*.bmp *.png *.asd
TESTS=test.sh

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
	maim -u -f bmp | ./$(NAME) | i3lock --raw 3840x1130:rgb --image /dev/stdin

.PHONY: test
test: $(NAME) $(TESTS)
	NAME=$(NAME) ./$(TESTS)

