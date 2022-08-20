CFLAGS=-std=c11 -g -static -Wall -Werror -Wextra
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -rf *.o *~ tmp* *.out

fclean: clean
	rm -rf 9cc

re: fclean 9cc

.PHONY: test clean fclean
