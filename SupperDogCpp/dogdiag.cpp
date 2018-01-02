////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dogdiag.h"


// only include when in debug mode
#if defined(DIAGNOSTICS) && defined(_DEBUG)
#if defined(WIN32) || defined(WIN64)
    
    // switch on memory checking
    const int _daigInit = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|
                                         _CRTDBG_CHECK_ALWAYS_DF|
                                         _CRTDBG_LEAK_CHECK_DF);

    ////////////////////////////////////////////////////////////////
    //
    //
    //
    ////////////////////////////////////////////////////////////////
    bool pascal DIAGAssertFailedLine(const char* lpszFileName, 
                                     int nLine)
    {
        // next is lend from MFC :)
        MSG msg;
        BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
        BOOL bResult = _CrtDbgReport(_CRT_ASSERT, 
                                     lpszFileName, 
                                     nLine, 
                                     NULL, 
                                     NULL);
        if (bQuit)
            PostQuitMessage((int)msg.wParam);
    
        return (0 != bResult);
    }

    ////////////////////////////////////////////////////////////////
    //
    //
    //
    ////////////////////////////////////////////////////////////////
    void pascal DIAGOutputString(const char* lpsz)
    {
        ::OutputDebugStringA(lpsz);
    }

#endif // WIN32 || WIN64
#endif // DIAGNOSTICS && _DEBUG
