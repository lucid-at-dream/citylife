.PHONY=test clean memleak default compile full_build

## Compilation
CC=gcc
CC_FLAGS= -g -Wall -fprofile-arcs -ftest-coverage --coverage -std=gnu99
LIB_FLAGS=-pthread
LD_LIBS=

## Final executable final
target_bin = auth.exe

## Files listings
src_files += assert
src_files += auth_verbs
src_files += authenticator
src_files += map
src_files += queue
src_files += server
src_files += worker_pool
src_files += jsmn
src_files += requests_resolver

test_files += authenticator_tests
test_files += map_tests
test_files += server_tests
test_files += queue_tests
test_files += worker_pool_tests
test_files += requests_resolver_tests

test_lib += test

src_main = main.c

## Auxiliary variables
OBJECT_FILES = $(patsubst %, %.o, $(src_files))
TEST_EXE_FILES = $(patsubst %, %.exe, $(test_files))

RUN_TEST_TARGETS = $(patsubst %, run_%, $(test_files))
MEMLEAK_TEST_TARGETS = $(patsubst %, memleak_%, $(test_files))

## The default target
default: compile

## Executable compilation target
${target_bin}: ${OBJECT_FILES} ${src_main}
	${CC} ${CC_FLAGS} ${LIB_FLAGS} $^ -o $@ ${LD_LIBS}

## Object file compilation targets

%.o: %.c %.h
	${CC} ${CC_FLAGS} ${LIB_FLAGS} -c $< -o $@ ${LD_LIBS}

## Tests executable file creation targets

%_tests.exe: %_tests.c ${OBJECT_FILES} ${test_lib}.o
	@echo "Building $@"
	${CC} ${CC_FLAGS} ${LIB_FLAGS} $^ -o $@ ${LD_LIBS}

run_%_tests: %_tests.exe
	$(eval TEST_EXE = $(patsubst run_%, %.exe, $@))
	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Executing unit tests for ${TEST_EXE}"
	./${TEST_EXE}

memleak_%_tests: %_tests.exe
	$(eval TEST_EXE = $(patsubst memleak_%, %.exe, $@))
	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Checking memory leaks for ${TEST_EXE}"
	sh ../ci-scripts/memory_leak_check.sh ./${TEST_EXE}

## phony targets

compile: ${OBJECT_FILES} ${target_bin}

docker-image: ${target_bin}
	docker build -t dimension-benders/services/auth:0.0.1 .

full_build: test memleak

test: ${RUN_TEST_TARGETS}
	@echo "Test build is finished"

memleak: ${MEMLEAK_TEST_TARGETS}

sonar: clean
	build-wrapper-linux-x86-64 --out-dir bw-output make compile test
	gcov *.c
	cd ..; sonar-scanner

clean:
	rm -rf bw_output bw-output .scannerwork out.strace
	rm -f *.exe
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno

