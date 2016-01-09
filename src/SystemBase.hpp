#ifndef SYSTEMBASE_HPP
#define SYSTEMBASE_HPP

#include <cstddef>
#include <cstdio>
#include <cstdlib>

/*define a debug macro that can be easily deactivated*/
#ifdef SUPRESS_NOTES
#  define TODO(x)
#else
#  define DO_PRAGMA(x) _Pragma (#x)
#  define TODO(x) DO_PRAGMA(message ("TODO - " #x))
#endif

/*define the error margin (comparations will use this value)*/
#define ERR_MARGIN 1e-6

#define MAX(a,b)( ((a)>(b) ? (a) : (b)) )
#define MIN(a,b)( ((a)<(b) ? (a) : (b)) )

#endif
