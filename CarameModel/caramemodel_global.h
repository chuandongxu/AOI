#ifndef CARAMEMODEL_GLOBAL_H
#define CARAMEMODEL_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef CARAMEMODEL_LIB
# define CARAMEMODEL_EXPORT Q_DECL_EXPORT
#else
# define CARAMEMODEL_EXPORT Q_DECL_IMPORT
#endif

// �Ӿ�ģ������
#define ERROR_GRABIMAGE             0x60030001


#define MSG_ERROR_GRABIMAGE         QStringLiteral("��ȡͼ��ʧ��")

const int DLP_SEQ_PATTERN_IMG_NUM = 12;

#endif // CARAMEMODEL_GLOBAL_H
