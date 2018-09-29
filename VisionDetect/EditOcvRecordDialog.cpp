#include <QMouseEvent>

#include "EditOcvRecordDialog.h"
#include "../DataModule/CalcUtils.hpp"

/*static*/ const cv::Scalar RED_SCALAR    (0,   0,   255 );
/*static*/ const cv::Scalar BLUE_SCALAR   (255, 0,   0   );
/*static*/ const cv::Scalar CYAN_SCALAR   (255, 255, 0   );
/*static*/ const cv::Scalar GREEN_SCALAR  (0,   255, 0   );
/*static*/ const cv::Scalar YELLOW_SCALAR (0,   255, 255 );

EditOcvRecordDialog::EditOcvRecordDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

EditOcvRecordDialog::~EditOcvRecordDialog()
{
}

void EditOcvRecordDialog::setImage(const cv::Mat &matImage) {
    m_matImage = matImage;
    if (m_matImage.type() == CV_8UC1)
        cv::cvtColor(m_matImage, m_matImage, CV_GRAY2BGR);
}

void EditOcvRecordDialog::setCharRects(const Vision::VectorOfRect &vecRects) {
    m_vecCharRects = vecRects;
}

Vision::VectorOfRect EditOcvRecordDialog::getCharRects() const {
    return m_vecCharRects;
}

void EditOcvRecordDialog::showEvent(QShowEvent *event) {
    _repaintAll();
}

void EditOcvRecordDialog::mouseMoveEvent(QMouseEvent * event) {
    if (Qt::LeftButton != event->buttons())
        return;

    if (m_nCurrentRectIndex < 0 || m_nCurrentRectIndex >= m_vecCharRects.size())
        return;

    double mouseX = event->x(), mouseY = event->y();
    double motionX = 0, motionY = 0;
    motionX = mouseX - m_preMoveX;
    motionY = mouseY - m_preMoveY;

    auto rectEdit = m_vecCharRects[m_nCurrentRectIndex];
    cv::Rect rectBigTmpl(0, 0, m_matImage.cols, m_matImage.rows);

    switch (m_enEditMode)
    {
    case EDIT_MODE::MOVE:
    {
        rectEdit.x += motionX;
        rectEdit.y += motionY;
        if (CalcUtils::isRectInRect(rectEdit, rectBigTmpl)) {
            m_vecCharRects[m_nCurrentRectIndex] = rectEdit;
        }
        break;
    }    
    case EDIT_MODE::RESIZE:
        _resizeRect(motionX, motionY);
        break;

    case EDIT_MODE::UNDEFINED:
    default:
        break;
    }
    _repaintAll();

    m_preMoveX = event->x();
    m_preMoveY = event->y();
}

void EditOcvRecordDialog::_resizeRect(double motionX, double motionY) {
    auto rectEdit = m_vecCharRects[m_nCurrentRectIndex];
    cv::Rect rectBigTmpl(0, 0, m_matImage.cols, m_matImage.rows);

    switch (m_enResizeDirection)
    {
    case Vision::PR_DIRECTION::UP:
    {
        auto oldBtm = rectEdit.y + rectEdit.height;
        rectEdit.y += motionY;
        rectEdit.height = oldBtm - rectEdit.y;
        break;
    }
    case Vision::PR_DIRECTION::DOWN:
    {
        rectEdit.height += motionY;
        break;
    }
    case Vision::PR_DIRECTION::LEFT:
    {
        auto oldRgt = rectEdit.x + rectEdit.width;
        rectEdit.x += motionX;
        rectEdit.width = oldRgt - rectEdit.x;
        break;
    }
    case Vision::PR_DIRECTION::RIGHT:
    {
        rectEdit.width += motionX;
        break;
    }
    default:
        break;
    }

    if (CalcUtils::isRectInRect(rectEdit, rectBigTmpl)) {
        m_vecCharRects[m_nCurrentRectIndex] = rectEdit;
    }
}

void EditOcvRecordDialog::mousePressEvent(QMouseEvent * event) {
    if (Qt::LeftButton != event->buttons())
        return;

    if (m_nCurrentRectIndex < 0 || m_nCurrentRectIndex >= m_vecCharRects.size())
        return;

    cv::Rect &rectChar = m_vecCharRects[m_nCurrentRectIndex];
    const QPoint pos = event->pos();
    cv::Point point(pos.x(), pos.y());
    point = _convertToImgPos(point);
    const int SELECT_EDGE_MARGIN = 3;
    if (abs(point.x - rectChar.x) <= SELECT_EDGE_MARGIN) {
        m_enEditMode = EDIT_MODE::RESIZE;
        m_enResizeDirection = Vision::PR_DIRECTION::LEFT;
        setCursor(Qt::SizeHorCursor);
    }if (abs(point.y - rectChar.y) <= SELECT_EDGE_MARGIN) {
        m_enEditMode = EDIT_MODE::RESIZE;
        m_enResizeDirection = Vision::PR_DIRECTION::UP;
        setCursor(Qt::SizeVerCursor);
    }else if (abs(point.x - rectChar.x - rectChar.width) <= SELECT_EDGE_MARGIN) {
        m_enEditMode = EDIT_MODE::RESIZE;
        m_enResizeDirection = Vision::PR_DIRECTION::RIGHT;
        setCursor(Qt::SizeHorCursor);
    }else if (abs(point.y - rectChar.y - rectChar.height) <= SELECT_EDGE_MARGIN) {
        m_enEditMode = EDIT_MODE::RESIZE;
        m_enResizeDirection = Vision::PR_DIRECTION::DOWN;
        setCursor(Qt::SizeVerCursor);
    }else if (rectChar.contains(point)) {
        m_enEditMode = EDIT_MODE::MOVE;
        setCursor(Qt::SizeAllCursor);
    }

    m_preMoveX = event->x();
    m_preMoveY = event->y();
}

