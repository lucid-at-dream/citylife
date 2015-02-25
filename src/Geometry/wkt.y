%{
    #include <stdio.h>
    #include <string.h>
    #include "Geometry.hpp"
    #include <iostream>

    using namespace GIMS_GEOMETRY;
    using namespace std;

    void initProgram();
    int yylex(void);

    // int lineno = 1, colno = 1, tmp_lineno, tmp_colno;
    void yyerror (char *s);
    extern int lineno, colno, tmp_lineno, tmp_colno;
    extern char *yytext;
    extern int yyleng;

    GIMS_Geometry *geom; // final geometry
%}

%token _POINT_
%token _LINESTRING_
%token _POLYGON_
%token _MULTIPOINT_
%token _MULTILINESTRING_
%token _MULTIPOLYGON_
%token _COLLECTION_
%token _COMMA_
%token _LPAR_
%token _RPAR_

%token <numberlit> _NUMBERLIT_

%union{
    double numberlit;
    GIMS_Geometry *g;
    GIMS_Point *pt;
    GIMS_LineString *ls;
    GIMS_Polygon *pol;
    GIMS_MultiPoint *mpt;
    GIMS_MultiLineString *mls;
    GIMS_MultiPolygon *mpol;
    GIMS_GeometryCollection *gc;
}

%type <g> start
%type <pt> point_def
%type <ls> linestring_def
%type <pol> polygon_def 
%type <mls> interior_def
%type <mpt> multipoint_def
%type <mls> multilinestring_def
%type <mpol> multipolygon_def
%type <g> atomic
%type <gc> collection_def
%% 

start: _POINT_           _LPAR_ point_def           _RPAR_ {geom = $3;}
     | _LINESTRING_      _LPAR_ linestring_def      _RPAR_ {geom = $3;}
     | _POLYGON_         _LPAR_ polygon_def         _RPAR_ {geom = $3;}
     | _MULTIPOINT_      _LPAR_ multipoint_def      _RPAR_ {geom = $3;}
     | _MULTILINESTRING_ _LPAR_ multilinestring_def _RPAR_ {geom = $3;}
     | _MULTIPOLYGON_    _LPAR_ multipolygon_def    _RPAR_ {geom = $3;}
     | _COLLECTION_      _LPAR_ collection_def      _RPAR_ {geom = $3;}
     ;

point_def: _NUMBERLIT_ _NUMBERLIT_ { $$ = new GIMS_Point($1, $2); }
         ;

linestring_def: point_def _COMMA_ point_def {
                    GIMS_LineString *ls = new GIMS_LineString(2); 
                    ls->appendPoint($1);
                    ls->appendPoint($3);
                    $$=ls;
                }
              | linestring_def _COMMA_ point_def { 
                    $1->appendPoint($3);
                    $$=$1;
                }
              ;

polygon_def : _LPAR_ linestring_def _COMMA_ point_def _RPAR_ {
                  GIMS_MultiLineString *exterior = new GIMS_MultiLineString(1);
                  $2->appendPoint($4);
                  exterior->append($2);
                  $$ = new GIMS_Polygon(exterior, NULL);
              }
            | _LPAR_ linestring_def _COMMA_ point_def _RPAR_ _COMMA_ interior_def {
                  GIMS_MultiLineString *exterior = new GIMS_MultiLineString(1);
                  $2->appendPoint($4);
                  exterior->append($2);
                  $$ = new GIMS_Polygon(exterior, $7);
              }
            ;

interior_def: _LPAR_ linestring_def _COMMA_ point_def _RPAR_ {
                  GIMS_MultiLineString *interior = new GIMS_MultiLineString(1);
                  $2->appendPoint($4);
                  interior->append($2);
                  $$ = interior;
              }
            | _LPAR_ linestring_def _COMMA_ point_def _RPAR_ _COMMA_ interior_def {
                  $2->appendPoint($4);
                  $7->append($2);
                  $$ = $7;
              }
            ;

multipoint_def: point_def {
                    GIMS_MultiPoint *mp = new GIMS_MultiPoint(1);
                    mp->append($1);
                    $$ = mp;
                }
              | _LPAR_ point_def _RPAR_ {
                    GIMS_MultiPoint *mp = new GIMS_MultiPoint(1);
                    mp->append($2);
                    $$ = mp;
                }
              | point_def _COMMA_ multipoint_def {
                    $3->append($1);
                    $$ = $3;
                }
              | _LPAR_ point_def _RPAR_ _COMMA_ multipoint_def {
                    $5->append($2);
                    $$ = $5;
                }
              ;

multilinestring_def: _LPAR_ linestring_def _RPAR_ {
                         GIMS_MultiLineString *mls = new GIMS_MultiLineString(1);
                         mls->append($2);
                         $$ = mls;
                     }
                   | _LPAR_ linestring_def _RPAR_ _COMMA_ multilinestring_def {
                         $5->append($2);
                         $$ = $5;
                     }
                   ;

multipolygon_def: _LPAR_ polygon_def _RPAR_ {
                      GIMS_MultiPolygon *mp = new GIMS_MultiPolygon(1);
                      mp->append($2);
                      $$ = mp;
                  }
                | _LPAR_ polygon_def _RPAR_ _COMMA_ multipolygon_def {
                      $5->append($2);
                      $$ = $5;
                  }
                ;

atomic: _POINT_           _LPAR_ point_def           _RPAR_ {$$ = $3;}
      | _LINESTRING_      _LPAR_ linestring_def      _RPAR_ {$$ = $3;}
      | _POLYGON_         _LPAR_ polygon_def         _RPAR_ {$$ = $3;}
      | _MULTIPOINT_      _LPAR_ multipoint_def      _RPAR_ {$$ = $3;}
      | _MULTILINESTRING_ _LPAR_ multilinestring_def _RPAR_ {$$ = $3;}
      | _MULTIPOLYGON_    _LPAR_ multipolygon_def    _RPAR_ {$$ = $3;}
      ;

collection_def: atomic {
                    GIMS_GeometryCollection *c = new GIMS_GeometryCollection(1);
                    c->append($1);
                    $$ = c;
                }
              | atomic _COMMA_ collection_def {
                    $3->append($1);
                    $$ = $3;
                }
              ;

%%

void yyerror (char *s) {
    printf ("Line %d, col %d: %s: %s\n", lineno, colno-(int)strlen(yytext), s, yytext);
}

int main(int argc, char **argv ) {

    if( !yyparse() ){
        cout << geom->toWkt() << endl;
    }

    return 0;
}
