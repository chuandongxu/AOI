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
    void setMaskBinary(Binary& mask){ m_maskBinary = mask; }
    cv::Mat getMaskMat();

protected:
    InspWindowWidget           *m_pParent;
    Engine::Window              m_currentWindow;
    Engine::WindowGroup         m_windowGroup;
    bool                        m_bSupportMask;
    Binary                      m_maskBinary;
};

using InspWindowBaseWidgetPtr = std::unique_ptr<EditInspWindowBaseWidget>;
