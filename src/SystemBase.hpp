

#ifdef SUPRESS_NOTES
#  define TODO(x)
#else
#  define DO_PRAGMA(x) _Pragma (#x)
#  define TODO(x) DO_PRAGMA(message ("TODO - " #x))
#endif
