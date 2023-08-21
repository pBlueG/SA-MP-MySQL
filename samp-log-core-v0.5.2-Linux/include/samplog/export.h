//NOTE: Passing "-fvisibility=hidden" as a compiler option to GCC is advised!

#if defined _WIN32 || defined __CYGWIN__
# ifdef __GNUC__
#  ifdef IN_LOGCORE_PLUGIN
#   define DLL_PUBLIC __attribute__ ((dllexport))
#  else
#   define DLL_PUBLIC __attribute__ ((dllimport))
#  endif
# else
#  ifdef IN_LOGCORE_PLUGIN
#   define DLL_PUBLIC __declspec(dllexport)
#  else
#   define DLL_PUBLIC __declspec(dllimport)
#  endif
# endif
#else
# if __GNUC__ >= 4
#  define DLL_PUBLIC __attribute__ ((visibility ("default")))
# else
#  define DLL_PUBLIC
# endif
#endif
