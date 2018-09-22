#pragma once

#include "curveeditor_global.h"

#include <QDialog>

#include "libspline/aaCurve.h"
#include <vector>

class graphiceditor;
class  CURVEEDITOR_EXPORT IGraphicEditor
{
public:
    IGraphicEditor();
    ~IGraphicEditor();

    void setViewPos(int nPosX, int nPosY);
    void setScale(double dScaleX, double dScaleY);

    void clearSplines();
    void addSpline(const aaAaa::aaSpline &knot);
    void setSplines(const std::vector<aaAaa::aaSpline> &splines);
    void addKnot(int index, const aaAaa::aaPoint &pt);
    void addKnot(int index, float t);

    int getCurrentSplineIndex();

    const std::vector<aaAaa::aaSpline> &getSplines(void) const;

    void setPos(int posX, int posY);
    void getSize(int& width, int& height);
    void show();
    void hide();

private:
    graphiceditor *m_pInstance;
};


