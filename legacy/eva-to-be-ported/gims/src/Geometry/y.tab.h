/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SRC_GEOMETRY_Y_TAB_H_INCLUDED
#define YY_YY_SRC_GEOMETRY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype {
    _POINT_ = 258,
    _LINESTRING_ = 259,
    _POLYGON_ = 260,
    _MULTIPOINT_ = 261,
    _MULTILINESTRING_ = 262,
    _MULTIPOLYGON_ = 263,
    _COLLECTION_ = 264,
    _COMMA_ = 265,
    _LPAR_ = 266,
    _RPAR_ = 267,
    _NUMBERLIT_ = 268
};
#endif
/* Tokens.  */
#define _POINT_ 258
#define _LINESTRING_ 259
#define _POLYGON_ 260
#define _MULTIPOINT_ 261
#define _MULTILINESTRING_ 262
#define _MULTIPOLYGON_ 263
#define _COLLECTION_ 264
#define _COMMA_ 265
#define _LPAR_ 266
#define _RPAR_ 267
#define _NUMBERLIT_ 268

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED

union YYSTYPE {
#line 35 "src/Geometry/wkt.y" /* yacc.c:1909  */

    double numberlit;
    GIMS_Geometry *g;
    GIMS_Point *pt;
    GIMS_LineString *ls;
    GIMS_Polygon *pol;
    GIMS_MultiPoint *mpt;
    GIMS_MultiLineString *mls;
    GIMS_MultiPolygon *mpol;
    GIMS_GeometryCollection *gc;

#line 92 "src/Geometry/y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

int yyparse(void);

#endif /* !YY_YY_SRC_GEOMETRY_Y_TAB_H_INCLUDED  */
