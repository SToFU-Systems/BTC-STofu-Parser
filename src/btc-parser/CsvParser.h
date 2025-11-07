#pragma once
#include <string>
#include <map>


bool DownloadKaggleCsv();
std::map<std::string, double> LoadCsvToMap(const std::string& path);
