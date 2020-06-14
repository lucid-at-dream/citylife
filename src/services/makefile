SRC_FOLDERS = $(shell ls -d */)

.PHONY=compile ${SRC_FOLDERS}

compile: TARGET=compile
compile: ${SRC_FOLDERS}

test: TARGET = test
test: ${SRC_FOLDERS}

clean: TARGET = clean
clean: ${SRC_FOLDERS}

%/: FORCE
	make -C $@ ${TARGET}

FORCE: ;

