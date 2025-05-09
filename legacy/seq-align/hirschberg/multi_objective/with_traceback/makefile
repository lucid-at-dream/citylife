
all: solve run

PROJECT_DIR=../../..
CC_FLAGS=-O2 -Wall -Werror -g

${PROJECT_DIR}/common/seq_align.o: ${PROJECT_DIR}/common/seq_align.c ${PROJECT_DIR}/common/seq_align.h
	gcc -c ${CC_FLAGS} ${PROJECT_DIR}/common/seq_align.c -o ${PROJECT_DIR}/common/seq_align.o

${PROJECT_DIR}/common/list.o: ${PROJECT_DIR}/common/list.c ${PROJECT_DIR}/common/list.h
	gcc -c ${CC_FLAGS} ${PROJECT_DIR}/common/list.c -o ${PROJECT_DIR}/common/list.o

hirsch.o: hirsch.c hirsch.h
	gcc -c hirsch.c ${CC_FLAGS} -I${PROJECT_DIR}/common -o hirsch.o

solve: solver.c ${PROJECT_DIR}/common/seq_align.o ${PROJECT_DIR}/common/list.o hirsch.o
	gcc solver.c ${PROJECT_DIR}/common/list.o ${PROJECT_DIR}/common/seq_align.o hirsch.o -I${PROJECT_DIR}/common ${CC_FLAGS} -o solve

run: solve
	for f in $(wildcard ${PROJECT_DIR}/problems/*); do\
		valgrind -s --show-leak-kinds=all --leak-check=full --track-origins=yes ./solve $$f; \
	done

clean:
	rm solve
