#pragma once

// Standard Library headers
#include <string>

// Project headers
#include "Common.h"

//================================================================================
// Class: IDatabase
// Description: Generic interface for databases. Serves as a common base for 
//              specialized database interfaces, ensuring consistent interaction 
//              across different database backends.
//================================================================================
class IDatabase
{
public:
    virtual ~IDatabase() = default;


    //================================================================================
    // Method: connect
    // Description: Establishes a connection to the database using the provided
    //              connection URL. The format of URL is implementation-specific.
    //================================================================================
    virtual bool connect(IN const std::string& connectionUrl) = 0;


    //================================================================================
    // Method: disconnect
    // Description: Closes the active connection to the database.
    //================================================================================
    virtual void disconnect() = 0;


    //================================================================================
    // Method: isConnected
    // Description: Checks whether a valid and active connection to the database
    //              currently exists.
    //================================================================================
    virtual bool isConnected() = 0;
};
