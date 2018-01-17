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

void TestParseCad()
{
    std::string strCadFile("./data/MyTestCAD.cad");
    VectorOfCadData vecCadData;
    std::string strErrorMsg;

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        std::vector<CAD_DATA_COLUMNS> vecColumns;
        for ( int i = CAD_DATA_COLUMNS::BEGIN; i < CAD_DATA_COLUMNS::END; ++ i )
            vecColumns.push_back ( static_cast<CAD_DATA_COLUMNS> ( i ) );
        int nResult = DataUtils::parseCAD ( strCadFile, vecColumns, vecCadData, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        }else {
            std::cout << "Success to parse CAD, device count " << vecCadData.size() << std::endl;
        }
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD case 2";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        std::vector<CAD_DATA_COLUMNS> vecColumns;
        vecColumns.push_back ( CAD_DATA_COLUMNS::NAME );
        vecColumns.push_back ( CAD_DATA_COLUMNS::X );
        vecColumns.push_back ( CAD_DATA_COLUMNS::Y );
        vecColumns.push_back ( CAD_DATA_COLUMNS::ANGLE );
        vecColumns.push_back ( CAD_DATA_COLUMNS::TOP_BOTTOM );

        vecColumns.push_back ( CAD_DATA_COLUMNS::WIDTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::LENGTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PLACEMENT );
        vecColumns.push_back ( CAD_DATA_COLUMNS::BOARD_NO );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PIN_COUNT );

        int nResult = DataUtils::parseCAD ( strCadFile, vecColumns, vecCadData, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        }else {
            std::cout << "Success to parse CAD, device count " << vecCadData.size() << std::endl;
        }
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD case 3";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        std::vector<CAD_DATA_COLUMNS> vecColumns;        
        vecColumns.push_back ( CAD_DATA_COLUMNS::X );
        vecColumns.push_back ( CAD_DATA_COLUMNS::NAME );    //change sequence with X to test error handle
        vecColumns.push_back ( CAD_DATA_COLUMNS::Y );
        vecColumns.push_back ( CAD_DATA_COLUMNS::ANGLE );
        vecColumns.push_back ( CAD_DATA_COLUMNS::TOP_BOTTOM );

        vecColumns.push_back ( CAD_DATA_COLUMNS::WIDTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::LENGTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PLACEMENT );
        vecColumns.push_back ( CAD_DATA_COLUMNS::BOARD_NO );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PIN_COUNT );

        int nResult = DataUtils::parseCAD ( strCadFile, vecColumns, vecCadData, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        }else {
            std::cout << "Success to parse CAD, device count " << vecCadData.size() << std::endl;
        }
    }
}

void TestParseCAD_1()
{
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD and readPackageSize.";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    std::vector<CAD_DATA_COLUMNS> vecColumns;
    vecColumns.push_back(CAD_DATA_COLUMNS::NAME);    //change sequence with X to test error handle
    vecColumns.push_back(CAD_DATA_COLUMNS::X);    
    vecColumns.push_back(CAD_DATA_COLUMNS::Y);
    vecColumns.push_back(CAD_DATA_COLUMNS::ANGLE);
    vecColumns.push_back(CAD_DATA_COLUMNS::TOP_BOTTOM);
    vecColumns.push_back(CAD_DATA_COLUMNS::TYPE);
    vecColumns.push_back(CAD_DATA_COLUMNS::GROUP);
    vecColumns.push_back(CAD_DATA_COLUMNS::PLACEMENT);
    vecColumns.push_back(CAD_DATA_COLUMNS::BOARD_NO);

    VectorOfCadData vecCadData;
    std::string strErrorMsg;
    int nResult = DataUtils::parseCAD ( "./data/NN_DemoBoard.aoi.txt", vecColumns, vecCadData, strErrorMsg );
    if ( nResult != 0 ) {
        std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        return;
    }
    std::cout << "Success to parse CAD" << std::endl;

    bool bCadWithWidthLength = false;
    for ( const auto &column : vecColumns ) {
        if ( column == CAD_DATA_COLUMNS::WIDTH ) {
            bCadWithWidthLength = true;
            break;
        }
    }

    MapGroupPackageSize mapGroupPackageSize;
    if ( ! bCadWithWidthLength ) {
        nResult = DataUtils::readPackageSize ( "./data/PackageSize.csv", mapGroupPackageSize, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to read package size, error " << strErrorMsg << std::endl;
            return;
        }
    }

    char chArray[100];
    for ( auto &cadData : vecCadData ) {
        if ( ! bCadWithWidthLength ) {
            std::string strOutputPackage, strOutputType;
            PackageSize packageSize;
            if ( DataUtils::decodePackageAndType ( cadData.type, cadData.group, strOutputPackage, strOutputType ) == 0 ) {
                if ( mapGroupPackageSize.find(strOutputPackage) != mapGroupPackageSize.end() ) {
                    auto mapTypePackageSize = mapGroupPackageSize[strOutputPackage];
                    if ( mapTypePackageSize.find(strOutputType) != mapTypePackageSize.end() ) {
                        packageSize = mapTypePackageSize[strOutputType];
                        _snprintf_s ( chArray, sizeof ( chArray ), "(%.2f, %.2f)", packageSize.Width, packageSize.Length );
                        std::cout << " Device \"" << cadData.name << "\" type \"" << cadData.type << "\" " << " package size " << chArray << std::endl;
                    }else {
                        std::cout << "Cannot find device type \"" << cadData.type << "\" size in package" << std::endl;
                    }
                }else {
                    std::cout << "Cannot find package type \"" << strOutputPackage << "\" size in package" << std::endl;
                }
            }else {
                std::cout << "Cannot decode device type \"" << cadData.type << "\"" << std::endl;
            }
        }        
    }
}

void TestDataUtils()
{
    TestIsNumber();
    TestParseCad();
    TestParseCAD_1();
}