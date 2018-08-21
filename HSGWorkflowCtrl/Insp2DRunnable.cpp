#include <QJsonObject>
#include <QJsonDocument>

#include "Insp2DRunnable.h"
#include "../include/constants.h"
#include "../DataModule/DataUtils.h"

#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IVisionUI.h"
#include "../include/IVision.h"
#include "../DataModule/CalcUtils.hpp"
#include "TimeLog.h"

Insp2DRunnable::Insp2DRunnable(
    const cv::Mat                &mat3DHeight,
    const Vision::VectorOfMat    &vec2DImages,
    const DeviceInspWindowVector &vecDeviceWindows,
    const cv::Point2f            &ptFramePos,
    BoardInspResultPtr           &ptrBoardInsResult) :
    m_mat3DHeight               (mat3DHeight),
    m_vec2DImages               (vec2DImages),
    m_vecDeviceWindows          (vecDeviceWindows),
    m_pThreadPoolCalc3D         (nullptr),
    InspRunnable                (ptFramePos, ptrBoardInsResult) {
}

Insp2DRunnable::Insp2DRunnable(const Vision::VectorOfMat            &vec2DCaptureImages,
                               Vision::VectorOfVectorOfMat          *pVecVecFrameImages,
                               QThreadPool                          *pThreadPoolCalc3D,
                               const VectorCalc3DHeightRunnablePtr  &vecCalc3DHeightRunnable,
                               Vision::VectorOfMat                  *pVec3DFrameImages,
                               const DeviceInspWindowVector         &vecDeviceWindows,
                               int                                   nRow,
                               int                                   nCol,
                               int                                   nTotalRows,
                               int                                   nTotalCols,
                               const cv::Point2f                    &ptFramePos,
                               BoardInspResultPtr                   &ptrBoardInsResult) :
    m_vec2DCaptureImages        (vec2DCaptureImages),
    m_pVecVecFrameImages        (pVecVecFrameImages),
    m_pThreadPoolCalc3D         (pThreadPoolCalc3D),
    m_vecCalc3DHeightRunnable   (vecCalc3DHeightRunnable),
    m_pVec3DFrameImages         (pVec3DFrameImages),
    m_vecDeviceWindows          (vecDeviceWindows),
    m_nRow                      (nRow),
    m_nCol                      (nCol),
    m_nTotalRows                (nTotalRows),
    m_nTotalCols                (nTotalCols),
    InspRunnable                (ptFramePos, ptrBoardInsResult) {
}

Insp2DRunnable::~Insp2DRunnable() {
}

void Insp2DRunnable::run() {
    _prepareImages();

    int nWindowIndex = 0;
    for(auto &deviceWindow : m_vecDeviceWindows)
    {
        auto iterAlignmentWindow = std::find_if(deviceWindow.vecUngroupedWindows.begin(), deviceWindow.vecUngroupedWindows.end(), [](const Engine::Window &window) { return window.usage == Engine::Window::Usage::ALIGNMENT; });
        if (iterAlignmentWindow != deviceWindow.vecUngroupedWindows.end()) {
            if ( _alignment(*iterAlignmentWindow, deviceWindow) != Vision::VisionStatus::OK) {
                deviceWindow.bGood = false;
                deviceWindow.bAlignmentPassed = false;
                continue;
            }
        }

        for (const auto &window : deviceWindow.vecUngroupedWindows) {
            if (window.usage != Engine::Window::Usage::ALIGNMENT)
                _inspWindow(window);
        }

        for (const auto &windowGroup : deviceWindow.vecWindowGroup) {
            auto iterHeightCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::INSP_POLARITY == window.usage; });
            if (iterHeightCheckWindow != windowGroup.vecWindows.end()) {
                _inspPolarityGroup(windowGroup);
            }
        }
        m_ptrBoardInspResult->addDeviceInspWindow(deviceWindow);
    }
}

