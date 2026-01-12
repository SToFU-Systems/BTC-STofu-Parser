#pragma once

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
	static CurlGlobal& instance();
	
private:

	//============================================================================
	// Constructor: CurlGlobal
	// Description: Performs global initialization of cURL.
	//============================================================================
	CurlGlobal();


	//============================================================================
	// Destructor: ~CurlGlobal
	// Description: Cleans up global cURL resources automatically
	//              when the program terminates.
	//============================================================================
	~CurlGlobal();
	

	//============================================================================
	// Deleted special members (no copy, no move)
	//============================================================================
	CurlGlobal(const CurlGlobal&) = delete;
	CurlGlobal& operator=(const CurlGlobal&) = delete;
};
