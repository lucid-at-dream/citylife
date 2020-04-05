#directory structure
PROJECT_ROOT = $(shell git rev-parse --show-toplevel)
OBJ_DIR = ${PROJECT_ROOT}/obj
BIN_DIR = ${PROJECT_ROOT}/bin
TEST_DIR = ${PROJECT_ROOT}/tests


#compiler settings
CC = gcc
LDLIBS = -lm
COPT = -mtune=native -march=native -O2 -pipe -fstack-protector --param=ssp-buffer-size=4 -D_FORTIFY_SOURCE=2
COPT = -Wall -g -Og
CFLAGS = -pthread -fprofile-arcs -ftest-coverage --coverage -std=gnu99 ${LDLIBS} ${COPT}
INC_DIR = -I${OBJ_DIR} -I${PROJECT_ROOT}/src/libs


# SRC and Test files
SRC_FILES = $(shell ls *.c | grep -v "_tests.c$$" | cut -d"." -f1)
TEST_FILES = $(shell ls *.c | grep "_tests.c$$" | cut -d"." -f1)


## Auxiliary variables
OBJECT_FILES = $(patsubst %, ${OBJ_DIR}/%.o, $(SRC_FILES))
TEST_EXE_FILES = $(patsubst %, ${TEST_DIR}/%.exe, $(TEST_FILES))


## Auxiliary targets
default: compile

compile: ${OBJECT_FILES}

compile_tests: ${TEST_EXE_FILES}

clean:
	rm -rf *.gcno


#compile the object files
${OBJ_DIR}/%.o: %.c %.h | ${OBJ_DIR}
	@echo "Building $@"
	${CC} -c $< -o $@ ${INC_DIR} ${CFLAGS}


# Create test executables
${TEST_DIR}/%_tests.exe: %_tests.c | ${TEST_DIR}
	@echo "Building $@"
	${CC} $^ ${OBJ_DIR}/*.o -o $@ ${INC_DIR} ${CFLAGS}



#create directories
${OBJ_DIR}:
	mkdir -p $@

${TEST_DIR}:
	mkdir -p $@

${BIN_DIR}:
	mkdir -p ${BIN_DIR}$*