void Insp2DRunnable::_inspWindow(const Engine::Window &window)
{
    switch (window.usage) {
    case Engine::Window::Usage::INSP_CHIP:
        _inspChip(window);
        break;

    case Engine::Window::Usage::INSP_HOLE:
        _inspHole(window);
        break;

    case Engine::Window::Usage::FIND_LINE:
        _findLine(window);
        break;

    case Engine::Window::Usage::FIND_CIRCLE:
        _findCircle(window);
        break;

    case Engine::Window::Usage::INSP_CONTOUR:
        _inspContour(window);
        break;

    case Engine::Window::Usage::INSP_BRIDGE:
        _inspBridge(window);
        break;

    case Engine::Window::Usage::INSP_LEAD:
        _inspLead(window);
        break;

    case Engine::Window::Usage::OCV:
        _ocv(window);
        break;

    default:
        break;
    }
}

bool Insp2DRunnable::_preprocessImage(const Engine::Window &window, cv::Mat &matOutput) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.colorParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        std::string strErrorMsg = "Window \"" + window.name + "\" color parameters \"" + window.colorParams + "\" is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        return false;
    }

    if (! parse_doucment.isObject()) {
        m_ptrBoardInspResult->setFatalError();
        return false;
    }

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return false;
    }

    QJsonObject jsonObj = parse_doucment.object();

    cv::Mat matImage = m_vec2DImages[nImageIndex];
    bool bUseSrchWidth = false;
    if (Engine::Window::Usage::INSP_BRIDGE == window.usage) {
        if (window.srchWidth > window.width && window.srchHeight > window.height)
            bUseSrchWidth = true;
    }

    float fWidth = window.width, fHeight = window.height;
    if (bUseSrchWidth) {
        fWidth  = window.srchWidth;
        fHeight = window.srchHeight;
    }

    auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        fWidth,
        fHeight,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);
    cv::Mat matROI(matImage, rectROI);

    cv::Mat matColor;
    if (matROI.type() == CV_8UC1)
        cv::cvtColor(matROI, matColor, CV_GRAY2BGR);
    else if (matROI.type() == CV_8UC3)
        matColor = matROI;    

    auto enMethod = static_cast<GRAY_WEIGHT_METHOD>(jsonObj["Method"].toInt());

    if (GRAY_WEIGHT_METHOD::EM_MODE_PT_THRESHOLD == enMethod) {
        cv::Point ptPick;
        ptPick.x = jsonObj["PickPointX"].toInt();
        ptPick.y = jsonObj["PickPointY"].toInt();
        auto nRThreshold = jsonObj["ColorRThreshold"].toInt();
        auto nTThreshold = jsonObj["ColorTThreshold"].toInt();

        Vision::PR_PICK_COLOR_CMD stCmd;
        Vision::PR_PICK_COLOR_RPY stRpy;        

        stCmd.matInputImg = matColor;
        stCmd.rectROI = cv::Rect(0, 0, rectROI.width, rectROI.height);
        stCmd.ptPick = ptPick;
        stCmd.nColorDiff = nRThreshold;
        stCmd.nGrayDiff = nTThreshold;
        
        Vision::PR_PickColor(&stCmd, &stRpy);
        if (Vision::VisionStatus::OK != stRpy.enStatus) {
            Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
            Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
            if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
                std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
                m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
                m_ptrBoardInspResult->setFatalError();
            }
            return false;
        }
        matOutput = stRpy.matResultImg;
    }
    else {
        auto bEnableB = jsonObj["EnableB"].toBool();
        auto bEnableG = jsonObj["EnableG"].toBool();
        auto bEnableR = jsonObj["EnableR"].toBool();
        auto nScaleB = jsonObj["GrayScaleB"].toInt();
        auto nScaleG = jsonObj["GrayScaleG"].toInt();
        auto nScaleR = jsonObj["GrayScaleR"].toInt();
        auto nThreshold1 = jsonObj["GrayThreshold1"].toInt();
        auto nThreshold2 = jsonObj["GrayThreshold2"].toInt();

        Vision::PR_COLOR_TO_GRAY_CMD stCmd;
        Vision::PR_COLOR_TO_GRAY_RPY stRpy;

        stCmd.stRatio.fRatioR = bEnableR ? nScaleR / 100.0f : 0.f;
        stCmd.stRatio.fRatioG = bEnableG ? nScaleG / 100.0f : 0.f;
        stCmd.stRatio.fRatioB = bEnableB ? nScaleB / 100.0f : 0.f;
        stCmd.matInputImg = matColor;

        Vision::PR_ColorToGray(&stCmd, &stRpy);
        if (Vision::VisionStatus::OK != stRpy.enStatus) {
            Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
            Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
            if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
                std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
                m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
                m_ptrBoardInspResult->setFatalError();
            }
            return false;
        }

        cv::Mat matGray = stRpy.matResultImg;

        Vision::PR_THRESHOLD_CMD stThresholdCmd;
        Vision::PR_THRESHOLD_RPY stThresholdRpy;
        stThresholdCmd.bDoubleThreshold = EM_MODE_TWO_THRESHOLD == enMethod;
        stThresholdCmd.matInputImg = matGray;
        stThresholdCmd.nThreshold1 = nThreshold1;
        stThresholdCmd.nThreshold2 = nThreshold2;

        Vision::PR_Threshold(&stThresholdCmd, &stThresholdRpy);
        if (Vision::VisionStatus::OK != stRpy.enStatus) {
            Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
            Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
            if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
                std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
                m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
                m_ptrBoardInspResult->setFatalError();
            }
            return false;
        }
        matOutput = stThresholdRpy.matResultImg;
    }

    return true;
}

