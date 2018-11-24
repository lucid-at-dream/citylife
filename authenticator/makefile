.PHONY=test clean

CC_FLAGS=-Wall -O2

default: test

authenticator.o: authenticator.c authenticator.h
	gcc ${CC_FLAGS} -c authenticator.c -o authenticator.o

test.o: test.c test.h
	gcc ${CC_FLAGS} -c test.c -o test.o

test.exe: authenticator.o test.o unit_tests.c
	gcc ${CC_FLAGS} authenticator.o test.o unit_tests.c -o test.exe
	
test: test.exe
	@echo "Executing the test battery."
	./test.exe

clean:
	rm -f test.exe
	rm -f *.o

