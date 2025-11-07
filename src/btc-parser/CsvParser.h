#pragma once
#include <string>
#include <map>


const std::string CACERT = "../../cacert.pem";
const std::string kKaggleBaseUrl = "https://www.kaggle.com/api/v1/datasets/download/";
const std::string dataset = "novandraanugrah/bitcoin-historical-datasets-2018-2024";// это уникальный идентификатор набора данных (dataset) на Kaggle API.
const std::string fileName = "btc_1h_data_2018_to_2025.csv";
const std::string outPath = "./btc_1h_data_2018_to_2025.csv";

bool DownloadKaggleCsv();
std::map<std::string, double> LoadCsvToMap(const std::string& path);
