// Precompiled headers
#include "pch.h"

// Project headers
#include "Logger.hpp"
#include "CsvParser.h"

//================================================================================
// Method: CsvParser
// Description: Initializes internal paths used for CSV download and parsing.
//================================================================================
CsvParser::CsvParser(
    IN std::string_view outPath,
    IN std::string_view cacertPath
)
    : m_OutPath(std::move(outPath))
    , m_Cacert(std::move(cacertPath))
{
}
 
//================================================================================
// Method: DownloadKaggleCsv
// Description: Downloads the Kaggle BTC dataset as a CSV file using cURL.
//================================================================================
bool CsvParser::DownloadKaggleCsv()
{

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl{ curl_easy_init(), &curl_easy_cleanup };
    if (!curl)
    {
        LOG_ERROR("Failed to initialize CURL (curl_easy_init returned nullptr)");
        return false;
    }

    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, GetOutPath(), "wb");
    if (err)
    {
        LOG_ERROR("Failed to open output file = \"{}\", errno = {}", GetOutPath(), err);
        return false;
    }

    constexpr const char* kKaggleUrl = "https://www.kaggle.com/api/v1/datasets/download/"
        "novandraanugrah/bitcoin-historical-datasets-2018-2024"
        "?fileName=btc_1h_data_2018_to_2025.csv";

    curl_easy_setopt(curl.get(), CURLOPT_URL, kKaggleUrl);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl.get(), CURLOPT_CAINFO, GetCacert());
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
        std::remove(GetOutPath());
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
std::expected<std::map<std::string, double>, AppErrorCode> CsvParser::LoadCsvToMap()
{
    // Opening a CSV file
    std::ifstream fin(GetOutPath());
    if (!fin.is_open())
    {
        LOG_ERROR("Cannot open CSV file: {}\n", GetOutPath());
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
// Method: GetOutputFilePath
// Description: Returns internal output CSV path as a null-terminated C-string.
//================================================================================
const char* CsvParser::GetOutPath() const noexcept
{
    return m_OutPath.c_str();
}

//================================================================================
// Method: GetCacertFilePath
// Description: Returns internal CA certificate bundle path
//================================================================================
const char* CsvParser::GetCacert() const noexcept
{
    return m_Cacert.c_str();
}
