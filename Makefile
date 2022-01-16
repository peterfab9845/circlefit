CC=clang
CFLAGS=-lpng -g -fsanitize=address
NAME=circlefit

.PHONY: all
all: $(NAME)

$(NAME): $(NAME).c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm $(NAME)

.PHONY: run
run: $(NAME)
	maim -u | ./$(NAME) | i3lock --raw 3840x1130:rgb --image /dev/stdin

