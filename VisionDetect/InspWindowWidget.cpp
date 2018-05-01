#include <QMessageBox>

#include "InspWindowWidget.h"
#include "../include/IVisionUI.h"
#include "../include/IData.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"

#include "../DataModule/QDetectObj.h"
#include "../Common/eos.h"
#include "InspWindowSelectDialog.h"
#include "FindLineWidget.h"
#include "FindCircleWidget.h"
#include "InspVoidWidget.h"
#include "AlignmentWidget.h"
#include "HeightDetectWidget.h"

static const QString DEFAULT_WINDOW_NAME[] =
{
    "Find Line",
    "Inspect Hole",
    "Caliper Circle",
    "Alignment",
    "Height Detect",
};

static_assert (static_cast<size_t>(INSP_WIDGET_INDEX::SIZE) == sizeof(DEFAULT_WINDOW_NAME) / sizeof(DEFAULT_WINDOW_NAME[0]), "The window name size is not correct");

InspWindowWidget::InspWindowWidget(QWidget *parent, QColorWeight *pColorWidget)
: QWidget(parent), m_pColorWidget(pColorWidget) {
    ui.setupUi(this);

    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::FIND_LINE)] = std::make_unique<FindLineWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::INSP_HOLE)] = std::make_unique<InspVoidWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::CALIPER_CIRCLE)] = std::make_unique<FindCircleWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::ALIGNMENT)] = std::make_unique<AlignmentWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::HEIGHT_DETECT)] = std::make_unique<HeightDetectWidget>(this);

    for (const auto &ptrInspWindowWidget : m_arrInspWindowWidget)
        ui.stackedWidget->addWidget(ptrInspWindowWidget.get());

    QEos::Attach(EVENT_INSP_WINDOW_STATE, this, SLOT(onInspWindowState(const QVariantList &)));

    connect(ui.listWindowWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onSelectedWindowChanged(int)));

    m_pComboBoxLighting = std::make_unique<QComboBox>(this);
    QStringList ls;
    ls << QStringLiteral("白光") << QStringLiteral("低角度光") << QStringLiteral("彩色光") << QStringLiteral("均匀光") << QStringLiteral("3D灰阶图");
    m_pComboBoxLighting->addItems(ls);
    ui.tableWidgetHardware->setCellWidget(0, DATA_COLUMN, m_pComboBoxLighting.get());
    connect(m_pComboBoxLighting.get(), SIGNAL(currentIndexChanged(int)), SLOT(on_comboBoxLighting_indexChanged(int)));

    _hideWidgets();
}

InspWindowWidget::~InspWindowWidget() {
}

void InspWindowWidget::setCurrentIndex(int index) {
    ui.stackedWidget->setCurrentIndex(index);
}

void InspWindowWidget::UpdateInspWindowList() {
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);

    auto result = Engine::GetDeviceWindows(pUI->getSelectedDevice().getId(), m_vecCurrentDeviceWindows);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get inspect windows from database, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Inspect Window"), errorMessage.c_str(), QStringLiteral("Quit"));
    }

    ui.listWindowWidget->clear();
    for (const auto &window : m_vecCurrentDeviceWindows)
        ui.listWindowWidget->addItem(window.name.c_str());

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
}

int InspWindowWidget::getSelectedLighting() const {
    return m_pComboBoxLighting->currentIndex();
}

QColorWeight *InspWindowWidget::getColorWidget() const {
    return m_pColorWidget;
}

void InspWindowWidget::showEvent(QShowEvent *event) {
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);

    Engine::WindowVector vecWindow;
    auto result = Engine::GetAllWindows(vecWindow);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get inspect windows from database, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Inspect Window"), errorMessage.c_str(), QStringLiteral("Quit"));
    }

    double dResolutionX, dResolutionY;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    QVector<QDetectObj> vecDetectObjs;
    for (const auto &window : vecWindow) {
        auto x = window.x / dResolutionX;
        auto y = window.y / dResolutionY;
        auto width  = window.width  / dResolutionX;
        auto height = window.height / dResolutionY;
        cv::RotatedRect detectObjWin(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
        QDetectObj detectObj(window.Id, window.name.c_str());
        detectObj.setFrame(detectObjWin);
        vecDetectObjs.push_back(detectObj);
    }

    pUI->setDetectObjs(vecDetectObjs);
}

void InspWindowWidget::_showWidgets() {
    ui.stackedWidget->show();
    ui.labelWindowName->show();
    ui.labelHardwareConfig->show();
    ui.tableWidgetHardware->show();
}

void InspWindowWidget::_hideWidgets() {
    ui.stackedWidget->hide();
    ui.labelWindowName->hide();
    ui.labelHardwareConfig->hide();
    ui.tableWidgetHardware->hide();
}

