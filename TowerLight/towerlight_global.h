#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(TOWERLIGHT_LIB)
#  define TOWERLIGHT_EXPORT Q_DECL_EXPORT
# else
#  define TOWERLIGHT_EXPORT Q_DECL_IMPORT
# endif
#else
# define TOWERLIGHT_EXPORT
#endif
