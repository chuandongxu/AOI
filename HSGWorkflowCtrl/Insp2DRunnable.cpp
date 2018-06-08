#include <QJsonObject>
#include <QJsonDocument>

#include "Insp2DRunnable.h"
#include "../include/constants.h"
#include "../DataModule/DataUtils.h"

#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IVisionUI.h"

Insp2DRunnable::Insp2DRunnable(const Vision::VectorOfMat    &vec2DImages,
                               const DeviceInspWindowVector &vecDeviceWindows,
                               const cv::Point2f            &ptFramePos,
                               BoardInspResultPtr           &ptrBoardInsResult) :
    m_vec2DImages      (vec2DImages),
    m_vecDeviceWindows (vecDeviceWindows),
    InspRunnable       (ptFramePos, ptrBoardInsResult) {
}

Insp2DRunnable::~Insp2DRunnable() {
}

void Insp2DRunnable::run() {
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

    default:
        break;
    }
}

bool Insp2DRunnable::_preprocessImage(const Engine::Window &window, cv::Mat &matOutput) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.colorParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
        return false;
    }

    if (! parse_doucment.isObject()) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
        return false;
    }

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
        return false;
    }

    cv::Mat matImage = m_vec2DImages[nImageIndex];
    auto rectROI = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        window.width,
        window.height,
        m_ptFramePos,
        m_nImageWidthPixel,
        m_nImageHeightPixel,
        m_dResolutionX,
        m_dResolutionY);

    QJsonObject obj = parse_doucment.object();

    auto enMethod = static_cast<GRAY_WEIGHT_METHOD>(obj["Method"].toInt());
    auto bEnableB = obj["EnableB"].toBool();
    auto bEnableG = obj["EnableG"].toBool();
    auto bEnableR = obj["EnableR"].toBool();
    auto nScaleB = obj["GrayScaleB"].toInt();
    auto nScaleG = obj["GrayScaleG"].toInt();
    auto nScaleR = obj["GrayScaleR"].toInt();
    auto nThreshold1 = obj["GrayThreshold1"].toInt();
    auto nThreshold2 = obj["GrayThreshold2"].toInt();

    Vision::PR_COLOR_TO_GRAY_CMD stCmd;
	Vision::PR_COLOR_TO_GRAY_RPY stRpy;

    stCmd.stRatio.fRatioR = bEnableR ? nScaleR / 100.0f : 0.f;
    stCmd.stRatio.fRatioG = bEnableG ? nScaleG / 100.0f : 0.f;
    stCmd.stRatio.fRatioB = bEnableB ? nScaleB / 100.0f : 0.f;
    stCmd.matInputImg = matImage;

    Vision::PR_ColorToGray(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
        return false;
    }

    matImage = stRpy.matResultImg;

    auto nRThreshold = obj["ColorRThreshold"].toInt();
    auto nTThreshold = obj["ColorTThreshold"].toInt();

    if (EM_MODE_ONE_THRESHOLD == enMethod || EM_MODE_TWO_THRESHOLD == enMethod) {
        Vision::PR_THRESHOLD_CMD stThresholdCmd;
        Vision::PR_THRESHOLD_RPY stThresholdRpy;
        stThresholdCmd.bDoubleThreshold = EM_MODE_TWO_THRESHOLD == enMethod;
        stThresholdCmd.matInputImg = cv::Mat(matImage, rectROI);
        stThresholdCmd.nThreshold1 = nThreshold1;
        stThresholdCmd.nThreshold2 = nThreshold2;

        Vision::PR_Threshold(&stThresholdCmd, &stThresholdRpy);
        matOutput = stThresholdRpy.matResultImg;
    }

    return true;
}

void Insp2DRunnable::_inspChip(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
		return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_CHIP_CMD stCmd;
    Vision::PR_INSP_CHIP_RPY stRpy;

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
        return;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectSrchWindow = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        jsonValue["SrchWinWidth"].toDouble(),
        jsonValue["SrchWinHeight"].toDouble(),
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
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspHole(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_findLine(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_findCircle(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
    m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

void Insp2DRunnable::_inspContour(const Engine::Window &window) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
        m_ptrBoardInspResult->addWindowStatus(windowInsp.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
    for (const auto &window : windowGroup.vecWindows)
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(stRpy.enStatus));
}

Vision::VisionStatus Insp2DRunnable::_alignment(const Engine::Window &window, DeviceInspWindow &deviceInspWindow) {
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
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
        m_ptrBoardInspResult->addWindowStatus(window.Id, Vision::ToInt32(Vision::VisionStatus::INVALID_PARAM));
        return Vision::VisionStatus::INVALID_PARAM;
    }

    stCmd.matInputImg = m_vec2DImages[nImageIndex];
    stCmd.rectSrchWindow = DataUtils::convertWindowToFrameRect(cv::Point2f(window.x, window.y),
        jsonValue["SrchWinWidth"].toDouble(),
        jsonValue["SrchWinHeight"].toDouble(),
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
    }
    return stRpy.enStatus;
}