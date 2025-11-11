// Precompiled headers
#include "pch.h"

// Project headers
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
    constexpr const char* url = kKaggleUrl;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        curl_global_cleanup();
        return false;
    }

    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, kOutPath, "wb");
    if (err)
    {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

    curl_easy_setopt(curl, CURLOPT_CAINFO, kCacert);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);
    long http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);

    fclose(fp);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (res != CURLE_OK || http != 200)
    {
        std::remove(kOutPath);
        std::cerr << "curl error: " << curl_easy_strerror(res)
                  << ", HTTP: " << http << "\n";
        return false;
    }

    return true;
}

//================================================================================
// Method: LoadCsvToMap
// Description: Parses a UTF-8 CSV file and loads it into a std::map<std::string, double>.
//================================================================================
std::map<std::string, double> LoadCsvToMap(const std::string& path)
{
    std::map<std::string, double> result;

    // Opening a CSV file
    std::ifstream fin(path);
    if (!fin.is_open())
    {
        std::cerr << "Cannot open CSV file: " << path << "\n";
        return result;
    }

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
            std::cerr << "Invalid number format in line: " << line << "\n";
        }
        catch (const std::out_of_range&)
        {
            std::cerr << "Number out of range in line: " << line << "\n";
        }
    }

    return result;
}


//================================================================================
// Method: LoadCsvToMap
// Description: Parses a Unicode (UTF-16/UTF-8) CSV file and loads it into a std::map<std::wstring, double>.
//================================================================================
std::map<std::wstring, double> LoadCsvToMap(const std::wstring& path)
{
    std::map<std::wstring, double> result;

    // Opening a CSV file
    std::wifstream fin(path);
    if (!fin.is_open())
    {
        std::wcerr << "Cannot open CSV file: " << path << "\n";
        return result;
    }

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
            std::wcerr << L"Invalid number format in line: " << line << L"\n";
        }
        catch (const std::out_of_range&)
        {
            std::wcerr << L"Number out of range in line: " << line << L"\n";
        }
    }

    return result;
}
