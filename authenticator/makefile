.PHONY=test clean

CC_FLAGS=-ansi -Wall -O2

default: test

authenticator.o: authenticator.c authenticator.h
	gcc ${CC_FLAGS} -c authenticator.c -o authenticator.o

test.exe: authenticator.o test.c
	gcc ${CC_FLAGS} authenticator.o test.c -o test.exe
	
test: test.exe test.c
	@echo "Executing the test battery."
	./test.exe

clean:
	rm -f test.exe
	rm -f *.o

