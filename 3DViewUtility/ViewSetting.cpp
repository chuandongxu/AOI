#include "ViewSetting.h"
#include "../Common/SystemData.h"

QViewSetting::QViewSetting(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	QStringList ls;
	ls << QStringLiteral("CSG��������") << QStringLiteral("˫���Բ�ֵ") << QStringLiteral("3D��������");
	ui.comboBoxCSG->addItems(ls);

	int nCSGIndex = System->getParam("3D_view_param_csg_alg").toInt();
	if (nCSGIndex < 0 || nCSGIndex > (ls.size() - 1)) nCSGIndex = 0;
	ui.comboBoxCSG->setCurrentIndex(nCSGIndex);

	ls.clear();
	ls << QStringLiteral("����������ʾ") << QStringLiteral("�Ͳ�������ʾ") << QStringLiteral("���Ͳ�������ʾ");
	ui.comboBoxSampFreq->addItems(ls);

	int nSampFreqIndex = System->getParam("3D_view_param_samp_freq").toInt();
	if (nSampFreqIndex < 0 || nSampFreqIndex >(ls.size() - 1)) nSampFreqIndex = 0;
	ui.comboBoxSampFreq->setCurrentIndex(nSampFreqIndex);

	connect(ui.comboBoxCSG, SIGNAL(currentIndexChanged(int)), SLOT(onCSGIndexChanged(int)));
	connect(ui.comboBoxSampFreq, SIGNAL(currentIndexChanged(int)), SLOT(onSampFreqIndexChanged(int)));
}

QViewSetting::~QViewSetting()
{
}

void QViewSetting::closeEvent(QCloseEvent *e)
{
	//qDebug() << "�ر��¼�";
	//e->ignore();

	this->hide();
}

void QViewSetting::onCSGIndexChanged(int index)
{
	int nCSGIndex = ui.comboBoxCSG->currentIndex();
	System->setParam("3D_view_param_csg_alg", nCSGIndex);
}

void QViewSetting::onSampFreqIndexChanged(int index)
{
	int nSampFreqIndex = ui.comboBoxSampFreq->currentIndex();
	System->setParam("3D_view_param_samp_freq", nSampFreqIndex);
}
