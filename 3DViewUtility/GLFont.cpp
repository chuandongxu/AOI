#include "GLFont.h"
#include <QDebug>

HDC CGameFont::m_hDeviceContext = NULL;

CGameFont::CGameFont() : m_uiListBase(0)
{
}

CGameFont::~CGameFont()
{
	if (m_uiListBase)
		glDeleteLists(m_uiListBase,255);
	DeleteObject(m_hFont);
}

void CGameFont::CreateFont(const QString& strTypeface,
						   int iFontHeight,
						   int iFontWeight)
{
	if (!m_hDeviceContext)
	{
		QString strError = "Impossible to create the font: ";
		strError += strTypeface;
		qDebug() << strError;
		return;
	}

	// Ask openGL to generate a contiguous set of 255 display lists.
	m_uiListBase = glGenLists(255);
	if (m_uiListBase == 0)
	{
		QString strError = "Impossible to create the font: ";
		strError += strTypeface;
		qDebug() << strError;
		return;
	}

	// Create the Windows font
	m_hFont = ::CreateFont(-iFontHeight,
		0,
		0,
		0,
		iFontWeight,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		FF_DONTCARE|DEFAULT_PITCH,
		strTypeface.toStdWString().c_str());
	if (m_hFont == NULL)
	{
		m_uiListBase = 0;
		QString strError = "Impossible to create the font: ";
		strError += strTypeface;
		qDebug() << strError;
		return;
	}

	// Select the newly create font into the device context (and save the previous
	// one).
	HFONT hOldFont = (HFONT)SelectObject(m_hDeviceContext, m_hFont);
	// Generate the font display list (for the 255 characters) starting
	// at display list m_uiListBase.
	wglUseFontBitmaps(m_hDeviceContext, 0, 255, m_uiListBase);  
	// Set the original font back in the device context
	SelectObject(m_hDeviceContext, hOldFont); 
}

void CGameFont::DrawText(const QString& strText,
						 int XPos, int YPos,
						 GLfloat fRed, 
						 GLfloat fGreen, 
						 GLfloat fBlue)
{
	if (m_uiListBase == 0)
	{
		qDebug() << "Impossible to display the text.";
		return;
	}

	// Disable 2D texturing
	glDisable(GL_TEXTURE_2D);
	// Specify the current color
	glColor3f(fRed, fGreen, fBlue);
	// Specify the position of the text
	glRasterPos2i(XPos, YPos);

	// Push the list base value
	glPushAttrib (GL_LIST_BIT);
	// Set a new list base value. 
	glListBase(m_uiListBase);
	// Call the lists to draw the text.
	glCallLists((GLsizei)strText.size(), GL_UNSIGNED_BYTE, 
		(GLubyte *)strText.toStdWString().c_str());
	glPopAttrib ();

	// Reenable 2D texturing
	glEnable(GL_TEXTURE_2D);
}

SIZE CGameFont::GetTextSize(const QString &strText)
{
	SIZE TextSize;
	HFONT hOldFont = (HFONT)SelectObject(m_hDeviceContext, m_hFont);
	// Retrieve the size of the text
	GetTextExtentPoint32(m_hDeviceContext, strText.toStdWString().c_str(), 
		(int)strText.size(), &TextSize);
	SelectObject(m_hDeviceContext, hOldFont);
	
	return TextSize;
}
