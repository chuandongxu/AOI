#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(DATAMODULE_LIB)
#  define DATAMODULE_EXPORT Q_DECL_EXPORT
# else
#  define DATAMODULE_EXPORT Q_DECL_IMPORT
# endif
#else
# define DATAMODULE_EXPORT
#endif

#define STATION_COUNT 2
