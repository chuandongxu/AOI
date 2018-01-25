#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <string>

//DataVariables is a singleton class to pass variables.
class DataVariables
{
private:
    DataVariables() {}   //No constructor exposure.
    DataVariables &operator=(DataVariables const &) = delete; //No assign exposure.

public:
    static DataVariables *GetInstance();
    void setFrameImageFolder(const std::string &strFrameImageFolder);
    std::string getFrameImageFolder() const;
    void setCountOfImgPerFrame(int nCountOfImgPerFrame);
    int  getCountOfImgPerFrame() const;
    void setOverlapX(int nOverlapX);
    int  getOverlapX() const;
    void setOverlapY(int nOverlapY);
    int  getOverlapY() const;
    void setFrameCountX(int nFrameCountX);
    int  getFrameCountX() const;
    void setFrameCountY(int nFrameCountY);
    int  getFrameCountY() const;
    void setCountOfImgPerRow(int nCountOfImgPerRow);
    int  getCountOfImgPerRow() const;
    void setCombinedImgScale(float fCombinedImgScale);
    float getCombinedImgScale() const;
private:
    std::string     m_strFrameImageFolder;
    int             m_nCountOfImgPerFrame;
    int             m_nOverlapX;
    int             m_nOverlapY;
    int             m_nFrameCountX;
    int             m_nFrameCountY;
    int             m_nCountOfImgPerRow;
    float           m_fCombinedImgScale;
private:
    static std::atomic<DataVariables *> m_pInstance;
    static std::mutex                   m_mutex;
};

#define DataVariablesInstance   (DataVariables::GetInstance())
