#pragma once

// Standard Library headers
#include <string>
#include <map>


//================================================================================
// Function: DownloadKaggleCsv
// Description: Downloads the Bitcoin historical dataset (in CSV format) from Kaggle
//              using a built-in HTTP method (e.g., cURL or WinHTTP). The file is
//              stored locally near the executable for later parsing.
//================================================================================
bool DownloadKaggleCsv();


//================================================================================
// Function: LoadCsvToMap
// Description: Loads a CSV file into a std::map container where each entry maps a
//              date string (key) to the corresponding Bitcoin price (value). Used
//              for efficient lookups and data processing in the BTC parser.
//================================================================================
std::map<std::string, double> LoadCsvToMap(const std::string& path);
