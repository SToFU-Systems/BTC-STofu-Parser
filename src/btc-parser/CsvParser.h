#pragma once

// Standard Library headers
#include <string>
#include <map>

//================================================================================
// Class: CurlGlobal
// Description: Singleton responsible for initializing and cleaning up
//              global cURL resources. Ensures curl_global_init() and
//              curl_global_cleanup() are called exactly once per program.
//================================================================================
class CurlGlobal
{
public:
	//============================================================================
	// Method: instance
	// Description: Returns the singleton instance of CurlGlobal.
	//              Initializes global cURL resources on first call.
	//============================================================================
	static CurlGlobal& instance()
	{
		static CurlGlobal curlGlobal;
		return curlGlobal;
	}

private:

	//============================================================================
	// Constructor: CurlGlobal
	// Description: Performs global initialization of cURL.
	//============================================================================
	CurlGlobal()
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}


	//============================================================================
	// Destructor: ~CurlGlobal
	// Description: Cleans up global cURL resources automatically
	//              when the program terminates.
	//============================================================================
	~CurlGlobal()
	{
		curl_global_cleanup();
	}

	//============================================================================
	// Deleted special members (no copy, no move)
	//============================================================================
	CurlGlobal(const CurlGlobal&) = delete;
	CurlGlobal& operator=(const CurlGlobal&) = delete;
};


//================================================================================
// Method: DownloadKaggleCsv
// Description: Downloads the Kaggle BTC dataset as a CSV file using cURL.
//================================================================================
bool DownloadKaggleCsv();


//================================================================================
// Method: LoadCsvToMap
// Description: Parses a UTF-8 CSV file and loads it into a std::map<std::string, double>.
//================================================================================
std::map<std::string, double> LoadCsvToMap(const std::string& path);


//================================================================================
// Method: LoadCsvToMap
// Description: Parses a Unicode (UTF-16/UTF-8) CSV file and loads it into a std::map<std::wstring, double>.
//================================================================================
std::map<std::string, double> LoadCsvToMap(const std::string& path);