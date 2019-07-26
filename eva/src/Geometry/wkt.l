%{
  #include <cstdlib>
  #include <ctype.h>
  #include <cstring>

  #include "Geometry.hpp"
  using namespace GIMS_GEOMETRY;

  #include "y.tab.h"

  extern GIMS_Geometry *geom;

  int lineno = 1, colno = 1, tmp_lineno, tmp_colno;
%}

%%

"POINT"                       colno += strlen(yytext); return _POINT_;
"LINESTRING"                  colno += strlen(yytext); return _LINESTRING_;
"POLYGON"                     colno += strlen(yytext); return _POLYGON_;
"MULTIPOINT"                  colno += strlen(yytext); return _MULTIPOINT_;
"MULTILINESTRING"             colno += strlen(yytext); return _MULTILINESTRING_;
"MULTIPOLYGON"                colno += strlen(yytext); return _MULTIPOLYGON_;
"GEOMETRYCOLLECTION"          colno += strlen(yytext); return _COLLECTION_;
","                           colno += strlen(yytext); return _COMMA_;
"("                           colno += strlen(yytext); return _LPAR_;
")"                           colno += strlen(yytext); return _RPAR_;

[-+]?[0-9]*\.?[0-9]+          yylval.numberlit=atof(yytext); colno += strlen(yytext); return _NUMBERLIT_;

<<EOF>>                     return 0;

[ \t]                       colno++; /* skip whitespace */
\n                          lineno++; colno=1; /* skip whitespace */
.                           printf("Line %d, col %d: illegal character ('%s')\n", lineno, colno, yytext); colno++;

%%

GIMS_Geometry *lyWktParse(char *wkt){
    YY_BUFFER_STATE buff = yy_scan_string(wkt);
    yyparse();
    yy_delete_buffer(buff);
    return geom;
}

int yywrap(void) 
{
  return 1;
}
