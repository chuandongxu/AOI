#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(WORKHOLDER_LIB)
#  define WORKHOLDER_EXPORT Q_DECL_EXPORT
# else
#  define WORKHOLDER_EXPORT Q_DECL_IMPORT
# endif
#else
# define WORKHOLDER_EXPORT
#endif