void Insp2DRunnable::_inspChip(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
		return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_CHIP_CMD stCmd;
    Vision::PR_INSP_CHIP_RPY stRpy;

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectSrchWindow = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.srchWidth,
        window.srchHeight,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    stCmd.enInspMode = static_cast<Vision::PR_INSP_CHIP_MODE>(jsonValue["InspMode"].toInt()); 

    Vision::PR_InspChip(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspHole(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_HOLE_CMD stCmd;
    Vision::PR_INSP_HOLE_RPY stRpy;

    stCmd.enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(jsonValue["InspMode"].toInt());

    // The preprocessed image returns the image of ROI.
    if (! _preprocessImage(window, stCmd.matInputImg))
        return;
    
    stCmd.bPreprocessedImg = true;
    stCmd.rectROI = cv::Rect(0, 0, stCmd.matInputImg.cols, stCmd.matInputImg.rows);
    if (Vision::PR_INSP_HOLE_MODE::RATIO == stCmd.enInspMode) {
        QJsonObject jsonRatioMode = jsonValue["RatioMode"].toObject();
        stCmd.stRatioModeCriteria.fMaxRatio = jsonRatioMode["MaxAreaRatio"].toDouble() / ONE_HUNDRED_PERCENT;
        stCmd.stRatioModeCriteria.fMinRatio = jsonRatioMode["MinAreaRatio"].toDouble() / ONE_HUNDRED_PERCENT;
    }else {
        QJsonObject jsonBlobMode = jsonValue["BlobMode"].toObject();
        stCmd.stBlobModeCriteria.nMaxBlobCount = jsonBlobMode["MaxHoleCount"].toInt();
        stCmd.stBlobModeCriteria.nMinBlobCount = jsonBlobMode["MinHoleCount"].toInt();
        stCmd.stBlobModeCriteria.fMaxArea = jsonBlobMode["MaxHoleArea"].toDouble();
        stCmd.stBlobModeCriteria.fMinArea = jsonBlobMode["MinHoleArea"].toDouble();
    }

    Vision::PR_InspHole(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_findLine(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        std::string strErrorMsg = "Window \"" + window.name + "\" inspect parameters \"" + window.colorParams + "\" is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_FIND_LINE_CMD stCmd;
    Vision::PR_FIND_LINE_RPY stRpy;

    stCmd.enAlgorithm = Vision::PR_FIND_LINE_ALGORITHM::CALIPER;
    stCmd.bFindPair = jsonValue["FindPair"].toBool();
    stCmd.enDetectDir = static_cast<Vision::PR_CALIPER_DIR>(jsonValue["DetectDir"].toInt());
    stCmd.nCaliperCount = jsonValue["CaliperCount"].toInt();
    stCmd.fCaliperWidth = jsonValue["CaliperWidth"].toDouble();
    stCmd.nEdgeThreshold = jsonValue["EdgeThreshold"].toInt();
    stCmd.enSelectEdge = static_cast<Vision::PR_CALIPER_SELECT_EDGE>(jsonValue["SelectEdge"].toInt());
    stCmd.fRmStrayPointRatio = jsonValue["RmStrayPointRatio"].toDouble();
    stCmd.nDiffFilterHalfW = jsonValue["DiffFilterHalfW"].toInt();
    stCmd.fDiffFilterSigma = jsonValue["DiffFilterSigma"].toDouble();
    stCmd.bCheckLinearity = jsonValue["CheckLinerity"].toBool();
    stCmd.fPointMaxOffset = jsonValue["PointMaxOffset"].toDouble() / m_dResolutionX;
    stCmd.fMinLinearity = jsonValue["MinLinearity"].toDouble();
    stCmd.bCheckAngle = jsonValue["CheckAngle"].toBool();
    stCmd.fExpectedAngle = jsonValue["ExpectedAngle"].toDouble();
    stCmd.fAngleDiffTolerance = jsonValue["AngleDiffTolerance"].toDouble();

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    stCmd.rectRotatedROI.center = cv::Point2f(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2);
    stCmd.rectRotatedROI.size = rectROI.size();
    Vision::PR_FindLine(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_findCircle(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_FIND_CIRCLE_CMD stCmd;
    Vision::PR_FIND_CIRCLE_RPY stRpy;

    stCmd.enInnerAttribute = static_cast<Vision::PR_OBJECT_ATTRIBUTE>(jsonValue["InnerAttri"].toInt());
	stCmd.bFindCirclePair = jsonValue["FindPair"].toBool();
	stCmd.fStartSrchAngle = jsonValue["SearchStartAngle"].toDouble();
	stCmd.fEndSrchAngle = jsonValue["SearchEndAngle"].toDouble();
	stCmd.nCaliperCount = jsonValue["CaliperCount"].toInt();
	stCmd.fCaliperWidth = jsonValue["CaliperWidth"].toDouble();
	stCmd.nEdgeThreshold = jsonValue["EdgeThreshold"].toInt();
	stCmd.enSelectEdge = static_cast<Vision::PR_CALIPER_SELECT_EDGE>(jsonValue["SelectEdge"].toInt());
	stCmd.fRmStrayPointRatio = jsonValue["RmStrayPointRatio"].toDouble();
	stCmd.nDiffFilterHalfW = jsonValue["DiffFilterHalfW"].toInt();
	stCmd.fDiffFilterSigma = jsonValue["DiffFilterSigma"].toDouble();

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    stCmd.ptExpectedCircleCtr = cv::Point2f(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2);
    stCmd.fMaxSrchRadius = qMin(rectROI.width / 2.0f, rectROI.height / 2.0f);
	stCmd.fMinSrchRadius = qMin(rectROI.width / 2.0f, rectROI.height / 2.0f) / 3.0f;

    Vision::PR_FindCircle(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspContour(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_CONTOUR_CMD stCmd;
    Vision::PR_INSP_CONTOUR_RPY stRpy;

    stCmd.nDefectThreshold = jsonValue["DefectThreshold"].toInt();
    stCmd.fMinDefectArea = jsonValue["MinDefectArea"].toDouble() / m_dResolutionX / m_dResolutionY;
    stCmd.fDefectInnerLengthTol = jsonValue["DefectInnerLengthTol"].toDouble() / m_dResolutionX;
    stCmd.fDefectOuterLengthTol = jsonValue["DefectOuterLengthTol"].toDouble() / m_dResolutionX;
    stCmd.fInnerMaskDepth = jsonValue["InnerMaskDepth"].toDouble() / m_dResolutionX;
    stCmd.fOuterMaskDepth = jsonValue["OuterMaskDepth"].toDouble() / m_dResolutionX;
    stCmd.nRecordId = window.recordId;

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    Vision::PR_InspContour(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspPolarityGroup(const Engine::WindowGroup &windowGroup) {
    auto iterPolarityCheckWindow = std::find_if(windowGroup.vecWindows.begin(), windowGroup.vecWindows.end(), [](const Engine::Window &window) { return Engine::Window::Usage::INSP_POLARITY == window.usage; });
    auto windowInsp = *iterPolarityCheckWindow;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(windowInsp.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(windowInsp.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_POLARITY_CMD stCmd;
	Vision::PR_INSP_POLARITY_RPY stRpy;

    int nImageIndex = windowInsp.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + windowInsp.name + "\" image id " + std::to_string(windowInsp.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];

	stCmd.enInspROIAttribute = static_cast<Vision::PR_OBJECT_ATTRIBUTE>(jsonValue["Attribute"].toInt());
	stCmd.nGrayScaleDiffTol = jsonValue["IntensityDiffTol"].toInt();

	stCmd.rectInspROI = DataUtils::convertWindowToFrameRect(cv::Point2f(windowInsp.x, windowInsp.y),
        windowInsp.width,
        windowInsp.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    for (const auto &window : windowGroup.vecWindows) {
        if (Engine::Window::Usage::INSP_POLARITY_REF == window.usage) {
            stCmd.rectCompareROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
                window.width,
                window.height,
                m_ptFramePos,
                m_nImageWidthPixel,
                m_nImageHeightPixel,
                m_dResolutionX,
                m_dResolutionY);
            break;
        }
    }

	Vision::PR_InspPolarity(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window group \"" + windowGroup.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    for (const auto &window : windowGroup.vecWindows)
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspBridge(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_BRIDGE_CMD stCmd;
    Vision::PR_INSP_BRIDGE_RPY stRpy;

    stCmd.enInspMode = static_cast<Vision::PR_INSP_BRIDGE_MODE>(jsonValue["InspMode"].toInt());

    // The preprocessed image returns the image of ROI.
    if (! _preprocessImage(window, stCmd.matInputImg))
        return;
    
    if (Vision::PR_INSP_BRIDGE_MODE::INNER == stCmd.enInspMode) {
        QJsonObject jsonInnerMode = jsonValue["InnerMode"].toObject();
        stCmd.rectROI = cv::Rect(0, 0, stCmd.matInputImg.cols, stCmd.matInputImg.rows);
        stCmd.stInnerInspCriteria.fMaxLengthX = jsonInnerMode["MaxWidth"].toDouble();
        stCmd.stInnerInspCriteria.fMaxLengthY = jsonInnerMode["MaxHeight"].toDouble();
    } else {
        stCmd.rectOuterSrchWindow = cv::Rect(0, 0, stCmd.matInputImg.cols, stCmd.matInputImg.rows);
        stCmd.rectROI = CalcUtils::resizeRect(stCmd.rectOuterSrchWindow, cv::Size(window.width / m_dResolutionX, window.height / m_dResolutionY));
        QJsonObject jsonOuterMode = jsonValue["OuterMode"].toObject();
        if (jsonOuterMode["CheckLeft"].toBool())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::LEFT);
        if (jsonOuterMode["CheckRight"].toBool())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::RIGHT);
        if (jsonOuterMode["CheckUp"].toBool())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::UP);
        if (jsonOuterMode["CheckDown"].toBool())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::DOWN);
    }

    Vision::PR_InspBridge(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspLead(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_LEAD_TMPL_CMD stCmd;
    Vision::PR_INSP_LEAD_TMPL_RPY stRpy;

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);
    stCmd.nPadRecordId = jsonValue["PadRecordId"].toInt();
    stCmd.nLeadRecordId = jsonValue["LeadRecordId"].toInt();
    stCmd.fLrnedPadLeadDist = jsonValue["PadLeadDist"].toDouble() / m_dResolutionX;
    stCmd.fMaxLeadOffsetX = jsonValue["MaxOffsetX"].toDouble() / m_dResolutionX;
    stCmd.fMaxLeadOffsetY = jsonValue["MaxOffsetY"].toDouble() / m_dResolutionX;

    Vision::PR_InspLeadTmpl(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_ocv(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_OCV_CMD stCmd;
    Vision::PR_OCV_RPY stRpy;

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);
    auto strRecordList = jsonValue["RecordList"].toString();
    auto datalist = strRecordList.split(',');
    for (const auto &strRecordId : datalist) {
        stCmd.vecRecordId.push_back(strRecordId.toInt());
    }
    stCmd.fMinMatchScore = jsonValue["MinScore"].toDouble();

    Vision::PR_Ocv(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
            return;
        }
    }
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

Vision::VisionStatus Insp2DRunnable::_alignment(const Engine::Window &window, DeviceInspWindow &deviceInspWindow) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->setFatalError();
        return Vision::VisionStatus::INVALID_PARAM;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_MATCH_TEMPLATE_CMD stCmd;
    Vision::PR_MATCH_TEMPLATE_RPY stRpy;

    stCmd.enAlgorithm = static_cast<Vision::PR_MATCH_TMPL_ALGORITHM>(jsonValue["Algorithm"].toInt());
    stCmd.bSubPixelRefine = jsonValue["SubPixel"].toBool();
    stCmd.enMotion = static_cast<Vision::PR_OBJECT_MOTION>(jsonValue["Motion"].toInt());
    stCmd.fMinMatchScore = jsonValue["MinScore"].toDouble();
    stCmd.nRecordId = window.recordId;

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        std::string strErrorMsg = "Window \"" + window.name + "\" image id " + std::to_string(window.lightId) + " is invalid.";
        m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
        m_ptrBoardInspResult->setFatalError();
        return Vision::VisionStatus::INVALID_PARAM;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectSrchWindow = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.srchWidth,
        window.srchHeight,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    if (stCmd.rectSrchWindow.x < 0) stCmd.rectSrchWindow.x = 0;
    if (stCmd.rectSrchWindow.y < 0) stCmd.rectSrchWindow.y = 0;
    if ((stCmd.rectSrchWindow.x + stCmd.rectSrchWindow.width) > stCmd.matInputImg.cols)
        stCmd.rectSrchWindow.width = stCmd.matInputImg.cols - stCmd.rectSrchWindow.x;
    if ((stCmd.rectSrchWindow.y + stCmd.rectSrchWindow.height) > stCmd.matInputImg.rows)
        stCmd.rectSrchWindow.height = stCmd.matInputImg.rows - stCmd.rectSrchWindow.y;

    Vision::PR_MatchTmpl(&stCmd, &stRpy);
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
    if (Vision::VisionStatus::OK == stRpy.enStatus) {
        float fRealPosX = m_ptFramePos.x + (stRpy.ptObjPos.x - m_nImageWidthPixel  / 2) * m_dResolutionX;
        float fRealPosY = m_ptFramePos.y - (stRpy.ptObjPos.y - m_nImageHeightPixel / 2) * m_dResolutionX;
        float fOffsetX = fRealPosX - window.x;
        float fOffsetY = fRealPosY - window.y;

        for (auto &window : deviceInspWindow.vecUngroupedWindows) {
            window.x += fOffsetX;
            window.y += fOffsetY;
        }

        for (auto &windowGroup : deviceInspWindow.vecWindowGroup) {
            for (auto &window : windowGroup.vecWindows) {
                window.x += fOffsetX;
                window.y += fOffsetY;
            }
        }
    }else {
        Vision::PR_GET_ERROR_INFO_RPY stGetErrInfoRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stGetErrInfoRpy);
        if (Vision::PR_STATUS_ERROR_LEVEL::PR_FATAL_ERROR == stGetErrInfoRpy.enErrorLevel) {
            std::string strErrorMsg = "Window \"" + window.name + "\" inspect failed with error: " + stGetErrInfoRpy.achErrorStr;
            m_ptrBoardInspResult->setErrorMsg(strErrorMsg.c_str());
            m_ptrBoardInspResult->setFatalError();
        }
    }

    return stRpy.enStatus;
}

Vision::VectorOfMat Insp2DRunnable::_generate2DImages(const Vision::VectorOfMat &vecInputImages)
{
    assert(vecInputImages.size() == CAPTURE_2D_IMAGE_SEQUENCE::TOTAL_COUNT);

    Vision::VectorOfMat vecResultImages;
    cv::Mat matRed, matGreen, matBlue;
    bool bColorCamera = false;
    if (bColorCamera) {
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::RED_LIGHT],   matRed,   CV_BayerGR2GRAY);
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::GREEN_LIGHT], matGreen, CV_BayerGR2GRAY);
        cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::BLUE_LIGHT],  matBlue,  CV_BayerGR2GRAY);
    }else {
        matRed   = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::RED_LIGHT];
        matGreen = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::GREEN_LIGHT];
        matBlue  = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::BLUE_LIGHT];
    }

    Vision::VectorOfMat vecChannels{matBlue, matGreen, matRed};
    cv::Mat matPseudoColorImage;
    cv::merge(vecChannels, matPseudoColorImage);

    cv::Mat matTopLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::WHITE_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::WHITE_LIGHT], matTopLightImage, CV_BayerGR2BGR);
	vecResultImages.push_back(matTopLightImage);

    cv::Mat matLowAngleLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::LOW_ANGLE_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::LOW_ANGLE_LIGHT], matLowAngleLightImage, CV_BayerGR2BGR);
    vecResultImages.push_back(matLowAngleLightImage);

    vecResultImages.push_back(matPseudoColorImage);

	cv::Mat matUniformLightImage = vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::UNIFORM_LIGHT];
    if (bColorCamera)
	    cv::cvtColor(vecInputImages[CAPTURE_2D_IMAGE_SEQUENCE::UNIFORM_LIGHT], matUniformLightImage, CV_BayerGR2BGR);
	vecResultImages.push_back(matUniformLightImage);

    return vecResultImages;
}

