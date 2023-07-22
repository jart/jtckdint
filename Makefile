check: test
	./test

test: test.o

test.o: test.c jtckdint.h

clean:
	rm -f test test.o
