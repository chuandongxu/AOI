#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(VISIONTEST_LIB)
#  define VISIONTEST_EXPORT Q_DECL_EXPORT
# else
#  define VISIONTEST_EXPORT Q_DECL_IMPORT
# endif
#else
# define VISIONTEST_EXPORT
#endif
