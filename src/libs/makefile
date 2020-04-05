.PHONY=default compile compile_tests

#compiler settings
CC = gcc
LDLIBS = -lm
COPT = -mtune=native -march=native -O2 -pipe -fstack-protector --param=ssp-buffer-size=4 -D_FORTIFY_SOURCE=2
COPT = -Wall -g -Og
CFLAGS = -pthread -fprofile-arcs -ftest-coverage --coverage -std=gnu99 ${LDLIBS} ${COPT}

#directory structure
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

## Files listings
SRC_FILES += queue/queue
SRC_FILES += ctest/assert
SRC_FILES += ctest/test

TEST_FILES += queue/queue_tests

INC_DIR += -I .

## Auxiliary variables
OBJECT_FILES = $(patsubst %, ${OBJ_DIR}/%.o, $(SRC_FILES))
TEST_EXE_FILES = $(patsubst %, ${TEST_DIR}/%.exe, $(TEST_FILES))

RUN_TEST_TARGETS = $(patsubst %, run_%, $(TEST_FILES))
MEMLEAK_TEST_TARGETS = $(patsubst %, memleak_%, $(TEST_FILES))

## The default target
default: compile

compile: ${OBJECT_FILES}

compile_tests: ${TEST_EXE_FILES}

clean:
	rm -rf ${OBJ_DIR}
	rm -rf ${TEST_DIR}
	rm -rf ${BIN_DIR}

.SECONDEXPANSION:

#compile the object files
${OBJ_DIR}/%.o: %.c %.h | $$(@D)/
	@echo "Building $@"
	${CC} -c $< -o $@ ${INC_DIR} ${CFLAGS}
	$(info )
	$(info )

# Create test executables
${TEST_DIR}/%_tests.exe: %_tests.c ${OBJ_DIR}/%.o ${OBJ_DIR}/ctest/test.o ${OBJ_DIR}/ctest/assert.o | $$(@D)/
	@echo "Building $@"
	${CC} $^ -o $@ ${INC_DIR} ${CFLAGS}


#create directories
${OBJ_DIR}%/:
	mkdir -p $@

${TEST_DIR}%/:
	mkdir -p $@

${BIN_DIR}:
	mkdir -p ${BIN_DIR}$*

