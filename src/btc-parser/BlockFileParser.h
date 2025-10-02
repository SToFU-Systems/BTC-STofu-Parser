#pragma once

// Standard Library headers
#include <expected>
#include <vector>

// Project headers
#include "AppErrorCode.hpp"
#include "BitcoinReader.h"
#include "Common.h"
#include "DataTypes.h"

class BlockFileParser
{
public:
    //================================================================================
    // Method: BlockFileParser
    // Description: Constructor. Initializes a BlockFileParser with a non-owning
    //              reference to the given BitcoinReader, which supplies access to
    //              the raw block file data.
    //================================================================================
    explicit BlockFileParser(IN BitcoinReader& reader);


    //================================================================================
    // Method: parse
    // Description: Iterates over the entire block file until EOF, invoking
    //              SingleBlockParser for each block encountered. Aggregates all
    //              successfully parsed blocks into a vector. Returns the vector on
    //              success, or an AppErrorCode if parsing fails at any point.
    //================================================================================
    [[nodiscard]] std::expected<std::vector<Block>, AppErrorCode> parse();


private:
    BitcoinReader& m_reader;
};
