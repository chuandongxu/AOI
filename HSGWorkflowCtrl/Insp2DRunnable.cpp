#include <QJsonObject>
#include <QJsonDocument>

#include "Insp2DRunnable.h"
#include "../include/constants.h"
#include "../DataModule/DataUtils.h"

Insp2DRunnable::Insp2DRunnable(const Vision::VectorOfMat &vec2DImages, const Engine::WindowVector &vecWindows, const cv::Point2f &ptFramePos) :
    m_vec2DImages(vec2DImages),
    m_vecWindows(vecWindows),
    m_ptFramePos(ptFramePos)
{
}

Insp2DRunnable::~Insp2DRunnable() {
}

void Insp2DRunnable::run()
{
    int nWindowIndex = 0;
    for(const auto &window : m_vecWindows)
    {
        switch (window.usage) {
        case Engine::Window::Usage::INSP_HOLE:
            _inspHole(window);
            break;

        case Engine::Window::Usage::FIND_LINE:
            _findLine(window);
            break;

        default:
            break;
        }
        ++ nWindowIndex;
    }
}

bool Insp2DRunnable::_preprocessImage(const Engine::Window &window, cv::Mat &matOutput)
{
    QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.colorParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        m_mapWindowStatus[window.Id] = Vision::VisionStatus::INVALID_PARAM;
        return false;
    }

    if (! parse_doucment.isObject()) {
        m_mapWindowStatus[window.Id] = Vision::VisionStatus::INVALID_PARAM;
        return false;
    }

    int nImageIndex = window.lightId - 1;
    if (nImageIndex < 0 || nImageIndex >= m_vec2DImages.size()) {
        m_mapWindowStatus[window.Id] = Vision::VisionStatus::INVALID_PARAM;
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
        m_mapWindowStatus[window.Id] = stRpy.enStatus;
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

void Insp2DRunnable::_inspHole(const Engine::Window &window)
{
    QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
	if (json_error.error != QJsonParseError::NoError) {
        m_mapWindowStatus[window.Id] = Vision::VisionStatus::INVALID_PARAM;
		return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    Vision::PR_INSP_HOLE_CMD stCmd;
    Vision::PR_INSP_HOLE_RPY stRpy;

    stCmd.enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(jsonValue["InspMode"].toInt());

    // The preprocessed image returns the image of ROI, so 
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

    m_mapWindowStatus[window.Id] = Vision::PR_InspHole(&stCmd, &stRpy);
}

void Insp2DRunnable::_findLine(const Engine::Window &window) {
}