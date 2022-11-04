#pragma once

#ifdef _WIN32
#  define LIBPLATEAU_API __stdcall
#  ifdef LIBPLATEAU_BUILD
#    define LIBPLATEAU_EXPORT  __declspec(dllexport)
#  else
#    define LIBPLATEAU_EXPORT  __declspec(dllimport)
#  endif
#else
#  define LIBPLATEAU_API
#  define LIBPLATEAU_EXPORT
#endif
