#ifndef VISIONUI_GLOBAL_H
#define VISIONUI_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef VISIONUI_LIB
# define VISIONUI_EXPORT Q_DECL_EXPORT
#else
# define VISIONUI_EXPORT Q_DECL_IMPORT
#endif

#endif // VISIONUI_GLOBAL_H
