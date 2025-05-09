
all: solve run

PROJECT_DIR=../../..

${PROJECT_DIR}/common/seq_align.o: ${PROJECT_DIR}/common/seq_align.c ${PROJECT_DIR}/common/seq_align.h
	gcc -c -Werror -Wall -O2 -g ${PROJECT_DIR}/common/seq_align.c -o ../common/seq_align.o

solve: solver.c ${PROJECT_DIR}/common/seq_align.o
	gcc solver.c ${PROJECT_DIR}/common/seq_align.o -I${PROJECT_DIR}/common  -Wall -O2 -g -Werror -o solve

run: solve
	for f in $(wildcard ${PROJECT_DIR}/problems/*); do\
		valgrind -s --show-leak-kinds=all --leak-check=full --track-origins=yes ./solve $$f; \
	done

clean:
	rm solve
