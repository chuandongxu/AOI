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
    __int64                getId() const        { return m_nId; };
    const std::string     &getName() const      { return m_strName; }
    const cv::RotatedRect &getWindow() const    { return m_rrWindow; }
private:
    __int64             m_nId;
    std::string         m_strName;
    cv::RotatedRect     m_rrWindow;
};

using VisionViewDeviceVector = std::vector<VisionViewDevice>;