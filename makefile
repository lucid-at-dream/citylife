#compiler settings
CC = g++
LDLIBS = -lm -lpq -lgdal
#COPT = -O2 -march=native -mtune=native
COPT = -Og
CFLAGS = -g -Wall -std=c++11 -pedantic ${LDLIBS} ${COPT}
CTESTFLAGS = -lgtest

#directory structure
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin
TEST_DIR = tests

#directories with source code to include
CAIROMM = `pkg-config --cflags --libs cairomm-svg-1.0`
GTKMM = `pkg-config --cflags --libs gtkmm-3.0`
INC_DIR = -I${LIB_DIR} -I /usr/include/postgresql ${CAIROMM} ${GTKMM}

#resulting executable filename 
OFILE = gims
OTESTFILE = test_gims

#src file list
HDRFILES = $(shell find ${SRC_DIR} | grep "\\.hpp$$\\|\\.h$$")
SRCFILES = $(shell find ${SRC_DIR} | grep "\\.cpp$$")
#add lex and yacc src files
OBJFILES = ${OBJ_DIR}/Geometry/lex.yy.o ${OBJ_DIR}/Geometry/y.tab.o
OBJFILES += $(addprefix ${OBJ_DIR}/./, $(addsuffix .o, $(shell find ${SRC_DIR} | grep "\\.cpp$$" | cut -d '.' -f1 | cut -d / -f2-)))

#obj files for testing
#add lex and yacc src files
OBJFILESEXCEPTMAIN = ${OBJ_DIR}/Geometry/lex.yy.o ${OBJ_DIR}/Geometry/y.tab.o
OBJFILESEXCEPTMAIN += $(addprefix ${OBJ_DIR}/./, $(addsuffix .o, $(shell find ${SRC_DIR} | grep "\\.cpp$$" | grep -v "${SRC_DIR}/main\\.cpp$$" | cut -d '.' -f1 | cut -d / -f2-)))
OBJTESTFILESTMP = $(addprefix ${TEST_DIR}/${OBJ_DIR}/, $(addsuffix .o, $(shell find ${TEST_DIR} | grep "\\.cpp$$" | cut -d '.' -f1 | cut -d / -f2-)))
OBJTESTFILES = ${OBJTESTFILESTMP} ${OBJFILESEXCEPTMAIN}


INC_DIR += $(addprefix -I, $(shell dirname $$(find ${SRC_DIR} | grep "\\.hpp$$\\|\\.h$$") | sort | uniq) )

all: ${BIN_DIR}/${OFILE}

run: ${BIN_DIR}/${OFILE}
	./${BIN_DIR}/${OFILE}

.IGNORE: format

.SECONDEXPANSION:
#create the wanted object files provided the source files exist.
${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp | $$(@D)/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR}

#link the executable
${BIN_DIR}/${OFILE}: ${OBJFILES} | ${BIN_DIR}
	${CC} ${CFLAGS} $^ -o $@ ${INC_DIR}


#make an executable for the unit tests
test: ${BIN_DIR}/${OTESTFILE} 
${BIN_DIR}/${OTESTFILE}: ${OBJTESTFILES} | ${BIN_DIR}
	${CC} ${CFLAGS} $^ -o $@ ${INC_DIR} ${CTESTFLAGS} -lgtest_main

${TEST_DIR}/${OBJ_DIR}/%.o: ${TEST_DIR}/%.cpp | $$(@D)/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR} ${CTESTFLAGS}

#rules for the yacc and lex generated files
${OBJ_DIR}/Geometry/lex.yy.o: ${SRC_DIR}/Geometry/lex.yy.c ${SRC_DIR}/Geometry/y.tab.c | $$(@D)/
	${CC} ${CFLAGS} -c ${SRC_DIR}/Geometry/lex.yy.c -o $@ ${INC_DIR}

${SRC_DIR}/Geometry/lex.yy.c: ${SRC_DIR}/Geometry/wkt.l
	flex -o $@ $<

${OBJ_DIR}/Geometry/y.tab.o: ${SRC_DIR}/Geometry/y.tab.c | $$(@D)/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR}

${SRC_DIR}/Geometry/y.tab.c: ${SRC_DIR}/Geometry/wkt.y
	yacc -d $< -o $@


#create directories
${OBJ_DIR}%/:
	mkdir -p $@

${TEST_DIR}%/:
	mkdir -p $@

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
	rm -rf ${TEST_DIR}/${OBJ_DIR}
	rm -rf ${OBJ_DIR}
	rm -f ${BIN_DIR}/${OFILE}

