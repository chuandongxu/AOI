#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(DVIEWUTILITY_LIB)
#  define DVIEWUTILITY_EXPORT Q_DECL_EXPORT
# else
#  define DVIEWUTILITY_EXPORT Q_DECL_IMPORT
# endif
#else
# define DVIEWUTILITY_EXPORT
#endif
