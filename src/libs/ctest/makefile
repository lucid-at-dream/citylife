.PHONY=test clean memleak default compile full_build

## Compilation
CC=gcc
CC_FLAGS= -g -Wall -fprofile-arcs -ftest-coverage --coverage -std=gnu99 ${DEBUG}
LIB_FLAGS= -pthread
LD_LIBS=
OUTPUT_PATH=obj

src_files += assert
src_files += test

test_files += 



## Auxiliary variables
OBJECT_FILES = $(patsubst %, %.o, $(src_files))
TEST_EXE_FILES = $(patsubst %, %.exe, $(test_files))

RUN_TEST_TARGETS = $(patsubst %, run_%, $(test_files))
MEMLEAK_TEST_TARGETS = $(patsubst %, memleak_%, $(test_files))



## The default target
default: create_paths build test

create_paths:
	mkdir -p ${OUTPUT_PATH}


## Object file compilation targets
%.o: %.c %.h
	${CC} ${CC_FLAGS} ${LIB_FLAGS} -c $< -o ${OUTPUT_PATH}/$@ ${LD_LIBS}



## phony targets
build: ${OBJECT_FILES}

test: ${RUN_TEST_TARGETS}
	@echo "All tests finished"

sonar: clean
	build-wrapper-linux-x86-64 --out-dir ../bw-output make test build
	gcov *.c

clean:
	rm -rf ${OUTPUT_PATH}
	rm -rf bw_output bw-output .scannerwork out.strace
	rm -f *.exe
	rm -f *.o
	rm -f *.gcov *.gcda *.gcno

