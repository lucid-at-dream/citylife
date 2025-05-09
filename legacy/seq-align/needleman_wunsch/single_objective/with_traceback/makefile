
all: solve run

PROJECT_DIR=../../..
CC_FLAGS=-O2 -Wall -Werror -g

${PROJECT_DIR}/common/seq_align.o: ${PROJECT_DIR}/common/seq_align.c ${PROJECT_DIR}/common/seq_align.h
	gcc -c ${CC_FLAGS} ${PROJECT_DIR}/common/seq_align.c -o ${PROJECT_DIR}/common/seq_align.o

nmw.o: nmw.c nmw.h
	gcc -c nmw.c ${CC_FLAGS} -I${PROJECT_DIR}/common -o nmw.o

solve: solver.c ${PROJECT_DIR}/common/seq_align.o nmw.o
	gcc solver.c ${PROJECT_DIR}/common/seq_align.o nmw.o -I${PROJECT_DIR}/common ${CC_FLAGS} -o solve

run: solve
	for f in $(wildcard ${PROJECT_DIR}/problems/*); do\
		valgrind -s --show-leak-kinds=all --leak-check=full --track-origins=yes ./solve $$f; \
	done

clean:
	rm solve
