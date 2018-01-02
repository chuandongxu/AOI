// Copyright (C) 2010, SafeNet, Inc. All rights reserved.

#ifndef ERRORPRINTER_H
#define ERRORPRINTER_H

#include <map>

class ErrorPrinter
{
public:
    ErrorPrinter();
    void printError(dogStatus status);
    const char* getError(dogStatus status);
    typedef std::map<dogStatus, const char*> StatusMap;

protected:
    StatusMap errorMap;
};

#endif // ERRORPRINTER_H