void EditOcvRecordDialog::mouseDoubleClickEvent(QMouseEvent *event) {
    if (Qt::LeftButton != event->buttons())
        return;

    const QPoint pos = event->pos();
    cv::Point point(pos.x(), pos.y());
    point = _convertToImgPos(point);
    bool bFound = false;
    int index = 0;
    for (const auto &rect : m_vecCharRects) {
        if (rect.contains(point)) {
            m_nCurrentRectIndex = index;
            bFound = true;
            break;
        }
        ++ index;
    }
    if (bFound)
        _repaintAll();
}

void EditOcvRecordDialog::mouseReleaseEvent(QMouseEvent *event) {
    setCursor(Qt::ArrowCursor);
}

void EditOcvRecordDialog::_repaintAll() {
    if (m_matImage.empty())
        return;

    cv::Mat matDisplay = m_matImage.clone();
    for (const auto& rect : m_vecCharRects) {
        cv::rectangle(matDisplay, rect, BLUE_SCALAR, 1);
    }
    if (m_nCurrentRectIndex >= 0 && m_nCurrentRectIndex < m_vecCharRects.size())
        cv::rectangle(matDisplay, m_vecCharRects[m_nCurrentRectIndex], YELLOW_SCALAR, 1);
    _cutImageForDisplay(matDisplay);
}

void EditOcvRecordDialog::_cutImageForDisplay(const cv::Mat &matInputImg)
{
    if (matInputImg.empty())
        return;

    double m_dScale = 1.0;
    float m_dMovedX = 0.f, m_dMovedY = 0.f;

    auto displayWidth  = ui.labelImage->size().width();
    auto displayHeight = ui.labelImage->size().height();
    cv::Mat matOutput = cv::Mat::ones(displayHeight, displayWidth, matInputImg.type()) * 255;
    matOutput.setTo(cv::Scalar(255, 255, 255));

    cv::Mat matZoomResult;
    cv::resize(matInputImg, matZoomResult, cv::Size(), m_dScale, m_dScale);

    if (matZoomResult.cols >= displayWidth && matZoomResult.rows >= displayHeight) {
        cv::Rect rectROISrc((matZoomResult.cols - displayWidth) / 2 - m_dMovedX, (matZoomResult.rows - displayHeight) / 2 - m_dMovedY, displayWidth, displayHeight);
        cv::Mat matSrc(matZoomResult, rectROISrc);
        matSrc.copyTo(matOutput);
    }
    else if (matZoomResult.cols >= displayWidth && matZoomResult.rows <= displayHeight) {
        cv::Rect rectROISrc((matZoomResult.cols - displayWidth) / 2 - m_dMovedX, 0, displayWidth, matZoomResult.rows);
        cv::Mat matSrc(matZoomResult, rectROISrc);
        cv::Rect rectROIDst(0, (displayHeight - matZoomResult.rows) / 2, displayWidth, matZoomResult.rows);
        cv::Mat matDst(matOutput, rectROIDst);
        matSrc.copyTo(matDst);
    }
    else if (matZoomResult.cols <= displayWidth && matZoomResult.rows >= displayHeight) {
        cv::Rect rectROISrc(0, (matZoomResult.rows - displayHeight) / 2 - m_dMovedY, matZoomResult.cols, displayHeight);
        cv::Mat matSrc(matZoomResult, rectROISrc);
        cv::Rect rectROIDst((displayWidth - matZoomResult.cols) / 2, 0, matZoomResult.cols, displayHeight);
        cv::Mat matDst(matOutput, rectROIDst);
        matSrc.copyTo(matDst);
    }
    else if (matZoomResult.cols <= displayWidth && matZoomResult.rows <= displayHeight) {
        cv::Rect rectROIDst((displayWidth - matZoomResult.cols) / 2, (displayHeight - matZoomResult.rows) / 2, matZoomResult.cols, matZoomResult.rows);
        cv::Mat matDst(matOutput, rectROIDst);
        matZoomResult.copyTo(matDst);
    }

    cv::Mat matDisplay;
    if (matOutput.type() == CV_8UC3)
        cvtColor(matOutput, matDisplay, CV_BGR2RGB);
    else if (matOutput.type() == CV_8UC1)
        cvtColor(matOutput, matDisplay, CV_GRAY2RGB);

    QImage imagePixmap = QImage((uchar*)matDisplay.data, matDisplay.cols, matDisplay.rows, static_cast<int>(matDisplay.step), QImage::Format_RGB888);
    ui.labelImage->setPixmap(QPixmap::fromImage(imagePixmap));
}

cv::Point EditOcvRecordDialog::_convertToImgPos(const cv::Point &ptMousePos) {
    const auto COLS = m_matImage.cols;
    const auto ROWS = m_matImage.rows;
    const auto LABEL_IMAGE_WIDTH  = ui.labelImage->size().width();
    const auto LABEL_IMAGE_HEIGHT = ui.labelImage->size().height();
    cv::Point ptOnImage;
    ptOnImage.x = ptMousePos.x - (LABEL_IMAGE_WIDTH  -  COLS) / 2;
    ptOnImage.y = ptMousePos.y - (LABEL_IMAGE_HEIGHT - ROWS ) / 2;

    return ptOnImage;
}