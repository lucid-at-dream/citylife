.PHONY=test clean memleak default compile full_build

CC_FLAGS= -g -Wall -fprofile-arcs -ftest-coverage --coverage -std=c99

default: compile

## File compilation targets

authenticator.o: authenticator.c authenticator.h
	gcc ${CC_FLAGS} -c authenticator.c -o authenticator.o

map.o: map.c map.h
	gcc ${CC_FLAGS} -c map.c -o map.o

server.o: server.c server.h
	gcc ${CC_FLAGS} -c server.c -o server.o

test.o: test.c test.h
	gcc ${CC_FLAGS} -c test.c -o test.o

assert.o: assert.c assert.h
	gcc ${CC_FLAGS} -c assert.c -o assert.o

## Executables targets

map_tests.exe: authenticator.o test.o map.o assert.o map_tests.c
	gcc ${CC_FLAGS} authenticator.o test.o map.o assert.o map_tests.c -o map_tests.exe

authenticator_tests.exe: authenticator.o test.o map.o assert.o authenticator_tests.c
	gcc ${CC_FLAGS} authenticator.o test.o map.o assert.o authenticator_tests.c -o authenticator_tests.exe

server_tests.exe: server.o test.o assert.o server_tests.c
	gcc ${CC_FLAGS} -pthread server.o test.o assert.o server_tests.c -o server_tests.exe

## phony targets

compile: authenticator.o test.o map.o assert.o server.o

full_build: test memleak

test: map_tests.exe authenticator_tests.exe server_tests.exe
	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Executing unit tests for map"
	./map_tests.exe

	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Executing unit tests for authenticator"
	./authenticator_tests.exe
	
	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Executing unit tests for server"
	./server_tests.exe

memleak: map_tests.exe authenticator_tests.exe server_tests.exe
	sh ../ci-scripts/memory_leak_check.sh ./map_tests.exe
	sh ../ci-scripts/memory_leak_check.sh ./authenticator_tests.exe
	sh ../ci-scripts/memory_leak_check.sh ./server_tests.exe

sonar: clean
	build-wrapper-linux-x86-64 --out-dir bw-output make compile test
	gcov *.c
	cd ..; sonar-scanner

clean:
	rm -rf bw_output bw-output .scannerwork out.strace
	rm -f *.exe
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno

