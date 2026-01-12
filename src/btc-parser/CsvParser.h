#pragma once

// Standard Library headers
#include <string>
#include <map>

// Project headers
#include <Common.h>
#include <AppErrorCode.hpp>

//================================================================================
// Class: CsvParser
// Description: Provides functionality for downloading a Kaggle BTC CSV dataset
//              and parsing it into in-memory structures. The class stores paths
//              for the output CSV file and CA certificate bundle, supplied by
//              the caller at construction time.
//================================================================================
class CsvParser
{
public:

    //================================================================================
    // Method: CsvParser (constructor)
    // Description: Initializes internal paths used for CSV download and parsing.
    //================================================================================
    CsvParser(
        IN std::string_view outPath,
        IN std::string_view cacertPath
    );

    //================================================================================
    // Method: DownloadKaggleCsv
    // Description: Downloads the Kaggle BTC dataset as a CSV file using cURL.
    //================================================================================
    [[nodiscard]] bool DownloadKaggleCsv();


    //================================================================================
    // Method: LoadCsvToMap
    // Description: Parses a UTF-8 CSV file and loads it into a std::map<std::string, double>.
    //================================================================================
    [[nodiscard]] std::expected<std::map<std::string, double>, AppErrorCode> LoadCsvToMap();

private:

    //================================================================================
    // Method: GetOutputFilePath
    // Description: Returns internal output CSV path as a null-terminated C-string.
    //================================================================================
    [[nodiscard]] const char* GetOutPath() const noexcept;



    //================================================================================
    // Method: LoadCsvToMap
    // Description: Parses a Unicode (UTF-16/UTF-8) CSV file and loads it into a std::map<std::wstring, double>.
    //================================================================================
    [[nodiscard]] const char* GetCacert() const noexcept;

private:
    std::string m_OutPath;
    std::string m_Cacert;
};
