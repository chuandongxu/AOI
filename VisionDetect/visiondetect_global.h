#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(VISIONDETECT_LIB)
#  define VISIONDETECT_EXPORT Q_DECL_EXPORT
# else
#  define VISIONDETECT_EXPORT Q_DECL_IMPORT
# endif
#else
# define VISIONDETECT_EXPORT
#endif

//#define STATION_COUNT 2