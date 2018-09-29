#ifndef EDITOCVRECORDDIALOG_H
#define EDITOCVRECORDDIALOG_H

#include <QDialog>
#include "ui_EditOcvRecordDialog.h"
#include "VisionAPI.h"

using namespace AOI;

class EditOcvRecordDialog : public QDialog
{
    Q_OBJECT

    enum class EDIT_MODE {
        UNDEFINED,
        MOVE,
        RESIZE,
    };

public:
    EditOcvRecordDialog(QWidget *parent = 0);
    ~EditOcvRecordDialog();
    void setImage(const cv::Mat &matImage);
    void setCharRects(const Vision::VectorOfRect &vecRects);
    Vision::VectorOfRect getCharRects() const;

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    void _resizeRect(double motionX, double motionY);
    void _repaintAll();
    void _cutImageForDisplay(const cv::Mat &matInputImg);
    cv::Point _convertToImgPos(const cv::Point &ptMousePos);

private:
    Ui::EditOcvRecordDialog ui;
    cv::Mat                 m_matImage;
    Vision::VectorOfRect    m_vecCharRects;
    int                     m_nCurrentRectIndex = -1;
    EDIT_MODE               m_enEditMode = EDIT_MODE::UNDEFINED;
    Vision::PR_DIRECTION    m_enResizeDirection;
    int                     m_preMoveX, m_preMoveY;
};

#endif // EDITOCVRECORDDIALOG_H
