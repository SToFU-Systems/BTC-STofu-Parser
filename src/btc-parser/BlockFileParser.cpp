// Precompiled headers
#include "pch.h"

// Project headers
#include "BlockFileParser.h"
#include "Logger.hpp"
#include "SingleBlockParser.h"


//================================================================================
// Method: BlockFileParser
// Description: Constructor. Initializes a BlockFileParser with a non-owning
//              reference to the given BitcoinReader, which supplies access to
//              the raw block file data.
//================================================================================
BlockFileParser::BlockFileParser(IN BitcoinReader& reader)
    : m_reader(reader)
{
}

//================================================================================
// Method: parse
// Description: Iterates over the entire block file until EOF, invoking
//              SingleBlockParser for each block encountered. Aggregates all
//              successfully parsed blocks into a vector. Returns the vector on
//              success, or an AppErrorCode if parsing fails at any point.
//================================================================================
std::expected<std::vector<Block>, AppErrorCode> BlockFileParser::parse()
{
    constexpr size_t kApproxBlockSizeBytes = 1000;
    const size_t kApproxReserve = m_reader.getFileSize() / kApproxBlockSizeBytes;

    std::vector<Block> out;
    out.reserve(kApproxReserve);

    size_t blockCounter = 1;
    while (!m_reader.isEof())
    {
        LOG_VERBOSE("Parsing block #{}...", blockCounter);
        Block block;
        SingleBlockParser blockParser(m_reader);

        auto kBlockParseResult = blockParser.parse();
        RETURN_IF_UNEXPECTED(kBlockParseResult);

        out.push_back(std::move(kBlockParseResult).value());
        LOG_VERBOSE("Successfully parsed block #{}...", blockCounter);

        ++blockCounter;
    }

    return out;
}
