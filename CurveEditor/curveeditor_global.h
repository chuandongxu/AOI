#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(CURVEEDITOR_LIB)
#  define CURVEEDITOR_EXPORT Q_DECL_EXPORT
# else
#  define CURVEEDITOR_EXPORT Q_DECL_IMPORT
# endif
#else
# define CURVEEDITOR_EXPORT
#endif
