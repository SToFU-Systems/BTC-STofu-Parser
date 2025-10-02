// Precompiled headers
#include "pch.h"

// Project headers
#include "BlockHeaderParser.h"
#include "Logger.hpp"
#include "SingleBlockParser.h"
#include "TransactionParser.h"
#include "Utils.h"


//================================================================================
// Method: SingleBlockParser
// Description: Constructor. Initializes a SingleBlockParser with a non-owning
//              reference to the given BitcoinReader and prepares internal state
//              for block parsing.
//================================================================================
SingleBlockParser::SingleBlockParser(IN BitcoinReader& reader)
    : m_reader(reader)
{
}

//================================================================================
// Method: parse
// Description: Parses a single Bitcoin block from the underlying reader. Steps:
//              1. Reads and validates the magic number.
//              2. Reads and validates the declared block size.
//              3. Uses BlockHeaderParser to compute the block hash and parse
//                 the block header.
//              4. Uses TransactionParser to read all transactions in the block.
//              5. Verifies that the reader consumed exactly the declared number
//                 of bytes for the block.
//              Returns the fully populated Block on success or an error code
//              wrapped in std::expected if any step fails.
//================================================================================
std::expected<Block, AppErrorCode> SingleBlockParser::parse()
{
    Block out;

    // 0. Get current position
    const size_t kStartReadPos = m_reader.getReadPosition();

    // 1. Check Magic
    const auto kMagicParseResult = parseMagicNumber();
    RETURN_IF_UNEXPECTED(kMagicParseResult);

    uint32_t magicNumber = kMagicParseResult.value();

    // 2. Parse Blocksize
    const auto kBlockSizeParseResult = parseBlockSize();
    RETURN_IF_UNEXPECTED(kBlockSizeParseResult);

    uint32_t blockSize = kBlockSizeParseResult.value();

    // 3. Parse BlockHeader
    BlockHeaderParser headerParser(m_reader);

    // 3.1. Current Block Hash
    auto currentBlockHashResult = headerParser.getBlockHash();
    RETURN_IF_UNEXPECTED(currentBlockHashResult);

    out.m_currentBlockHash = std::move(currentBlockHashResult.value());

    // 3.2. Block Header
    auto kBlockHeaderParseResult = headerParser.parse();
    RETURN_IF_UNEXPECTED(kBlockHeaderParseResult);

    out.m_blockHeader = std::move(kBlockHeaderParseResult).value();

    // 4. Parse Transactions
    TransactionParser transactionParser(m_reader);

    auto kTransactionParseResult = transactionParser.parseAllTransactions();
    RETURN_IF_UNEXPECTED(kTransactionParseResult);

    out.m_transactions = std::move(kTransactionParseResult).value();

    // 5. Check read position compared to blocksize
    const size_t kActualBlockSize = kStartReadPos + blockSize + sizeof(magicNumber) + sizeof(blockSize);
    const size_t kReadPosition = m_reader.getReadPosition();
    if (kReadPosition != kActualBlockSize)
    {
        LOG_ERROR("Block size differs from read position. Block size = {}, read position = {}.", kActualBlockSize, kReadPosition);
        return std::unexpected(AppErrorCode::BlockParsingFailure);
    }

    return out;
}

//================================================================================
// Method: parseMagicNumber
// Description: Reads a 32-bit magic number from the file and checks that it
//              matches the expected mainnet constant. Returns the magic number
//              if valid or AppErrorCode if invalid.
//================================================================================
std::expected<uint32_t, AppErrorCode> SingleBlockParser::parseMagicNumber()
{
    /*
    * Bitcoin networks:
        * Mainnet:  0xD9B4BEF9
        * Testnet:  0x0709110B
        * Regtest:  0xDAB5BFFA
        * Signet:   0x40CF030A
    * Bitcoin Core uses Mainnet
    */
    constexpr uint32_t kMainnetMagicNumber = 0xD9B4BEF9;

    LOG_VERBOSE("Parsing magic number...");
    auto kReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kReadResult);

    const uint32_t kParsedMagicNumber = kReadResult.value();

    if (kParsedMagicNumber != kMainnetMagicNumber)
    {
        LOG_ERROR("Incorrect magic number. Expected: {} (mainnet), instead received: {}", kMainnetMagicNumber, kParsedMagicNumber);
        return std::unexpected(AppErrorCode::BlockWrongMagic);
    }

    return kParsedMagicNumber;
}

//================================================================================
// Method: ParseBlockSize
// Description: Reads a 32-bit block size from the file and ensures that the value 
//              is non-zero and within reasonable bounds. Returns the size on 
//              success or an AppErrorCode otherwise.
//================================================================================
std::expected<uint32_t, AppErrorCode> SingleBlockParser::parseBlockSize()
{
    constexpr size_t kMaxExpectedSizeMb = 100;
    constexpr size_t kMaxExpectedSizeBytes = kMaxExpectedSizeMb * 1024 * 1024;

    LOG_VERBOSE("Parsing block size...");
    const auto kReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kReadResult);

    uint32_t out = kReadResult.value();

    if (out == 0)
    {
        LOG_ERROR("Parsed block size is 0.");
        return std::unexpected(AppErrorCode::EmptyBlockSize);
    }

    if (out > kMaxExpectedSizeBytes)
    {
        LOG_ERROR("Parsed block size is bigger than expected.");
        return std::unexpected(AppErrorCode::TooLargeBlockSize);
    }

    return out;
}