void Insp2DRunnable::_prepareImages() {
    if (m_pThreadPoolCalc3D != nullptr) {
        m_vec2DImages = _generate2DImages(m_vec2DCaptureImages);
        for (int i = 0; i < 4; ++i)
            (*m_pVecVecFrameImages)[i][m_nRow * m_nTotalCols + m_nCol] = m_vec2DImages[i];

        m_pThreadPoolCalc3D->waitForDone();
        TimeLogInstance->addTimeLog(std::string("Finished wait for 3D calculation done in thead ") + QThread::currentThread()->objectName().toStdString());
        if (!m_vecCalc3DHeightRunnable.empty()) {
            QVector<cv::Mat> vecMatHeight;
            for (const auto &ptrCalc3DHeightRunnable : m_vecCalc3DHeightRunnable)
                vecMatHeight.push_back(ptrCalc3DHeightRunnable->get3DHeight());

            IVision* pVision = getModule<IVision>(VISION_MODEL);
            if (!pVision) return;

            pVision->setInspect3DHeight(vecMatHeight, m_nRow, m_nCol, m_nTotalRows, m_nTotalCols);
            pVision->merge3DHeight(vecMatHeight, m_mat3DHeight, m_ptFramePos);
            (*m_pVec3DFrameImages)[m_nRow * m_nTotalCols + m_nCol] = m_mat3DHeight;
        }
    }    

    Vision::PR_HEIGHT_TO_GRAY_CMD stCmd;
    Vision::PR_HEIGHT_TO_GRAY_RPY stRpy;
    stCmd.matHeight = m_mat3DHeight;
    Vision::PR_HeightToGray(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus) {
        m_vec2DImages.push_back(stRpy.matGray);
    }
}