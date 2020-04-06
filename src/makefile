#directory structure
PROJECT_ROOT = $(shell git rev-parse --show-toplevel)
OBJ_DIR = ${PROJECT_ROOT}/obj
BIN_DIR = ${PROJECT_ROOT}/bin
TEST_DIR = ${PROJECT_ROOT}/tests

compile:
	make -C libs compile

test:
	make -C libs test
	
clean:
	make -C libs clean
