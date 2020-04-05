SRC_FOLDERS = $(shell ls -d */)

.PHONY=compile ${SRC_FOLDERS}

compile: TARGET=compile
compile: ${SRC_FOLDERS}

compile_tests: TARGET = compile_tests
compile_tests: ${SRC_FOLDERS}

clean: TARGET = clean
clean: ${SRC_FOLDERS}

%/: FORCE
	make -C $@ ${TARGET}

FORCE: ;

