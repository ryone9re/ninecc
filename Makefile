CFLAGS=-std=c11 -g -static -Wall -Werror -Wextra

9cc: 9cc.c

test: 9cc
	./test.sh

clean:
	rm -rf 9cc *.o *~ tmp*

re: clean 9cc

.PHONY: test clean
