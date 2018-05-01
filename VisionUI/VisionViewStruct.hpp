#pragma once
#include <string>
#include <vector>
#include "opencv/cv.h"

class VisionViewDevice
{
public:
    VisionViewDevice() : m_nId(0), m_strName(""), m_rrWindow() {}
    VisionViewDevice(const VisionViewDevice&) = default;
    VisionViewDevice(__int64 Id, const std::string &name, cv::RotatedRect &rrWindow) : m_nId(Id), m_strName(name), m_rrWindow(rrWindow) {}
    VisionViewDevice &operator=(const VisionViewDevice&) = default;
    inline __int64                getId()       const    { return m_nId; };
    inline const std::string     &getName()     const    { return m_strName; }
    inline const cv::RotatedRect &getWindow()   const    { return m_rrWindow; }
private:
    __int64             m_nId;
    std::string         m_strName;
    cv::RotatedRect     m_rrWindow;
};

using VisionViewDeviceVector = std::vector<VisionViewDevice>;

class VisionViewFM
{
public:
    VisionViewFM() = default;
    VisionViewFM(const VisionViewFM&) = default;
    VisionViewFM(__int64 Id, const cv::Rect &rectFM, cv::Rect &rectSrchWindow) : m_nId(Id), m_rectFM(rectFM), m_rectSrchWindow(rectSrchWindow) {}
    inline __int64          getId()         const   { return m_nId; }
    inline const cv::Rect  &getFM()         const   { return m_rectFM; }
    inline const cv::Rect  &getSrchWindow() const   { return m_rectSrchWindow; }
    inline void setFM(const cv::Rect &rect)         { m_rectFM = rect; }
    inline void setSrchWindow(const cv::Rect &rect) { m_rectSrchWindow = rect; }
private:
    __int64         m_nId;
    cv::Rect        m_rectFM;
    cv::Rect        m_rectSrchWindow;
};

using VisionViewFMVector = std::vector<VisionViewFM>;
