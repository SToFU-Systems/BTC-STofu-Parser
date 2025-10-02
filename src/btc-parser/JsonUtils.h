#pragma once

// 3rd-party libraries
#include <nlohmann/json.hpp>

// Project headers
#include "Common.h"
#include "DataTypes.h"


//================================================================================
// Function: blockToJson
// Description: Produces a complete JSON representation of a BTC Block. Initializes
//              the destination object, writes the current block hash (hex),
//              serializes the header via blockHeaderToJson function, and 
//              serializes all transactions via allTransactionsToJson function. 
//              Outputs the result into the provided json object.
//================================================================================
void blockToJson(IN const Block& block, OUT nlohmann::ordered_json& json);
