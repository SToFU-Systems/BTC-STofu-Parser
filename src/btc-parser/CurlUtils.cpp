#include "pch.h"
#include "CurlUtils.h"


//============================================================================
	// Constructor: CurlGlobal
	// Description: Performs global initialization of cURL.
	//============================================================================
CurlGlobal::CurlGlobal()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

//============================================================================
// Destructor: ~CurlGlobal
// Description: Cleans up global cURL resources automatically
//              when the program terminates.
//============================================================================
CurlGlobal::~CurlGlobal()
{
	curl_global_cleanup();
}

//============================================================================
// Method: instance
// Description: Returns the singleton instance of CurlGlobal.
//              Initializes global cURL resources on first call.
//============================================================================
CurlGlobal& CurlGlobal::instance()
{
	static CurlGlobal curlGlobal;
	return curlGlobal;
}
