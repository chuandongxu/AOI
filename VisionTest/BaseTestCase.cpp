#include "BaseTestCase.h"
#include <iostream>

#include "../lib/VisionLibrary/include/VisionAPI.h"
using namespace AOI::Vision;

QBaseTestCase::QBaseTestCase(QObject *parent)
    : QObject(parent)
{
}

QBaseTestCase::~QBaseTestCase()
{
}

QCaliper::QCaliper(QObject *parent)
    :QBaseTestCase(parent)
{

}

QCaliper::~QCaliper()
{

}

void QCaliper::setup()
{

}

bool QCaliper::run()
{
   /* PR_CALIPER_CMD stCmd;
    PR_CALIPER_RPY stRpy;

    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl << "CALIPER REGRESSION TEST #1 STARTING";
    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl;

    stCmd.matInputImg = cv::imread("./data/F1-5-1_Threshold.png");
    cv::Rect rectROI = cv::Rect(1216, 802, 142, 332);
    stCmd.rectRotatedROI.center = cv::Point(rectROI.x + rectROI.width / 2, rectROI.y + rectROI.height / 2);
    stCmd.rectRotatedROI.size = rectROI.size();
    stCmd.enDetectDir = PR_CALIPER_DIR::MAX_TO_MIN;
    stCmd.bCheckLinerity = true;
    stCmd.fPointMaxOffset = 5;
    stCmd.fMinLinerity = 60.;
    stCmd.bCheckAngle = true;
    stCmd.fExpectedAngle = 90;
    stCmd.fAngleDiffTolerance = 5;

    PR_Caliper(&stCmd, &stRpy);
    PrintRpy(stRpy);

    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl << "CALIPER REGRESSION TEST #2 STARTING";
    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl;

    stCmd.matInputImg = cv::imread("./data/F1-5-1_Threshold.png");
    rectROI = cv::Rect(928, 1276, 270, 120);
    stCmd.rectRotatedROI.center = cv::Point(rectROI.x + rectROI.width / 2, rectROI.y + rectROI.height / 2);
    stCmd.rectRotatedROI.size = rectROI.size();
    stCmd.enDetectDir = PR_CALIPER_DIR::MIN_TO_MAX;
    stCmd.fExpectedAngle = 0;

    PR_Caliper(&stCmd, &stRpy);
    PrintRpy(stRpy);

    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl << "CALIPER REGRESSION TEST #3 STARTING";
    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl;

    stCmd.matInputImg = cv::imread("./data/F1-5-1_Threshold.png");
    rectROI = cv::Rect(1496, 1576, 228, 88);
    stCmd.rectRotatedROI.center = cv::Point(rectROI.x + rectROI.width / 2, rectROI.y + rectROI.height / 2);
    stCmd.rectRotatedROI.size = rectROI.size();
    stCmd.enDetectDir = PR_CALIPER_DIR::MAX_TO_MIN;
    stCmd.fExpectedAngle = 0;

    PR_Caliper(&stCmd, &stRpy);
    PrintRpy(stRpy);

    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl << "CALIPER REGRESSION TEST #4 STARTING";
    std::cout << std::endl << "---------------------------------------------";
    std::cout << std::endl;

    stCmd.matInputImg = cv::imread("./data/F1-5-1_Threshold.png");
    rectROI = cv::Rect(1591, 970, 51, 90);
    stCmd.rectRotatedROI.center = cv::Point(rectROI.x + rectROI.width / 2, rectROI.y + rectROI.height / 2);
    stCmd.rectRotatedROI.size = rectROI.size();
    stCmd.enDetectDir = PR_CALIPER_DIR::MAX_TO_MIN;
    stCmd.bCheckLinerity = true;
    stCmd.fPointMaxOffset = 5;
    stCmd.fMinLinerity = 60.;
    stCmd.bCheckAngle = true;
    stCmd.fExpectedAngle = 90;
    stCmd.fAngleDiffTolerance = 5;

    PR_Caliper(&stCmd, &stRpy);
    PrintRpy(stRpy);

    TestCaliperSectionAvgGuassianDiff();
    TestCaliperRoatedROI();*/

    return true;
}

bool QCaliper::result()
{
    return true;
}
