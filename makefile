#compiler settings
CC = /usr/lib/ccache/bin/g++
LDLIBS = -lm -lpq -g -Wall
COPT = -march=native -O2 -pipe -fstack-protector --param=ssp-buffer-size=4 -D_FORTIFY_SOURCE=2
CFLAGS = -std=c++11 ${LDLIBS} ${COPT}
CTESTFLAGS = -lgtest -lgtest_main

#directory structure
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

#resulting executable filename 
OFILE = gims
OTESTFILE = test_gims

#location of the source file for lex
LEX_FILE = ${SRC_DIR}/Geometry/lex.yy.c

#location of the source file for yacc
YACC_FILE = ${SRC_DIR}/Geometry/y.tab.c

#the source directory list
INC_DIR  = -I ${SRC_DIR}
INC_DIR += -I ${SRC_DIR}/DBConnection
INC_DIR += -I ${SRC_DIR}/DataProcessor
INC_DIR += -I ${SRC_DIR}/Debugger
INC_DIR += -I ${SRC_DIR}/Geometry
INC_DIR += -I /usr/include/postgresql
INC_DIR += `pkg-config --cflags --libs cairomm-svg-1.0`
INC_DIR += `pkg-config --cflags --libs gtkmm-3.0`

#source file with main function
OBJMAIN = ${OBJ_DIR}/main.o

#object files
OBJFILES += ${OBJ_DIR}/DataProcessor/avl.o
OBJFILES += ${OBJ_DIR}/DataProcessor/DE9IM.o
OBJFILES += ${OBJ_DIR}/DataProcessor/PMQuadTree.o
OBJFILES += ${OBJ_DIR}/DBConnection/DBConnection.o
OBJFILES += ${OBJ_DIR}/Debugger/DebRender.o
OBJFILES += ${OBJ_DIR}/Geometry/BoundingBox.o
OBJFILES += ${OBJ_DIR}/Geometry/GIMSGeometry.o
OBJFILES += ${OBJ_DIR}/Geometry/GeometryCollection.o
OBJFILES += ${OBJ_DIR}/Geometry/LineString.o
OBJFILES += ${OBJ_DIR}/Geometry/Point.o
OBJFILES += ${OBJ_DIR}/Geometry/Polygon.o
OBJFILES += ${OBJ_DIR}/Geometry/lex.yy.o
OBJFILES += ${OBJ_DIR}/Geometry/y.tab.o
OBJFILES += ${OBJ_DIR}/Geometry/BentleySolver.o

#object files for testing source code
OBJTESTFILES = ${TEST_DIR}/${OBJ_DIR}/pointInPolygonTest.o


all: ${YACC_FILE} ${LEX_FILE} ${BIN_DIR}/${OFILE}

run: ${BIN_DIR}/${OFILE}
	./${BIN_DIR}/${OFILE}

#begin
#create the auto generated source code for lex and yacc
${SRC_DIR}/Geometry/y.tab.c: ${SRC_DIR}/Geometry/wkt.y
	yacc -d $< -o $@

${SRC_DIR}/Geometry/lex.yy.c: ${SRC_DIR}/Geometry/wkt.l
	flex -o $@ $<
#end

#target that should be called for the test executable
test: ${BIN_DIR}/${OTESTFILE} 

#make an executable for the unit tests
${BIN_DIR}/${OTESTFILE}: ${OBJTESTFILES} ${OBJFILES} | ${BIN_DIR}
	${CC} ${CFLAGS} $^ -o $@ ${INC_DIR} ${CTESTFLAGS}

#create object files for the unit tests
${TEST_DIR}/${OBJ_DIR}/%.o: ${TEST_DIR}/%.cpp | ${TEST_DIR}/${OBJ_DIR}/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR} ${CTESTFLAGS}


.SECONDEXPANSION:
#create the executable
${BIN_DIR}/${OFILE}: ${OBJFILES} ${OBJMAIN} | ${BIN_DIR}
	${CC} ${CFLAGS} $^ -o $@ ${INC_DIR}

#compile the object files
${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp | $$(@D)/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR}

#begin
#targets for the yacc and lex generated files
${OBJ_DIR}/Geometry/lex.yy.o: ${SRC_DIR}/Geometry/lex.yy.c ${SRC_DIR}/Geometry/y.tab.c | $$(@D)/
	${CC} ${CFLAGS} -c ${SRC_DIR}/Geometry/lex.yy.c -o $@ ${INC_DIR} -ly

${OBJ_DIR}/Geometry/y.tab.o: ${SRC_DIR}/Geometry/y.tab.c | $$(@D)/
	${CC} ${CFLAGS} -c $< -o $@ ${INC_DIR} -ly
#end

#create directories
${OBJ_DIR}%/:
	mkdir -p $@

${TEST_DIR}%/:
	mkdir -p $@

${BIN_DIR}:
	mkdir -p ${BIN_DIR}$*

#compiles with SUPRESS_NOTES flag defined
quiet: setquiet ${BIN_DIR}/${OFILE}
setquiet:
	$(eval CFLAGS += -DSUPRESS_NOTES)

#compiles with DEBUG flag defined
debug: setdebug ${BIN_DIR}/${OFILE}
#sets a DEBUG flag (as if defined in the source code) for every source file.
setdebug:
	$(eval CFLAGS += -DDEBUG)

#remove binaries
clean:
	rm -rf ${TEST_DIR}/${OBJ_DIR}
	rm -rf ${OBJ_DIR}
	rm -f ${BIN_DIR}/${OFILE}

