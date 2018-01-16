#include "../DataModule/DataUtils.h"
#include <iostream>

void TestIsNumber()
{
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::isNumber function";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    static const std::string strArrayTestCase[]{
        "+",
        "-",
        "0",
        "3",
        "9",
        "a",
        "99a9",
        "9909",
        "",
        "-3.14",
        "+32768",
        "-32768",
    };

    for ( const auto &strTestCase : strArrayTestCase ) {
        bool bIsNumber = DataUtils::isNumber ( strTestCase );
        std::cout << strTestCase << ( bIsNumber ? " is a number" : " is not a number" ) << std::endl; 
    }
}

void TestDataUtils()
{
    TestIsNumber();
}