LD = -I.. -std=c++11 -g -Wall -lm -pedantic

all: BoundingBox.o GeometryCollection.o GIMSGeometry.o LineString.o Point.o Polygon.o y.tab.o lex.yy.o BentleySolver.o main.o
	g++ $^ -o t -ly ${LD} 

main.o: main.cpp
	g++ -c main.cpp ${LD}

BentleySolver.o: BentleySolver.cpp BentleySolver.hpp
	g++ -c BentleySolver.cpp ${LD}

y.tab.o: lex.yy.c y.tab.c
	g++ -c y.tab.c ${LD} -ly

y.tab.c: wkt.y
	yacc -d wkt.y

lex.yy.o: lex.yy.c y.tab.c
	g++ -c lex.yy.c ${LD} -ly

lex.yy.c: wkt.l
	flex wkt.l

Geometry.hpp: lex.yy.c y.tab.c

BoundingBox.o: BoundingBox.cpp Geometry.hpp
	g++ -c BoundingBox.cpp ${LD} 

GeometryCollection.o: GeometryCollection.cpp Geometry.hpp
	g++ -c GeometryCollection.cpp ${LD}

GIMSGeometry.o: GIMSGeometry.cpp Geometry.hpp
	g++ -c GIMSGeometry.cpp ${LD}

LineString.o: LineString.cpp Geometry.hpp
	g++ -c LineString.cpp ${LD}

Point.o: Point.cpp Geometry.hpp
	g++ -c Point.cpp ${LD}

Polygon.o: Polygon.cpp Geometry.hpp
	g++ -c Polygon.cpp ${LD}

debug: setdebug all
setdebug:
	$(eval LD += -DDEBUG)

clean:
	rm y.tab.c lex.yy.c y.tab.h y.output *.o *.gch
