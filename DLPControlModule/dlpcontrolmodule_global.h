#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(DLPCONTROLMODULE_LIB)
#  define DLPCONTROLMODULE_EXPORT Q_DECL_EXPORT
# else
#  define DLPCONTROLMODULE_EXPORT Q_DECL_IMPORT
# endif
#else
# define DLPCONTROLMODULE_EXPORT
#endif

//#define STATION_COUNT 2
