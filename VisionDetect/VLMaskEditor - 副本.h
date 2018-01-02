#pragma once

#include <QWidget>
#include "ui_QVLMaskEditor.h"

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

enum MaskType
{
	MASK_TYPE_RECT,
	MASK_TYPE_CIRCLE,
	MASK_TYPE_POLYLINE,
	MASK_TYPE_NULL
};

class MaskObj
{
public:
	MaskObj()
	{
		m_bSelect = false;
	}
	~MaskObj(){}
public:
	MaskType getType(){ return m_emType; }
	void setType(MaskType type){ m_emType = type; }
	virtual bool IsRect(){ return MASK_TYPE_RECT == m_emType; }
	virtual bool IsCircle(){ return MASK_TYPE_CIRCLE == m_emType; }
	virtual bool IsPolyLine(){ return MASK_TYPE_POLYLINE == m_emType; }

	void setSelect(bool bSelect){ m_bSelect = bSelect; }
	bool IsSelect(){ return m_bSelect; }

private:
	MaskType m_emType;
	bool m_bSelect;
};

class RectMask : public MaskObj
{
public:
	RectMask()	
	{
		setType(MASK_TYPE_RECT);
	}
	~RectMask(){}

	bool isEmpty(){ return !(_rect.size.width > 1 || _rect.size.height > 1); }
	void clear(){ _rect.size.width = 0; _rect.size.height = 0; }

public:
	cv::RotatedRect _rect;
};

class CircleMask : public MaskObj
{
public:
	CircleMask(){ setType(MASK_TYPE_CIRCLE); }
	~CircleMask(){}

	bool isEmpty(){ return _radius <= 0; }
	void clear(){ _radius = 0; }

public:
	cv::Point2f _center;
	double 	    _radius;
};

class PolyLineMask : public MaskObj
{
public:
	PolyLineMask(){ setType(MASK_TYPE_POLYLINE); }
	~PolyLineMask(){}

	bool isEmpty(){ return _polyPts.size() == 0; }
	void clear(){ _polyPts.clear(); }

public:
	std::vector<cv::Point2f> _polyPts;
};

class QVLMaskEditor : public QWidget
{
	Q_OBJECT

public:
	QVLMaskEditor(QWidget *parent = Q_NULLPTR);
	~QVLMaskEditor();

protected:
	void closeEvent(QCloseEvent *e);

public:
	void setImage(cv::Mat& matImage, bool bClearAll = false);
	void repaintAll();

	void addSelect(RotatedRect& rect);
	void clear();
	int getSelectNum();
	RotatedRect getSelect(int nIndex = 0);
	Rect getSelectRect(int nIndex = 0);
	cv::Mat getMaskImage();

	void setEditorView(bool bSelectEnable = true, bool bMaskEnable = true);
	void setSelectRtRect(bool bSelectRect = false);
	
protected:
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

protected slots:
	void onSelectIndexChanged(int iState);
	void onAddSelect();
	void onDeleteSelect();

	void onMaskIndexChanged(int iState);
	void onAddMask();
	void onDeleteMask();

	void actionMove();
	void actionDelete();

private:
	void initValue();
	void A_Transform(Mat& src, Mat& dst, int dx, int dy);
	void setViewState(int state);
	void displayImage(cv::Mat& image);	
	double convertToImgX(double dMouseValue, bool bZero = false);
	double convertToImgY(double dMouseValue, bool bZero = false);
	void addImageText(cv::Mat image, Point ptPos, QString szText);

private:
	int findSelect(double mousePosX, double mousePosY);

	bool getRotRectLocationPt(Mat& img, int nCornerNum, Point2f* ptCorners, int nLocationNum, Point2f* ptLocations, double& dRadius);
	int getLocFromSelect(double mousePosX, double mousePosY);
	bool rangeSelect(double mousePosX, double mousePosY, double& dScaleX, double& dScaleY);
	bool rotateSelect(double mousePosX, double mousePosY, double& dRotatedAngle);

	void drawSelectRect(cv::RotatedRect& selectRect);
	void drawSelectRectRotated(cv::RotatedRect& selectRect, double dRotatedAngle);

	MaskType getMaskType();
	void unSelectMask();

private:
	int    m_stateView;
	bool   m_mouseLeftPressed;
	bool   m_mouseRightPressed;
	double m_startX, m_startY;

	int   m_windowWidth;
	int   m_windowHeight;
	int   m_imageWidth;
	int   m_imageHeight;

private:
	Ui::QVLMaskEditor ui;
	cv::Mat	m_hoImage;

	std::vector<cv::RotatedRect> m_selectRects;
	int m_nSelectIndex;
	cv::RotatedRect m_selectRectCur;

	std::vector<MaskObj*> m_maskObjs;
	RectMask m_maskRectCur;
	CircleMask m_maskCircleCur;
	PolyLineMask m_maskPolyLineCur;

	bool m_bSelectRect;

	//QAction *m_ActionMove;
	//QAction *m_ActionDelete;
};
