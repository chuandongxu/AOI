////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#if !defined(__DOG_DIAGNOSTICS_H__)
#define __DOG_DIAGNOSTICS_H__

#if defined(DIAGNOSTICS) && defined(_DEBUG)
#if defined(WIN32) || defined(WIN64)
extern const int _diagInit;

// exclude rarely used window stuff
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>

bool pascal DIAGAssertFailedLine(const char* lpszFileName, int nLine);
void pascal DIAGOutputString(const char* lpsz);

#if !defined(DIAG_ASSERT)
#pragma warning(disable: 4127)
#define DIAG_ASSERT(exp) \
	do \
{ \
	if (!(exp) && DIAGAssertFailedLine(__FILE__, __LINE__)) \
	_CrtDbgBreak(); \
} while (0)
#endif // DIAG_ASSERT

#if !defined(DIAG_VERIFY)
#define DIAG_VERIFY(exp)            DIAG_ASSERT(exp)
#endif // !DIAG_VERIFY

#if !defined(DIAG_TRACE)
#define DIAG_TRACE(exp)             DIAGOutputString((exp))
#endif // DIAG_TRACE

#if !defined(DIAG_NEW)
#define DIAG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif //!DIAG_NEW
#else
#include <assert.h>
#include <stdio.h>

#if !defined(DIAG_ASSERT)
#define DIAG_ASSERT(exp)            assert((exp))
#endif // DIAG_ASSERT

#if !defined(DIAG_VERIFY)
#define DIAG_VERIFY(exp)            DIAG_ASSERT(exp)
#endif // !DIAG_VERIFY

#if !defined(DIAG_TRACE)
#define DIAG_TRACE(exp)             fprintf(stderr, "%s", (exp))
#endif // DIAG_TRACE

#if !defined(DIAG_NEW)
#define DIAG_NEW new
#endif //!DIAG_NEW
#endif // WIN32 || WIN64
#else
#if !defined(DIAG_ASSERT)
#define DIAG_ASSERT(exp) ((void)(0))
#endif // DIAG_ASSERT

#if !defined(DIAG_TRACE)
#define DIAG_TRACE(exp) ((void)(0))
#endif // DIAG_TRACE

#if !defined(DIAG_VERIFY)
#define DIAG_VERIFY(exp) ((void)(exp))
#endif // DIAG_VERIFY

#if !defined(DIAG_NEW)
#define DIAG_NEW new
#endif // DIAG_NEW
#endif // DIAGNOSTICS && _DEBUG

#endif // !__DOG_DIAGNOSTICS_H__