void InspWindowWidget::on_btnAddWindow_clicked() {
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    if (pUI->getSelectedDevice().getId() <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Window"), QStringLiteral("Please select a device first."));
        return;
    }

    InspWindowSelectDialog dialog;
    int iReturn = dialog.exec();
    if (iReturn != QDialog::Accepted)
        return;

    _showWidgets();

    m_enCurrentInspWidget = dialog.getWindowIndex();
    int index = static_cast<int> (m_enCurrentInspWidget);
    ui.stackedWidget->setCurrentIndex(index);
    ui.labelWindowName->setText(DEFAULT_WINDOW_NAME[index]);
    m_enOperation = OPERATION::ADD;

    if (INSP_WIDGET_INDEX::INSP_HOLE == m_enCurrentInspWidget) {
        m_pColorWidget->show();
    }
    else {
        m_pColorWidget->hide();
    }

    if (INSP_WIDGET_INDEX::HEIGHT_DETECT == m_enCurrentInspWidget)
        m_pComboBoxLighting->setCurrentIndex(PROCESSED_IMAGE_SEQUENCE::HEIGHT_GRAY);
    else
        m_pComboBoxLighting->setCurrentIndex(PROCESSED_IMAGE_SEQUENCE::WHITE_LIGHT);

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->setDefaultValue();
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
}

void InspWindowWidget::on_btnRemoveWindow_clicked() {
    auto index = ui.listWindowWidget->currentRow();
    if (index >= 0) {
        auto windowId = m_vecCurrentDeviceWindows[index].Id;
        Engine::DeleteWindow(windowId);
        ui.listWindowWidget->takeItem(index);

        UpdateInspWindowList();

        IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
        auto vecDetectObjs = pUI->getDetectObjs();
        auto it = std::remove_if(vecDetectObjs.begin(), vecDetectObjs.end(), [windowId](QDetectObj &detectObj) { return detectObj.getID() == windowId; });
        vecDetectObjs.erase(it);
        pUI->setDetectObjs(vecDetectObjs);
    }
}

void InspWindowWidget::on_btnTryInsp_clicked() {
    if (INSP_WIDGET_INDEX::UNDEFINED == m_enCurrentInspWidget)
        return;

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->tryInsp();
}

void InspWindowWidget::on_btnConfirmWindow_clicked() {
    if (INSP_WIDGET_INDEX::UNDEFINED == m_enCurrentInspWidget)
        return;

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->confirmWindow(m_enOperation);
}

void InspWindowWidget::onInspWindowState(const QVariantList &data) {
    UpdateInspWindowList();
    if (ui.listWindowWidget->count() > 0) {
        ui.listWindowWidget->item(0)->setSelected(true);
        ui.listWindowWidget->setCurrentRow(0);
    }
}

void InspWindowWidget::onSelectedWindowChanged(int index) {
    if (index < 0) {
        _hideWidgets();
        return;
    }

    _showWidgets();

    auto window = m_vecCurrentDeviceWindows[index];

    if (Engine::Window::Usage::FIND_LINE == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::FIND_LINE;
    else if (Engine::Window::Usage::INSP_HOLE == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::INSP_HOLE;
    else if (Engine::Window::Usage::FIND_CIRCLE == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::CALIPER_CIRCLE;
    else if (Engine::Window::Usage::ALIGNMENT == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::ALIGNMENT;
    else if (Engine::Window::Usage::HEIGHT_MEASURE == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::HEIGHT_DETECT;
    else if (Engine::Window::Usage::HEIGHT_BASE == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::HEIGHT_DETECT;

    ui.labelWindowName->setText(window.name.c_str());

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->setCurrentWindow(m_vecCurrentDeviceWindows[index]);
    ui.stackedWidget->setCurrentIndex(static_cast<int>(m_enCurrentInspWidget));

    m_pComboBoxLighting->setCurrentIndex(window.lightId - 1);

    double dResolutionX, dResolutionY;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto x = window.x / dResolutionX;
    auto y = window.y / dResolutionY;
    auto width = window.width / dResolutionX;
    auto height = window.height / dResolutionY;
    cv::RotatedRect detectObjWin(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
    QDetectObj detectObj(window.Id, window.name.c_str());
    detectObj.setFrame(detectObjWin);

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setCurrentDetectObj(detectObj);
    m_enOperation = OPERATION::EDIT;

    if (INSP_WIDGET_INDEX::INSP_HOLE == m_enCurrentInspWidget) {
        cv::Mat matImage = pUI->getImage();
        cv::Rect rectROI = cv::RotatedRect(cv::Point(x, y), cv::Size(width, height), window.angle).boundingRect();
        cv::Mat matROI(matImage, rectROI);
        m_pColorWidget->setImage(matROI);
        m_pColorWidget->setJsonFormattedParams(window.colorParams);
        m_pColorWidget->show();
    }else
        m_pColorWidget->hide();
}

void InspWindowWidget::on_comboBoxLighting_indexChanged(int index) {
    auto pData = getModule<IData>(DATA_MODEL);
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto vecCombinedBigImage = pData->getCombinedBigImages();
    if (index >= 0 && index < vecCombinedBigImage.size() && !vecCombinedBigImage[index].empty())
        pUI->setImage(vecCombinedBigImage[index]);
}