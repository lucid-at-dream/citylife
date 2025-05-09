
all: solve run

PROJECT_DIR=../..
NMW_DIR=../../needleman_wunsch/single_objective/with_traceback
CC_FLAGS=-Werror -Wall -O2

${NMW_DIR}/nmw.o: ${NMW_DIR}/nmw.c ${NMW_DIR}/nmw.h
	gcc -c ${NMW_DIR}/nmw.c ${CC_FLAGS} -I${PROJECT_DIR}/common -o ${NMW_DIR}/nmw.o

${PROJECT_DIR}/common/seq_align.o: ${PROJECT_DIR}/common/seq_align.c ${PROJECT_DIR}/common/seq_align.h
	gcc -c ${CC_FLAGS} ${PROJECT_DIR}/common/seq_align.c -o ${PROJECT_DIR}/common/seq_align.o

solve: solver.c ${PROJECT_DIR}/common/seq_align.o ${NMW_DIR}/nmw.o
	gcc solver.c ${PROJECT_DIR}/common/seq_align.o ${NMW_DIR}/nmw.o -I${PROJECT_DIR}/common -I${NMW_DIR} ${CC_FLAGS} -o solve

run: solve
	for f in $(wildcard ${PROJECT_DIR}/problems/*); do\
		valgrind -s --leak-check=full --track-origins=yes ./solve $$f; \
	done

clean:
	rm solve
