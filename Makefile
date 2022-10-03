NAME=ninecc

CFLAGS=-std=c11 -g -fno-common -Wall -Wextra -Werror -Wno-switch
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(NAME)
	./test.sh

clean:
	$(RM) *.o *~ tmp* *.out */*.o */tmp* */*.out

fclean: clean
	$(RM) $(NAME)

re: fclean $(NAME)

.PHONY: test clean fclean
