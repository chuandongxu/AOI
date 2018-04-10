#include <QJsonObject>
#include <QJsonDocument>

#include "Insp2DRunnable.h"
#include "../include/constants.h"

Insp2DRunnable::Insp2DRunnable(const Vision::VectorOfMat &vec2DImages, const Engine::WindowVector &vecWindows) :
    m_vec2DImages(vec2DImages),
    m_vecWindows(vecWindows)
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

bool Insp2DRunnable::_preprocessImage(const cv::Mat &matInput, const Engine::Window &window, cv::Mat &matOutput)
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

    QJsonObject obj = parse_doucment.object();

    auto enMethod = static_cast<GRAY_WEIGHT_METHOD>(obj["Method"].toInt());
    auto bEnableB = obj["EnableB"].toBool();
    auto bEnableG = obj["EnableG"].toBool();
    auto bEnableR = obj["EnableR"].toBool();
    auto nBScale = obj["GrayScaleB"].toInt();
    auto nGScale = obj["GrayScaleG"].toInt();
    auto nRScale = obj["GrayScaleR"].toInt();
    auto nThreshold1 = obj["GrayThreshold1"].toInt();
    auto nThreshold2 = obj["GrayThreshold2"].toInt();

    auto nRThreshold = obj["ColorRThreshold"].toInt();
    auto nTThreshold = obj["ColorTThreshold"].toInt();

    Vision::PR_THRESHOLD_CMD stThresholdCmd;
    Vision::PR_THRESHOLD_RPY stThreadHoldRpy;
    //stThresholdCmd.matInputImg = 

    switch (enMethod) {
    case EM_MODE_PT_THRESHOLD:
        break;
    case EM_MODE_ONE_THRESHOLD:
        break;
    case EM_MODE_TWO_THRESHOLD:
        break;
    default:
        break;
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