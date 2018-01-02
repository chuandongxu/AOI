#include "IGraphicEditor.h"
#include "graphiceditor.h"

IGraphicEditor::IGraphicEditor()
{
	m_pInstance = new graphiceditor();
}

IGraphicEditor::~IGraphicEditor()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

void IGraphicEditor::setViewPos(int nPosX, int nPosY)
{
	m_pInstance->setViewPos(nPosX, nPosY);
}

void IGraphicEditor::setScale(double dScaleX, double dScaleY)
{
	m_pInstance->setScale(dScaleX, dScaleY);
}

void IGraphicEditor::clearSplines()
{
	m_pInstance->clearSplines();
}

void IGraphicEditor::addSpline(const aaAaa::aaSpline &knot)
{
	m_pInstance->addSpline(knot);
}

void IGraphicEditor::setSplines(const std::vector<aaAaa::aaSpline> &splines)
{
	m_pInstance->setSplines(splines);
}

void IGraphicEditor::addKnot(int index, const aaAaa::aaPoint &pt)
{
	m_pInstance->addKnot(index, pt);
}

void IGraphicEditor::addKnot(int index, float t)
{
	m_pInstance->addKnot(index, t);
}

int IGraphicEditor::getCurrentSplineIndex()
{
	return m_pInstance->getCurrentSplineIndex();
}

const std::vector<aaAaa::aaSpline>& IGraphicEditor::getSplines(void) const
{
	return m_pInstance->getSplines();
}

void IGraphicEditor::setPos(int posX, int posY)
{
	m_pInstance->setGeometry(posX, posY, m_pInstance->width(), m_pInstance->height());
}

void IGraphicEditor::getSize(int& width, int& height)
{
	width = m_pInstance->width();
	height = m_pInstance->height();
}

void IGraphicEditor::show()
{
	m_pInstance->show();
}

void IGraphicEditor::hide()
{
	m_pInstance->hide();
}


