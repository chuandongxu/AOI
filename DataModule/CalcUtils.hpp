#ifndef _CALC_UTILS_H_
#define _CALC_UTILS_H_

class CalcUtils
{
public:
    template<typename T>
    static inline cv::Rect_<T> scaleRect(const cv::Rect_<T> &rectInput, float fScale)
    {
        cv::Point2f ptCtr(ToFloat(rectInput.x + rectInput.width / 2.f), ToFloat(rectInput.y + rectInput.height / 2.f));
        float scaleWidth  = rectInput.width  * fScale;
        float scaleHeight = rectInput.height * fScale;
        return cv::Rect_<T>(static_cast<T>(ptCtr.x - scaleWidth / 2.f), static_cast<T>(ptCtr.y - scaleHeight / 2.f),
            static_cast<T>(scaleWidth), static_cast<T>(scaleHeight));
    }

    template<typename T>
    static inline cv::Rect_<T> resizeRect(const cv::Rect_<T> &rectInput, cv::Size_<T> szNew)
    {
        return cv::Rect_<T> (rectInput.x + rectInput.width / 2.f - szNew.width / 2, rectInput.y + rectInput.height / 2.f  - szNew.height / 2,
            szNew.width, szNew.height );
    }

    template<typename T>
    static void inline adjustRectROI(cv::Rect_<T> &rect, const cv::Mat &matInput) {
        if (rect.x < 0) rect.x = 0;
        if (rect.y < 0) rect.y = 0;
        if ((rect.x + rect.width)  > matInput.cols) rect.width = matInput.cols - rect.x;
        if ((rect.y + rect.height) > matInput.rows) rect.height = matInput.rows - rect.y;
    }

    template<typename T>
    static inline bool isRectInRect(const cv::Rect_<T> &rectIn, const cv::Rect_<T> &rectOut) {
    if (rectOut.contains(rectIn.tl()) && rectOut.contains(rectIn.br()))
        return true;
    return false;
}

private:
    CalcUtils();
    CalcUtils(CalcUtils const &);
    CalcUtils(CalcUtils const &&);
    CalcUtils &operator=(CalcUtils const &);
    ~CalcUtils();
};

#endif