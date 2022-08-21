INCLUDE=-I./includes
CFLAGS=-std=c11 -g -static -Wall -Werror -Wextra $(INCLUDE)
SRCS=$(wildcard srcs/*.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

test: 9cc
	./test.sh

clean:
	$(RM) *.o *~ tmp* *.out */*.o */tmp* */*.out

fclean: clean
	$(RM) 9cc

re: fclean 9cc

.PHONY: test clean fclean
