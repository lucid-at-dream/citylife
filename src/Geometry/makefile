LD = -I.. -std=c++11 -g

all: BoundingBox.o GeometryCollection.o GIMSGeometry.o LineString.o Point.o Polygon.o y.tab.o lex.yy.o
	g++ $^ -o t -ly ${LD} 

y.tab.o: lex.yy.c y.tab.c
	g++ -c y.tab.c ${LD}

y.tab.c: wkt.y
	yacc -d -v wkt.y

lex.yy.o: lex.yy.c y.tab.c
	g++ -c lex.yy.c ${LD} 

lex.yy.c: wkt.l
	flex wkt.l

BoundingBox.o:
	g++ -c BoundingBox.cpp ${LD} 

GeometryCollection.o:
	g++ -c GeometryCollection.cpp ${LD}

GIMSGeometry.o:
	g++ -c GIMSGeometry.cpp ${LD}

LineString.o:
	g++ -c LineString.cpp ${LD}

Point.o:
	g++ -c Point.cpp ${LD}

Polygon.o:
	g++ -c Polygon.cpp ${LD}

clean:
	rm y.tab.c lex.yy.c y.tab.h y.output *.o *.gch
