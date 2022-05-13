%{
    #include "geometry.h"
    #include "dynarray.h"

    void initProgram();
    int yylex(void);

    void yyerror (char *s);
    extern int lineno, colno, tmp_lineno, tmp_colno;
    extern char *yytext;
    extern int yyleng;

    geometry *geom;
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
    geometry *g;
    point *pt;
    line_string *ls;
    polygon *pol;
    multi_point *mpt;
    multi_line_string *mls;
    multi_polygon *mpol;
    geometry_collection *gc;
}

%type <g> start
%type <pt> point_def
%type <ls> linestring_def
%type <pol> polygon_def 
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

point_def: _NUMBERLIT_ _NUMBERLIT_ { $$ = point_new($1, $2); }
         ;

linestring_def: point_def _COMMA_ point_def {
                    line_string *ls = line_string_new();

                    dynarray_add(ls->vertex_list, $1);
                    dynarray_add(ls->vertex_list, $3);

                    $$=ls;
                }
              | linestring_def _COMMA_ point_def { 
                    dynarray_add($1->vertex_list, $3);
                    $$=$1;
                }
              ;

polygon_def : _LPAR_ linestring_def _RPAR_ {
                  polygon *p = polygon_new($2, NULL);
                  $$ = p;
              }
            | _LPAR_ linestring_def _RPAR_ _COMMA_ _LPAR_ multilinestring_def _RPAR_ {
                  polygon *p = polygon_new($2, $6);
                  $$ = p;
              }
            ;

multipoint_def: point_def {
                    multi_point *mp = multi_point_new();
                    dynarray_add(mp->point_list, $1);
                    $$ = mp;
                }
              | _LPAR_ point_def _RPAR_ {
                    multi_point *mp = multi_point_new();
                    dynarray_add(mp->point_list, $2);
                    $$ = mp;
                }
              | multipoint_def _COMMA_ point_def {
                    dynarray_add($1->point_list, $3);
                    $$ = $1;
                }
              | multipoint_def _COMMA_ _LPAR_ point_def _RPAR_ {
                    dynarray_add($1->point_list, $4);
                    $$ = $1;
                }
              ;

multilinestring_def: _LPAR_ linestring_def _RPAR_ {
                         multi_line_string *mls = multi_line_string_new();
                         dynarray_add(mls->line_string_list, $2);
                         $$ = mls;
                     }
                   | multilinestring_def _COMMA_ _LPAR_ linestring_def _RPAR_ {
                         dynarray_add($1->line_string_list, $4);
                         $$ = $1;
                     }
                   ;

multipolygon_def: _LPAR_ polygon_def _RPAR_ {
                      multi_polygon *mp = multi_polygon_new();
                      dynarray_add(multi_polygon->polygon_list, $2);
                      $$ = mp;
                  }
                | multipolygon_def _COMMA_ _LPAR_ polygon_def _RPAR_ {
                      dynarray_add($1->polygon_list, $4);
                      $$ = $1;
                  }
                ;

atomic: _POINT_ _LPAR_ point_def _RPAR_
      {
          geometric_object obj;
          obj.p = $3;
          $$ = geometry_new(POINT, obj);
      }

      | _LINESTRING_ _LPAR_ linestring_def _RPAR_
      {
          geometric_object obj;
          obj.ls = $3;
          $$ = geometry_new(LINESTRING, obj);
      }

      | _POLYGON_ _LPAR_ polygon_def _RPAR_
      {
          geometric_object obj;
          obj.pol = $3;
          $$ = geometry_new(POLYGON, obj);
      }

      | _MULTIPOINT_ _LPAR_ multipoint_def _RPAR_
      {
          geometric_object obj;
          obj.mp = $3;
          $$ = geometry_new(MULTIPOINT, obj);
      }

      | _MULTILINESTRING_ _LPAR_ multilinestring_def _RPAR_
      {
          geometric_object obj;
          obj.mls = $3;
          $$ = geometry_new(MULTILINESTRING, obj);
      }

      | _MULTIPOLYGON_ _LPAR_ multipolygon_def _RPAR_
      {
          geometric_object obj;
          obj.mpol = $3;
          $$ = geometry_new(MULTIPOLYGON, obj);
      };

collection_def: atomic {
                    geometry_collection *c = geometry_collection_new();
                    dynarray_add(c->geometry_list, $1);
                    $$ = c;
                }
              | collection_def _COMMA_ atomic  {
                    dynarray_add($1->geometry_list, $3);
                    $$ = $1;
                }
              ;

%%

void yyerror (char *s) {
    printf ("Line %d, col %d: %s: %s\n", lineno, colno-(int)strlen(yytext), s, yytext);
}

