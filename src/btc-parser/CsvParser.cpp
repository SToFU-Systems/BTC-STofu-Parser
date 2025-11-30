// Precompiled headers
#include "pch.h"

// Project headers
#include "Logger.hpp"
#include "CsvParser.h"

//================================================================================
// Constants
//================================================================================
namespace
{
	constexpr const char* kCacert = "../../cacert.pem";
	constexpr const char* kOutPath = "./btc_1h_data_2018_to_2025.csv";
	constexpr const char* kKaggleUrl = "https://www.kaggle.com/api/v1/datasets/download/"
		"novandraanugrah/bitcoin-historical-datasets-2018-2024"
		"?fileName=btc_1h_data_2018_to_2025.csv";
}

//================================================================================
// Method: DownloadKaggleCsv
// Description: Downloads the Kaggle BTC dataset as a CSV file using cURL.
//================================================================================
bool DownloadKaggleCsv()
{

	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl{ curl_easy_init(), &curl_easy_cleanup };
	if (!curl)
	{
		LOG_ERROR("Failed to initialize CURL (curl_easy_init returned nullptr)");
		return false;
	}


	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, kOutPath, "wb");
	if (err)
	{
		LOG_ERROR("Failed to open output file = \"{}\", errno = {}", kOutPath, err);
		return false;
	}

	curl_easy_setopt(curl.get(), CURLOPT_URL, kKaggleUrl);
	curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);

	curl_easy_setopt(curl.get(), CURLOPT_CAINFO, kCacert);
	curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);

	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, fp);

	CURLcode res = curl_easy_perform(curl.get());
	long http = 0;
	curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http);

	constexpr long kHttpOk = 200;

	if (res != CURLE_OK || http != kHttpOk)
	{
		fclose(fp);
		std::remove(kOutPath);
		LOG_ERROR("curl error: {}, HTTP: {}\n", curl_easy_strerror(res), http);
		return false;
	}

	fclose(fp);
	return true;
}

//================================================================================
// Method: LoadCsvToMap
// Description: Parses a UTF-8 CSV file and loads it into a std::map<std::string, double>.
//================================================================================
std::expected<std::map<std::string, double>, AppErrorCode> LoadCsvToMap(const std::string& path)

{
	// Opening a CSV file
	std::ifstream fin(path);
	if (!fin.is_open())
	{
		LOG_ERROR("Cannot open CSV file: {}\n", path);
		return std::unexpected(AppErrorCode::FileNotFound);
	}

	std::map<std::string, double> result;

	std::string line;
	std::getline(fin, line); // Skipping the header

	// Reading CSV lines and filling a map
	while (std::getline(fin, line))
	{
		if (line.empty())
			continue;

		std::istringstream ss(line);
		std::string datetime, open;

		std::getline(ss, datetime, ',');
		std::getline(ss, open, ',');

		try
		{
			double openPrice = std::stod(open);
			result[datetime] = openPrice;
		}
		catch (const std::invalid_argument&)
		{
			LOG_ERROR("Invalid number format in line: {}\n", line);
		}
		catch (const std::out_of_range&)
		{
			LOG_ERROR("Number out of range in line: {}\n", line);
		}
	}

	return result;
}

//================================================================================
// Method: LoadCsvToMap
// Description: Parses a Unicode (UTF-16/UTF-8) CSV file and loads it into a std::map<std::wstring, double>.
//================================================================================
std::expected<std::map<std::wstring, double>, AppErrorCode> LoadCsvToMap(const std::wstring& path)
{
	// Opening a CSV file
	std::wifstream fin(path);
	if (!fin.is_open())
	{
		LOG_ERROR("Cannot open CSV file: {}\n", path);
		return std::unexpected(AppErrorCode::FileNotFound);
	}

	std::map<std::wstring, double> result;

	std::wstring line;
	std::getline(fin, line); // Skipping the header

	// Reading CSV lines and filling a map
	while (std::getline(fin, line))
	{
		if (line.empty())
			continue;

		std::wistringstream ss(line);
		std::wstring datetime, open;

		std::getline(ss, datetime, L',');
		std::getline(ss, open, L',');

		try
		{
			double openPrice = std::stod(open);
			result[datetime] = openPrice;
		}
		catch (const std::invalid_argument&)
		{
			LOG_ERROR("Invalid number format in line: {}\n", line);
		}
		catch (const std::out_of_range&)
		{
			LOG_ERROR("Number out of range in line: {}\n", line);
		}
	}

	return result;
}
