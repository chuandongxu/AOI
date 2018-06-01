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
#include "DialogCreateGroup.h"
#include "FindLineWidget.h"
#include "FindCircleWidget.h"
#include "InspVoidWidget.h"
#include "AlignmentWidget.h"
#include "HeightDetectWidget.h"
#include "InspPolarityWidget.h"
#include "InspContourWidget.h"
#include "InspChipWidget.h"
#include "TreeWidgetInspWindow.h"

static const QString DEFAULT_WINDOW_NAME[] =
{
    "Find Line",
    "Inspect Hole",
    "Caliper Circle",
    "Alignment",
    "Height Detect",
    "Inspect Polarity",
    "Inspect Contour",
    "Inspect Chip",
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
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::INSP_POLARITY)] = std::make_unique<InspPolarityWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::INSP_CONTOUR)] = std::make_unique<InspContourWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::INSP_CHIP)] = std::make_unique<InspChipWidget>(this);

    for (const auto &ptrInspWindowWidget : m_arrInspWindowWidget)
        ui.stackedWidget->addWidget(ptrInspWindowWidget.get());

    QEos::Attach(EVENT_INSP_WINDOW_STATE, this, SLOT(onInspWindowState(const QVariantList &)));

    connect(ui.treeWidget, SIGNAL(regrouped()), this, SLOT(on_regrouped()));
    connect(ui.treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectedWindowChanged()));    

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
    auto deviceId = pUI->getSelectedDevice().getId();

    Int64Vector vecGroupId;
    auto result = Engine::GetDeviceWindowGroups(deviceId, vecGroupId);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("Failed to get device groups from database, error message "));
        msg += errorMessage.c_str();
        System->showMessage(QStringLiteral("检测框"), msg);
        return;
    }

    ui.treeWidget->clear();
    m_vecWindowGroup.clear();
    for (const auto groupId : vecGroupId) {
        Engine::WindowGroup windowGroup;
        auto result = Engine::GetGroupWindows(groupId, windowGroup);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("Failed to get group windows from database, error message "));
            msg += errorMessage.c_str();
            System->showMessage(QStringLiteral("检测框"), msg);
            return;
        }
        m_vecWindowGroup.push_back(windowGroup);

        QTreeWidgetItem *pGroupItem = new QTreeWidgetItem(QStringList{windowGroup.name.c_str()}, TREE_ITEM_GROUP);
        pGroupItem->setData(0, Qt::UserRole, windowGroup.Id);
        pGroupItem->setIcon(0, QIcon("./Image/Group.png"));
        ui.treeWidget->addTopLevelItem(pGroupItem);
        for (auto const &window : windowGroup.vecWindows) {
            QTreeWidgetItem *pItem = new QTreeWidgetItem(QStringList{window.name.c_str()}, TREE_ITEM_WINDOW);
            pItem->setData(0, Qt::UserRole, window.Id);
            pGroupItem->addChild(pItem);
        }
    }

    Engine::WindowVector vecCurrentDeviceWindows;
    result = Engine::GetDeviceUngroupedWindows(deviceId, vecCurrentDeviceWindows);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("Failed to get inspect windows from database, error message "));
        msg += errorMessage.c_str();
        System->showMessage(QStringLiteral("检测框"), msg);
        return;
    }

    m_mapIdWindow.clear();
    for(const auto &window : vecCurrentDeviceWindows)
        m_mapIdWindow.insert(std::pair<Int64, Engine::Window>(window.Id, window));
    for (const auto &window : vecCurrentDeviceWindows) {
        QTreeWidgetItem *pItem = new QTreeWidgetItem(QStringList{window.name.c_str()}, TREE_ITEM_WINDOW);
        pItem->setData(0, Qt::UserRole, window.Id);
        ui.treeWidget->addTopLevelItem(pItem);
    }
    ui.treeWidget->expandAll();
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

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    auto matImage = pUI->getImage();
    m_nBigImgWidth  = matImage.cols / dCombinedImageScale;
    m_nBigImgHeight = matImage.rows / dCombinedImageScale;

    QVector<QDetectObj> vecDetectObjs;
    for (const auto &window : vecWindow) {
        auto x = window.x / dResolutionX;
        auto y = window.y / dResolutionY;
        if (bBoardRotated)
            x = m_nBigImgWidth  - x;
        else
            y = m_nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

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
    auto selectedItems = ui.treeWidget->selectedItems();
    if (selectedItems.size() <= 0)
        return;

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    auto vecDetectObjs = pUI->getDetectObjs();

    for (const auto &pItem : selectedItems) {
        if (TREE_ITEM_WINDOW == pItem->type()) {
            auto windowId = pItem->data(0, Qt::UserRole).toInt();
            auto windowName = pItem->text(0);
            QString msg = "Are you sure to delete the inspection window: " + windowName;
            int result = QMessageBox::warning(this, QStringLiteral("删除检测框"), msg, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
            if (result != QMessageBox::StandardButton::Ok)
                return;

            result = Engine::DeleteWindow(windowId);
            if (result != Engine::OK) {
                String errorType, errorMessage;
                Engine::GetErrorDetail(errorType, errorMessage);
                QString msg(QStringLiteral("删除检测框失败, 错误消息: "));
                msg += errorMessage.c_str();
                System->showMessage(QStringLiteral("检测框"), msg);
                return;
            }
            auto pParent = pItem->parent();
            if (NULL == pParent)
                ui.treeWidget->takeTopLevelItem(ui.treeWidget->indexOfTopLevelItem(pItem));
            else {
                int index = pParent->indexOfChild(pItem);
                pParent->takeChild(index);
            }

            auto it = std::remove_if(vecDetectObjs.begin(), vecDetectObjs.end(), [windowId](QDetectObj &detectObj) { return detectObj.getID() == windowId; });
            vecDetectObjs.erase(it);
            pUI->setDetectObjs(vecDetectObjs);
        }else {
            auto groupwId = pItem->data(0, Qt::UserRole).toInt();
            auto groupName = pItem->text(0);
            QString msg = "Are you sure to delete the group: " + groupName;
            int result = QMessageBox::warning(this, QStringLiteral("删除检测框"), msg, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
            if (result != QMessageBox::StandardButton::Ok)
                return;

            result = Engine::DeleteWindowGroup(groupwId);
            if (result != Engine::OK) {
                String errorType, errorMessage;
                Engine::GetErrorDetail(errorType, errorMessage);
                QString msg(QStringLiteral("删除检测框组失败, 错误消息: "));
                msg += errorMessage.c_str();
                System->showMessage(QStringLiteral("检测框组"), msg);
                return;
            }

            ui.treeWidget->takeTopLevelItem(ui.treeWidget->indexOfTopLevelItem(pItem));
        }        
    }

    UpdateInspWindowList();    
}

void InspWindowWidget::on_btnCreateGroup_clicked() {
    QString strTitle(QStringLiteral("创建检测组"));
    auto listOfItems = ui.treeWidget->selectedItems();
    if (listOfItems.count() < 2) {
        System->showMessage(strTitle, QStringLiteral("请选择至少两个窗口!"));
        return;
    }

    for (const auto &pItem : listOfItems) {
        if (pItem->type() == TREE_ITEM_GROUP) {
            System->showMessage(strTitle, QStringLiteral("不能选择已经存在的group: ") + pItem->text(0));
            return;
        }

        if (pItem->parent() != NULL) {
            System->showMessage(strTitle, QStringLiteral("不能选择已经有分组的窗口: ") + pItem->text(0));
            return;
        }

        auto windowId = pItem->data(0, Qt::UserRole).toInt();
        auto window = m_mapIdWindow[windowId];
        if (window.usage != Engine::Window::Usage::HEIGHT_MEASURE && window.usage != Engine::Window::Usage::HEIGHT_BASE
            && window.usage != Engine::Window::Usage::INSP_POLARITY && window.usage != Engine::Window::Usage::INSP_POLARITY_REF) {
            System->showMessage(strTitle, QStringLiteral("当前只支持高度框和极性检测框分组!"));
            return;
        }
    }

    DialogCreateGroup dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;

    Engine::WindowGroup windowGroup;
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    windowGroup.deviceId = pUI->getSelectedDevice().getId();
    windowGroup.name = dialog.getGroupName().toStdString();
    for (const auto &pItem : listOfItems) {
        auto windowId = pItem->data(0, Qt::UserRole).toInt();
        windowGroup.vecWindows.push_back(m_mapIdWindow[windowId]);
    }
    auto result = Engine::CreateWindowGroup(windowGroup);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("创建检测框组失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return;
    }
    UpdateInspWindowList();
}

void InspWindowWidget::on_btnTryInsp_clicked() {
    if (INSP_WIDGET_INDEX::UNDEFINED == m_enCurrentInspWidget)
        return;

    if (INSP_WIDGET_INDEX::HEIGHT_DETECT == m_enCurrentInspWidget)
        _tryInspHeight();
    else if(INSP_WIDGET_INDEX::INSP_POLARITY == m_enCurrentInspWidget)
        _tryInspectPolarity();
    else
        m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->tryInsp();

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
}

void InspWindowWidget::_tryInspHeight() {
    QString strTitle(QStringLiteral("高度检测"));
    auto ptrCurrentItem = ui.treeWidget->currentItem();
    if (!ptrCurrentItem) {
        System->showMessage(strTitle, QStringLiteral("请先创建高度检测框组, 组里面包括高度检测框以及基准框."));
        return;
    }

    auto groupId = 0;
    if (ptrCurrentItem->type() == TREE_ITEM_GROUP)
        groupId = ptrCurrentItem->data(0, Qt::UserRole).toInt();
    else if (ptrCurrentItem->type() == TREE_ITEM_WINDOW) {
        auto pParent = ptrCurrentItem->parent();
        if (pParent != NULL)
            groupId = pParent->data(0, Qt::UserRole).toInt();
        else {
            System->showMessage(strTitle, QStringLiteral("请选择一个已经确定的高度检测框."));
            return;
        }
    }

    Engine::WindowGroup windowGroup;
    auto result = Engine::GetGroupWindows(groupId, windowGroup);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("读取检测框组失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return;
    }

    bool bHasCheckWindow = false, bHasBaseWindow = false;
    for (const auto &window : windowGroup.vecWindows) {
        if (Engine::Window::Usage::HEIGHT_MEASURE == window.usage)
            bHasCheckWindow = true;
        else if (Engine::Window::Usage::HEIGHT_BASE == window.usage)
            bHasBaseWindow = true;
    }

    if (!bHasCheckWindow) {
        System->showMessage(strTitle, QStringLiteral("分组内没有高度检测框!"));
        return;
    }

    if (!bHasBaseWindow) {
        System->showMessage(strTitle, QStringLiteral("分组内没有高度基准框!"));
        return;
    }

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->setWindowGroup(windowGroup);
    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->tryInsp();
}

void InspWindowWidget::_tryInspectPolarity() {
    QString strTitle(QStringLiteral("极性检测"));
    auto ptrCurrentItem = ui.treeWidget->currentItem();
    if (!ptrCurrentItem) {
        System->showMessage(strTitle, QStringLiteral("请选择一个已经确定的极性检测框."));
        return;
    }

    auto groupId = 0;
    if (ptrCurrentItem->type() == TREE_ITEM_GROUP)
        groupId = ptrCurrentItem->data(0, Qt::UserRole).toInt();
    else if (ptrCurrentItem->type() == TREE_ITEM_WINDOW) {
        auto pParent = ptrCurrentItem->parent();
        if (pParent != NULL)
            groupId = pParent->data(0, Qt::UserRole).toInt();
        else {
            System->showMessage(strTitle, QStringLiteral("请先创建极性检测框组, 组里面包括极性检测框以及参考框."));
            return;
        }
    }

    Engine::WindowGroup windowGroup;
    auto result = Engine::GetGroupWindows(groupId, windowGroup);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("读取检测框组失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return;
    }

    bool bHasCheckWindow = false, bHasBaseWindow = false;
    for (const auto &window : windowGroup.vecWindows) {
        if (Engine::Window::Usage::INSP_POLARITY == window.usage)
            bHasCheckWindow = true;
        else if (Engine::Window::Usage::INSP_POLARITY_REF == window.usage)
            bHasBaseWindow = true;
    }

    if (!bHasCheckWindow) {
        System->showMessage(strTitle, QStringLiteral("分组内没有极性检测框!"));
        return;
    }

    if (!bHasBaseWindow) {
        System->showMessage(strTitle, QStringLiteral("分组内没有极性检测参考框!"));
        return;
    }

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->setWindowGroup(windowGroup);
    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->tryInsp();
}

void InspWindowWidget::on_btnConfirmWindow_clicked() {
    if (INSP_WIDGET_INDEX::UNDEFINED == m_enCurrentInspWidget)
        return;

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->confirmWindow(m_enOperation);
}

void InspWindowWidget::onInspWindowState(const QVariantList &data) {
    UpdateInspWindowList();
    if (ui.treeWidget->topLevelItemCount() > 0) {
        ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount() - 1)->setSelected(true);
    }else {
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::UNDEFINED;

        IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
        pUI->setCurrentDetectObj(QDetectObj());
    }
}

void InspWindowWidget::onSelectedWindowChanged() {
    auto selectedItems = ui.treeWidget->selectedItems();
    if (selectedItems.size() <= 0) {
        _hideWidgets();
        return;
    }

    _showWidgets();

    auto modifiers = QApplication::keyboardModifiers();
    if (!modifiers.testFlag(Qt::ControlModifier) && selectedItems.size() > 1)
        ui.treeWidget->selectionModel()->clearSelection();

    Engine::Window window;
    auto pItem = ui.treeWidget->currentItem();
    if (NULL == pItem)
        pItem = selectedItems[0];

    if (pItem->type() == TREE_ITEM_WINDOW) {
        auto windowName = pItem->text(0).toStdString();
        auto windowId = pItem->data(0, Qt::UserRole).toInt();
        auto pParentItem = pItem->parent();
        if (NULL == pParentItem) {
            if (m_mapIdWindow.find(windowId) == m_mapIdWindow.end())
                return;
            window = m_mapIdWindow[windowId];
        }else {
            int index = ui.treeWidget->indexOfTopLevelItem(pParentItem);
            if (index < 0)
                return;

            int childIndex = pParentItem->indexOfChild(pItem);
            window = m_vecWindowGroup[index].vecWindows[childIndex];
        }
    }
    else {
        int index = ui.treeWidget->indexOfTopLevelItem(pItem);
        if (index < 0)
            return;

        if (m_vecWindowGroup[index].vecWindows.size() <= 0)
            return;

        window = m_vecWindowGroup[index].vecWindows[0];
    }

    if (pItem)
        pItem->setSelected(true);

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
    else if (Engine::Window::Usage::INSP_POLARITY == window.usage || Engine::Window::Usage::INSP_POLARITY_REF == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::INSP_POLARITY;
    else if (Engine::Window::Usage::INSP_CONTOUR == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::INSP_CONTOUR;
    else if (Engine::Window::Usage::INSP_CHIP == window.usage)
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::INSP_CHIP;
    else
        assert(0);

    ui.labelWindowName->setText(window.name.c_str());

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->setCurrentWindow(window);
    ui.stackedWidget->setCurrentIndex(static_cast<int>(m_enCurrentInspWidget));

    m_pComboBoxLighting->setCurrentIndex(window.lightId - 1);

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();

    auto x = window.x / dResolutionX;
    auto y = window.y / dResolutionY;
    if (bBoardRotated)
        x = m_nBigImgWidth  - x;
    else
        y = m_nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

    auto width  = window.width  / dResolutionX;
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

void InspWindowWidget::on_regrouped() {
    UpdateInspWindowList();
}