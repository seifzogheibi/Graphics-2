#ifdef _MSC_VER
#pragma warning ( disable : 4820 4365 4668 5039 4710 4267 4996 4505 5045 4711 4702)
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN 1
#	include <windows.h>
#endif

#if defined(__GNUC__)
#	pragma GCC diagnostic ignored "-Wconversion"
#	pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define FONTSTASH_IMPLEMENTATION 1
#include <fontstash.h>
