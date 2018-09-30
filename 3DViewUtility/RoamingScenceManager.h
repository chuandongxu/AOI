#ifndef ROAMINGSCENCEMANAGER_H
#define ROAMINGSCENCEMANAGER_H

#include <QtOpenGL/QGLWidget>

class Vector3D;
class Position3D;
//class CGameFont;

enum ViewPerspective
{
	ACTION_PERSPECT_FRONT,
	ACTION_PERSPECT_SIDE,
	ACTION_PERSPECT_TOP,	
	ACTION_PERSPECT_FULL,
	ACTION_PERSPECT_OPTIONAL,
	ACTION_PERSPECT_NULL
};

class RoamingScenceManager
{
public:
    RoamingScenceManager(QGLWidget* pWidget);
    ~RoamingScenceManager();

	void setOrthoSize(double orthoSize);
	void setXYSize(int xyAxisLen, int xyOffset);
	void setZSize(int zAxisLen, int zOffset);

    void init();
    void render();
    void executeRotateOperation(int x, int y);
    void executeScaleOperation(float factor);
    void executeTranslateOperation(int x,int y);
    void getInitPos(int x, int y);

	void fullView(ViewPerspective viewPsp);/* nView: 0 front view, 1 side view, 2 top view, 3 full view*/
	ViewPerspective getViewPsp(){ return m_viewPerspective; }

	void getAxisZLimit(int& nMaxZ, int& nMinZ);
	void getAxisXYLimit(int& nMaxValue, int& nMinValue);

	int getAxisUnit();

	double getScaleFactor();

	void setXYCoordinate(bool bInvert);

private:
	void GLGrid(Position3D& pt1, Position3D& pt2, int num);
	void GLDrawSpaceAxes(void);
	void drawString(QString& strText, double posX, double posY, double posZ, QFont& font);
	void drawStringExt(QString& strText, int posX, int posY, QFont& font);
	double AXES_LEN;
	double AXES_LEN_OFFSET;
	double AXES_LEN_Z;
	double AXES_LEN_Z_OFFSET;
	double AXES_GRADUATION;
	int AXES_UNIT;

private:
	ViewPerspective m_viewPerspective;
	double m_OrthoSize;
    //辅助坐标系三根轴
    Vector3D *AuxX;
    Vector3D *AuxY;
    Vector3D *AuxZ;

    //旋转后观察点方向与视线向上方向
    Vector3D *NewEye;
    Vector3D *NewUp;
    Vector3D *NewView;

    Position3D *OldMouse;
    Position3D *Mouse;

    Vector3D *TempTranslateVec;
    float TempscaleFactor;

    int ID_COORDINATY;
    void init_CoordinaryDisplayList();
	void render_CoordinaryDisplayText();

	//CGameFont* m_newFont;
	QGLWidget* m_viewWidget;

	bool m_bXYCoordinateInvert;
};

#endif // ROAMINGSCENCEMANAGER_H
