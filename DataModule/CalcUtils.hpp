#ifndef _CALC_UTILS_H_
#define _CALC_UTILS_H_

class CalcUtils
{
public:
    template<typename T>
    static inline cv::Rect_<T> scaleRect(const cv::Rect_<T> &rectInput, float fScale)
    {
        return cv::Rect_<T> (cv::Point_<T>(rectInput.x + ( 1.f - fScale ) * rectInput.width, rectInput.y + ( 1.f - fScale ) * rectInput.height ),
            cv::Point_<T>(rectInput.x + ( 1.f + fScale ) * rectInput.width, rectInput.y + ( 1.f + fScale ) * rectInput.height ) );
    }

    template<typename T>
    static inline cv::Rect_<T> resizeRect(const cv::Rect_<T> &rectInput, cv::Size_<T> szNew)
    {
        return cv::Rect_<T> (rectInput.x + rectInput.width / 2.f - szNew.width / 2, rectInput.y + rectInput.height / 2.f  - szNew.height / 2,
            szNew.width, szNew.height );
    }
private:
    CalcUtils();
    CalcUtils(CalcUtils const &);
    CalcUtils(CalcUtils const &&);
    CalcUtils &operator=(CalcUtils const &);        
    ~CalcUtils();
};

#endif