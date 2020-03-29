.PHONY=test clean memleak default compile

## Compilation
CC=gcc
CC_FLAGS= -g -Wall -fprofile-arcs -ftest-coverage --coverage -std=gnu99 ${DEBUG}
LIB_FLAGS= -pthread
LD_LIBS=

## Output Directories
libs_dir=../../obj/

## Files listings
src_files += queue/queue
src_files += ctest/assert
src_files += ctest/test

test_files += queue/queue_tests

test_lib += ctest/test



## The default target
default: compile



## Auxiliary variables
OBJECT_FILES = $(patsubst %, ${libs_dir}%.o, $(src_files))
TEST_EXE_FILES = $(patsubst %, %.exe, $(test_files))

RUN_TEST_TARGETS = $(patsubst %, run_%, $(test_files))
MEMLEAK_TEST_TARGETS = $(patsubst %, memleak_%, $(test_files))


## Object file compilation targets
%.o: ${libs_dir}/%.c ${libs_dir}/%.h
	install -D /dev/null $@
	${CC} ${CC_FLAGS} ${LIB_FLAGS} -c $< -o $@ ${LD_LIBS}

## Tests executable file creation targets
%_tests.exe: %_tests.c ${OBJECT_FILES} ${test_lib}.o
	@echo "Building $@"
	${CC} ${CC_FLAGS} ${LIB_FLAGS} $^ -o $@ ${LD_LIBS}



## phony targets

run_%_tests: %_tests.exe
	$(eval TEST_EXE = $(patsubst run_%, %.exe, $@))
	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Executing unit tests for ${TEST_EXE}"
	./${TEST_EXE}

memleak_%_tests: %_tests.exe
	$(eval TEST_EXE = $(patsubst memleak_%, %.exe, $@))
	@echo -e "\n= = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo "Checking memory leaks for ${TEST_EXE}"
	sh ../../ci-scripts/memory_leak_check.sh ./${TEST_EXE}


compile: ${OBJECT_FILES}

full_build: compile test memleak

test: ${RUN_TEST_TARGETS}
	@echo "Test build is finished"

memleak: ${MEMLEAK_TEST_TARGETS}

sonar: clean
	build-wrapper-linux-x86-64 --out-dir ../bw-output make compile test
	gcov *.c

clean:
	rm -rf bw_output bw-output .scannerwork out.strace
	rm -f *.exe
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno

