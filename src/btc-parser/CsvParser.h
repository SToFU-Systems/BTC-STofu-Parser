#pragma once

// Standard Library headers
#include <string>
#include <map>
#include <AppErrorCode.hpp>


//================================================================================
// Method: DownloadKaggleCsv
// Description: Downloads the Kaggle BTC dataset as a CSV file using cURL.
//================================================================================
bool DownloadKaggleCsv();


//================================================================================
// Method: LoadCsvToMap
// Description: Parses a UTF-8 CSV file and loads it into a std::map<std::string, double>.
//================================================================================
std::expected<std::map<std::string, double>, AppErrorCode> LoadCsvToMap();


//================================================================================
// Method: LoadCsvToMap
// Description: Parses a Unicode (UTF-16/UTF-8) CSV file and loads it into a std::map<std::wstring, double>.
//================================================================================
std::expected<std::map<std::wstring, double>, AppErrorCode> LoadCsvToMapW();
