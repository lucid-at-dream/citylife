.PHONY=test clean memleak default compile full_build

CC_FLAGS= -Wextra -fprofile-arcs -ftest-coverage --coverage -std=c99

default: compile

compile: test.exe

full_build: test memleak

sonar: clean
	build-wrapper-linux-x86-64 --out-dir bw-output make compile test
	mkdir -p gcov-reports
	gcov map.c assert.c authenticator.c test.c
	cd ..; sonar-scanner

authenticator.o: authenticator.c authenticator.h
	gcc ${CC_FLAGS} -c authenticator.c -o authenticator.o

map.o: map.c map.h
	gcc ${CC_FLAGS} -c map.c -o map.o

test.o: test.c test.h
	gcc ${CC_FLAGS} -c test.c -o test.o

assert.o: assert.c assert.h
	gcc ${CC_FLAGS} -c assert.c -o assert.o

test.exe: authenticator.o test.o map.o assert.o unit_tests.c
	gcc ${CC_FLAGS} authenticator.o test.o map.o assert.o unit_tests.c -o test.exe

memleak:
	sh ../ci-scripts/memory_leak_check.sh ./test.exe

test: test.exe
	@echo "Executing the test battery."
	./test.exe

clean:
	rm -rf bw_output bw-output .scannerwork out.strace
	rm -f test.exe
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno

