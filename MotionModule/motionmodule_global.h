#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(MOTIONMODULE_LIB)
#  define MOTIONMODULE_EXPORT Q_DECL_EXPORT
# else
#  define MOTIONMODULE_EXPORT Q_DECL_IMPORT
# endif
#else
# define MOTIONMODULE_EXPORT
#endif
