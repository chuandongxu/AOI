// Utility class used to draw text on the screen using a 
// specific font.
#include <QObject>
#include "GL/glut.h"

class CGameFont
{
public:
	// Default constructor
	CGameFont();
	// Default destructor
	~CGameFont();

	// Create the font with a specific height and weight.
	void CreateFont(const QString& strTypeface , 
		int iFontHeight, 
		int iFontWeight);
	// Draw text on the screen at the specified location with
	// the specified colour.
	void DrawText(const QString& strText, int XPos,
		int YPos, GLfloat fRed, GLfloat fGreen, 
		GLfloat fBlue);

	// Returns the size of the text. The top and right fields
	// of the returned rectangle are set to 0.
	SIZE GetTextSize(const QString& strText);

	static void SetDeviceContext(HDC hDevContext)  
	{ m_hDeviceContext = hDevContext; }

	bool isCreateFont(){ return m_uiListBase > 0; }

private:
	// The device context used to create the font.
	static HDC m_hDeviceContext;
	// The index of the base of the lists.
	GLuint m_uiListBase;
	// The win32 font
	HFONT m_hFont;
};