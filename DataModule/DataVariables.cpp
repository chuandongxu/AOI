#include "DataVariables.h"

std::atomic<DataVariables*> DataVariables::m_pInstance = nullptr;
std::mutex DataVariables::m_mutex;

/*static*/ DataVariables *DataVariables::GetInstance() {
    if ( m_pInstance.load() == nullptr ) {
        std::lock_guard<std::mutex> lock( m_mutex );
        if( m_pInstance.load() == nullptr) {
            m_pInstance = new DataVariables();
        }
    }
    return m_pInstance;
}

void DataVariables::setFrameImageFolder(const std::string &strFrameImageFolder) {
    m_strFrameImageFolder = strFrameImageFolder;
}

void DataVariables::setCountOfImgPerFrame(int nCountOfImgPerFrame) {
    m_nCountOfImgPerFrame = nCountOfImgPerFrame;
}

int DataVariables::getCountOfImgPerFrame() const {
    return m_nCountOfImgPerFrame;
}

std::string DataVariables::getFrameImageFolder() const {
    return m_strFrameImageFolder;
}

void DataVariables::setOverlapX(int nOverlapX) {
    m_nOverlapX = nOverlapX;
}

int DataVariables::getOverlapX() const {
    return m_nOverlapX;
}

void DataVariables::setOverlapY(int nOverlapY) {
    m_nOverlapY = nOverlapY;
}

int DataVariables::getOverlapY() const {
    return m_nOverlapY;
}

void DataVariables::setFrameCountX(int nFrameCountX) {
    m_nFrameCountX = nFrameCountX;
}

int DataVariables::getFrameCountX() const {
    return m_nFrameCountX;
}

void DataVariables::setFrameCountY(int nFrameCountY) {
    m_nFrameCountY = nFrameCountY;
}

int DataVariables::getFrameCountY() const {
    return m_nFrameCountY;
}

void DataVariables::setCountOfImgPerRow(int nCountOfImgPerRow) {
    m_nCountOfImgPerRow = nCountOfImgPerRow;
}

int DataVariables::getCountOfImgPerRow() const {
    return m_nCountOfImgPerRow;
}

void DataVariables::setCombinedImgScale(float fCombinedImgScale) {
    m_fCombinedImgScale = fCombinedImgScale;
}

float DataVariables::getCombinedImgScale() const {
    return m_fCombinedImgScale;
}