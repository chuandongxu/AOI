#pragma once

#include <QWidget>
#include <memory>
#include "DataStoreAPI.h"
#include "VisionAPI.h"

using namespace NFG::AOI;

enum class OPERATION {
    ADD,
    EDIT
};

class InspWindowWidget;
class EditInspWindowBaseWidget : public QWidget
{
    Q_OBJECT

public:
    EditInspWindowBaseWidget(InspWindowWidget *parent = Q_NULLPTR);
    virtual ~EditInspWindowBaseWidget() = default;

    virtual void setDefaultValue() = 0;
    virtual void tryInsp() = 0;
    virtual void confirmWindow(OPERATION enOperation) = 0;
    virtual void setCurrentWindow(const Engine::Window &window) = 0;
    virtual void setWindowGroup(const Engine::WindowGroup &windowGroup);
   
    bool isSupportMask() { return m_bSupportMask; }
    void setMask(cv::Mat& maskMat);
    cv::Mat getMask(){ return m_maskMat; }

    cv::Mat convertMaskBny2Mat(Binary maskBinary);
    Binary convertMaskMat2Bny(cv::Mat& maskMat);

protected:
    InspWindowWidget           *m_pParent;
    Engine::Window              m_currentWindow;
    Engine::WindowGroup         m_windowGroup;
    bool                        m_bSupportMask;
    cv::Mat                     m_maskMat;
};

using InspWindowBaseWidgetPtr = std::unique_ptr<EditInspWindowBaseWidget>;
