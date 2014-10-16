#compiler settings
CC = g++
LDLIBS = -lm -lpq -lgdal
COPT = -O2 -march=native -mtune=native
CFLAGS = -g -Wall -std=c++11 ${LDLIBS} ${COPT}

#directories with source code to include
INC_DIR = -I ${SRC_DIR} -I ${LIB_DIR} -I /usr/include/postgresql

#directory structure
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin

#resulting executable filename 
OFILE = gims

#src file list
HDRFILES = $(shell find ${SRC_DIR} | grep "\\.hpp$$\\|\\.h$$")
SRCFILES = $(shell find ${SRC_DIR} | grep "\\.cpp$$")
OBJFILES = $(addprefix ${OBJ_DIR}/./, $(addsuffix .o, $(shell find ${SRC_DIR} | grep "\\.cpp$$" | cut -d '.' -f1 | cut -d / -f2-)))

INC_DIR += $(addprefix -I, $(shell dirname $$(find ${SRC_DIR} | grep "\\.hpp$$\\|\\.h$$") | sort | uniq) )

all: cleanformat ${BIN_DIR}/${OFILE}

.IGNORE: format

.SECONDEXPANSION:
#create the wanted object files provided the source files exist.
${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp | $$(@D)/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR}


#link the executable
${BIN_DIR}/${OFILE}: ${OBJFILES} | ${BIN_DIR}
	${CC} ${CFLAGS} $^ -o $@

#create directories
${OBJ_DIR}%/:
	mkdir -p $@

#create directories
${BIN_DIR}:
	mkdir -p ${BIN_DIR}$*

#formats all code files using astyle tool and cleans up .orig files.
cleanformat: format softclean

#formats all code files using astyle tool.
format:
	astyle --recursive "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.hpp" --quiet -X

#compiles with SUPRESS_NOTES flag defined
quiet: setquiet ${BIN_DIR}/${OFILE}
setquiet:
	$(eval CFLAGS += -DSUPRESS_NOTES)

#compiles with DEBUG flag defined
debug: setdebug ${BIN_DIR}/${OFILE}
#sets a DEBUG flag (as if defined to 1 in the source code) for every source file.
setdebug:
	$(eval CFLAGS += -DDEBUG)

softclean:
	$(eval ORIG_FILES = $(shell find src/ | grep \\.orig))
	rm -f ${ORIG_FILES}

#remove garbage
clean: softclean
	rm -rf ${OBJ_DIR}
	rm -f ${BIN_DIR}/${OFILE}

