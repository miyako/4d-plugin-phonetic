#ifndef __STDAFX_H_INCLUDED__
#define __STDAFX_H_INCLUDED__

//#define UNICODE
//#define _UNICODE
//#define _WIN32_WINNT 0x0501
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
//#define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif

#include <windows.h>
#include <windowsx.h>
#include <crtdbg.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <Dbghelp.h>
#include <locale.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "oleaut32.lib")

#ifndef numberof
#define numberof(X)		((sizeof(X)) / (sizeof((X)[0])))
#endif

#ifndef RESERVED
#define RESERVED 			0
#endif

#ifdef _DEBUG
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif

#define RELEASE(X)	do {																						\
	if( X ) {																													\
		int ref_count = (X)->Release();																	\
		_RPTF1(_CRT_WARN, #X "->Release() returned %d.\n", ref_count);	\
		(X) = NULL;																											\
	}																																	\
} while( 0 )

#endif

