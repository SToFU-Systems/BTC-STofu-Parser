// Precompiled headers
#include "pch.h"

// Project headers
#include "BlockParser.h"
#include "DataTypes.h"
#include "Utils.h"


//================================================================================
// Method: BlockParser
// Description: Constructor that initializes a BlockParser instance using the
//              given block file path. Creates a FileReader to handle block file
//              input, sets the initial read position and block offset position
//              to zero, and disables XOR obfuscation by default. Prepares the
//              parser for reading block data sequentially.
//================================================================================
BlockParser::BlockParser(IN const std::wstring& blockPath)
    :
    m_fileReader(blockPath),
    m_readPosition(0),
    m_blockOffsetPosition(0),
    m_doXorObfuscation(false)
{
}

//================================================================================
// Method: BlockParser
// Description: Constructor that initializes a BlockParser instance with both a
//              block file path and XOR key file path. Calls the primary 
//              constructor to set up the block reader, then loads the XOR key. 
//              If loaded successfully, enables XOR obfuscation for subsequent 
//              block data parsing.
//================================================================================
BlockParser::BlockParser(
    IN const std::wstring& blockPath, 
    IN const std::wstring& xorPath
)
    : BlockParser(blockPath)
{
    if (xorPath.empty())
        return;

    constexpr size_t kXorKeyBytes = 8;

    FileReader xorFile(xorPath);
    const bool kXorReadResult = xorFile.readBytes(kXorKeyBytes, xorKey);
    if (!kXorReadResult)
        throw std::runtime_error("Couldn't read the xor.dat");

    if(xorKey.size() != kXorKeyBytes)
        throw std::runtime_error("Incorrect size of xor key");

    m_doXorObfuscation = true;
}

//================================================================================
// Method: ParseBlock
// Description: Parses a single Bitcoin block from the file. Steps:
//              1. Reads and validates the block magic number.
//              2. Reads the block size.
//              3.1. Converts whole block header (80 bytes) into hash.
//              3.2. Parses the block header into the output Block.
//              4. Reads the transaction counter.
//              5. Iterates and parses each transaction in the block.
//              6. Validates that the actual bytes read match the block size.
//              Updates the internal block offset position after successful
//              parsing. Returns true if the block is parsed correctly, false if
//              any validation or parsing step fails.
//================================================================================
bool BlockParser::parseBlock(OUT Block& out)
{
    // 1. Check Magic
    uint32_t magicNumber;
    const bool kMagicParseResult = parseMagicNumber(magicNumber);
    if (!kMagicParseResult)
    {
        std::cout << "Incorrect magic number: " << magicNumber << std::endl;
        return false;
    }

    // 2. Parse Blocksize
    uint32_t blockSize;
    const bool kBlockSizeParseResult = parseBlockSize(blockSize);
    if (!kBlockSizeParseResult)
    {
        std::cout << "Failed to get block size\n";
        return false;
    }

    // 3.1. Generate Current Block Hash
    auto currentBlockHashResult = getCurrentBlockHash();
    if (!currentBlockHashResult.has_value())
    {
        std::cout << "Failed to get Current Block Hash.\n";
        return false;
    }
    out.m_currentBlockHash = std::move(currentBlockHashResult.value());

    // 3.2. Parse BlockHeader
    const bool kBlockHeaderParseResult = parseBlockHeader(out.m_blockHeader);
    if (!kBlockHeaderParseResult)
    {
        std::cout << "Failed to parse BlockHeader\n";
        return false;
    }

    // 4. Get Transaction Counter
    uint64_t transactionCounter;
    const bool kTrxCounterParseResult = parseTransactionCounter(transactionCounter);
    if (!kTrxCounterParseResult)
    {
        std::cout << "Failed to parse Transaction Counter\n";
        return false;
    }

    // 5. Parse Transactions
    out.m_transactions.reserve(transactionCounter);
    for (size_t i = 0; i < transactionCounter; ++i)
    {
        Transaction transaction;
        const bool kTrxParseResult = parseTransaction(transaction);
        if (!kTrxParseResult)
        {
            std::cout << "Failed to parse transaction #" << i + 1 << std::endl;
            return false;
        }
        out.m_transactions.push_back(std::move(transaction));
    }

    // 6. Check read position compared to blocksize
    const size_t kActualBlockSize = m_blockOffsetPosition + blockSize + sizeof(magicNumber) + sizeof(blockSize);
    m_readPosition = m_fileReader.getReadPosition();
    if (m_readPosition != kActualBlockSize)
    {
        std::cout << "Block size is different from actual read data.\n";
        std::cout << "Block size: " << kActualBlockSize << " , actual read size: " << m_readPosition << std::endl;
        return false;
    }

    m_blockOffsetPosition = m_readPosition;
    return true;
}

//================================================================================
// Method: ParseAllBlocks
// Description: Parses all blocks sequentially from the opened block file until
//              EOF. For each block, calls ParseBlock and stores the result in 
//              the output vector. Returns true if all blocks are parsed correctly,
//              false if any failed.
//================================================================================
bool BlockParser::parseAllBlocks(OUT std::vector<Block>& out)
{
    constexpr size_t kApproxBlockSizeBytes = 1000;
    const size_t kApproxReserve = m_fileReader.getFileSize() / kApproxBlockSizeBytes;
    out.clear();
    out.reserve(kApproxReserve);

    while (!m_fileReader.isEof())
    {
        Block block;
        const bool kBlockParseResult = parseBlock(block);
        if (!kBlockParseResult)
            return false;

        out.push_back(std::move(block));
    }

    return true;
}

//================================================================================
// Method: ParseMagicNumber
// Description: Reads and validates the block magic number by comparing it to the 
//              expected mainnet magic number constant. Returns true if the read 
//              succeeds and the number matches, false otherwise.
//================================================================================
bool BlockParser::parseMagicNumber(OUT uint32_t& out)
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

    const bool kReadResult = readDataUint32(out);
    if (!kReadResult)
        return false;

    if (out != kMainnetMagicNumber)
        return false;

    return true;
}

//================================================================================
// Method: ParseBlockSize
// Description: Reads the block size from file. Ensures that the
//              size is greater than zero. Returns true on success, false if
//              reading fails or the size is invalid.
//================================================================================
bool BlockParser::parseBlockSize(OUT uint32_t& out)
{
    const bool kReadResult = readDataUint32(out);
    if (!kReadResult)
        return false;

    if (out == 0)
        return false;

    return true;
}

//================================================================================
// Method: ParseBlockHeader
// Description: Parses the block header data into the provided BlockHeader
//              structure. Steps:
//              - Reads the version.
//              - Reads the previous block hash.
//              - Reads the Merkle root hash.
//              - Reads the timestamp.
//              - Reads the difficulty bits.
//              - Reads the nonce.
//              Returns true if all fields are parsed correctly, false otherwise.
//================================================================================
bool BlockParser::parseBlockHeader(OUT BlockHeader& header)
{
    // Version
    const bool kVersionReadResult = readDataInt32(header.m_version);
    if (!kVersionReadResult)
        return false;
    
    // Hash previous block
    const bool kHashPrevReadResult = readDataArray32(header.m_hashPrevBlock);
    if (!kHashPrevReadResult)
        return false;

    // Hash merkle root
    const bool kHashMerkleReadResult = readDataArray32(header.m_hashMerkleRoot);
    if (!kHashMerkleReadResult)
        return false;

    // Time 
    const bool kTimeReadResult = readDataUint32(header.m_time);
    if (!kTimeReadResult)
        return false;

    // Bits
    const bool kBitsReadResult = readDataUint32(header.m_bits);
    if (!kBitsReadResult)
        return false;

    // Nonce
    const bool kNonceReadResult = readDataUint32(header.m_nonce);
    if (!kNonceReadResult)
        return false;

    return true;
}

//================================================================================
// Method: getCurrentBlockHash
// Description: Reads the current block header from the file, computes its hash 
//              (double SHA-256, reversed). Returns the computed hash on success,
//              otherwise std::nullopt.
//================================================================================
std::optional<std::array<uint8_t, 32>> BlockParser::getCurrentBlockHash()
{
    constexpr size_t kBlockHeaderSize = sizeof(BlockHeader);

    // Read 80 bytes (block header)
    const size_t kStartReadPosition = m_fileReader.getReadPosition();
    std::vector<uint8_t> fullBlockHeader;
    const bool kReadResult = readData(kBlockHeaderSize, fullBlockHeader);
    if (!kReadResult)
        return std::nullopt;

    // Generate hash
    std::array<uint8_t, 32> hashResult = getReversedDoubleSHA256(fullBlockHeader);

    // Set read position back
    const bool kSetPosResult = updateReadPosition(kStartReadPosition);
    if (!kSetPosResult)
        return std::nullopt;

    return hashResult;
}

//================================================================================
// Method: ParseTransactionCounter
// Description: Reads a CompactSize-encoded integer representing the number of
//              transactions in the current block. Returns true on success, false
//              otherwise.
//================================================================================
bool BlockParser::parseTransactionCounter(OUT uint64_t& out)
{
    constexpr size_t trxCounterSize = sizeof(out);
    const bool kTrxCounterResult = readDataCompactSize(out);
    if (!kTrxCounterResult)
        return false;

    return true;
}

//================================================================================
// Method: ParseTransaction
// Description: Parses a full Bitcoin transaction into the provided Transaction
//              structure. Steps:
//              - Reads the version.
//              - Reads the optional segwit marker and flag.
//              - Reads the input counter and parses all transaction inputs.
//              - Reads the output counter and parses all transaction outputs.
//              - If segwit is present, parses witness data for each input.
//              - Reads the transaction lock time.  
//              Returns true if all fields are parsed correctly, false otherwise.
//================================================================================
bool BlockParser::parseTransaction(OUT Transaction& transaction)
{
    // needed for transaction hash
    const size_t kStartReadPosition = m_fileReader.getReadPosition();

    // Version
    const bool kVersionReadResult = readDataInt32(transaction.m_version);
    if (!kVersionReadResult)
        return false;

    // Flag
    std::optional<bool> kFlagParseResult = parseTransactionFlag();
    if (!kFlagParseResult.has_value())
        return false;

    transaction.m_segwitFlag = kFlagParseResult.value();

    // In-counter
    uint64_t inCounter;
    const bool kInCounterReadResult = readDataCompactSize(inCounter);
    if (!kInCounterReadResult)
        return false;

    // TxIn
    transaction.m_vIn.reserve(inCounter);
    for (size_t i = 0; i < inCounter; ++i)
    {
        TxIn transactionInput;
        const bool kTxInpuParsetResult = parseTxIn(transactionInput);
        if (!kTxInpuParsetResult)
            return false;

        transaction.m_vIn.push_back(std::move(transactionInput));
    }
    
    // Out-counter
    uint64_t outCounter;
    const bool kOutCounterReadResult = readDataCompactSize(outCounter);
    if (!kOutCounterReadResult)
        return false;

    // TxOut
    transaction.m_vOut.reserve(outCounter);
    for (size_t i = 0; i < outCounter; ++i)
    {
        TxOut transactionOutput;
        const bool kTxInputParseResult = parseTxOut(transactionOutput);
        if (!kTxInputParseResult)
            return false;

        transaction.m_vOut.push_back(std::move(transactionOutput));
    }
    
    // Witness
    if (transaction.m_segwitFlag)
    {
        transaction.m_witnesses.resize(inCounter);
        for (size_t i = 0; i < inCounter; ++i)
        {
            std::vector<uint8_t> witness;
            const bool kWitnessParseResult = parseWitness(witness);
            if (!kWitnessParseResult)
                return false;

            transaction.m_witnesses[i] = witness;
        }
    }
    
    // Lock time
    const bool kLockTimeReadResult = readDataUint32(transaction.m_lockTime);
    if (!kLockTimeReadResult)
        return false;

    // Convert Transaction into hash
    auto currentTransactionHashResult = getCurrentTransactionHash(kStartReadPosition, m_readPosition);
    if (!currentTransactionHashResult.has_value())
        return false;

    transaction.m_currentTransactionHash = currentTransactionHashResult.value();

    return true;
}

//================================================================================
// Method: ParseTransactionFlag
// Description: Parses the optional transaction segwit flag (00 01). If the flag
//              is present, sets output to true. If not, rewinds the read
//              position and sets output to false. Returns true if parsing or 
//              fallback succeeds, false otherwise.
//================================================================================
std::optional<bool> BlockParser::parseTransactionFlag()
{
    // always 00 01 , but optional (may not be there)
    enum WitFlag : uint8_t
    {
        FIRST_BYTE = 0x00,
        SECOND_BYTE = 0x01
    };

    // save pre-read position
    size_t kCurrentPos = m_fileReader.getReadPosition();

    std::vector<uint8_t> flagBuffer;
    const bool kFlagFirstReadResult = readData(1, flagBuffer);
    if (!kFlagFirstReadResult)
        return std::nullopt;

    if (flagBuffer[0] == FIRST_BYTE)
    {
        flagBuffer.clear();
        const bool kFlagSecondReadResult = readData(1, flagBuffer);
        if (!kFlagSecondReadResult)
            return std::nullopt;

        if (flagBuffer[0] == SECOND_BYTE)
            return true;
    }

    // no flag, return to pre-read position
    const bool kSetPosResult = updateReadPosition(kCurrentPos);
    if (!kSetPosResult)
        return std::nullopt;

    return false;
}

//================================================================================
// Method: ParseTxIn
// Description: Parses a single transaction input (TxIn). Steps:
//              - Reads the previous transaction hash.
//              - Reads the output index being referenced.
//              - Reads the script length.
//              - Reads the scriptSig data.
//              - Reads the sequence number.  
//              Returns true if all fields are parsed correctly, false otherwise.
//================================================================================
bool BlockParser::parseTxIn(OUT TxIn& out)
{
    // Prevhash
    const bool kOutHashReadResult = readDataArray32(out.m_hashPreviousTx);
    if (!kOutHashReadResult)
        return false;

    // TxOut index
    const bool kPrevIndexReadResult = readDataUint32(out.m_prevTxOutIndex);
    if (!kPrevIndexReadResult)
        return false;
    
    // Script length
    uint64_t scriptLength;
    const bool kScriptLengthReadResult = readDataCompactSize(scriptLength);
    if (!kScriptLengthReadResult)
        return false;

    // ScriptSig
    // just parsing
    const bool kScriptSigReadResult = readData(scriptLength, out.m_scriptSignature);
    if (!kScriptSigReadResult)
        return false;

    // Sequence number, normally 0xFFFFFFFF
    const bool kSequenceNumberReadResult = readDataUint32(out.m_sequenceNumber);
    if (!kSequenceNumberReadResult)
        return false;

    return true;
}

//================================================================================
// Method: ParseTxOut
// Description: Parses a single transaction output (TxOut). Steps:
//              - Reads the output value in satoshis (uint64).
//              - Reads the script length (CompactSize).
//              - Reads the scriptPubKey data.  
//              Returns true if all fields are parsed correctly, false otherwise.
//================================================================================
bool BlockParser::parseTxOut(OUT TxOut& out)
{
    // Value
    const bool kValueReadResult = readDataUint64(out.m_value);
    if (!kValueReadResult)
        return false;

    // Script length
    uint64_t scriptLength;
    const bool kScriptLenReadResult = readDataCompactSize(scriptLength);
    if (!kScriptLenReadResult)
        return false;

    // ScriptPubKey
    // just parsing
    const bool kScriptPubKeyReadResult = readData(scriptLength, out.m_scripts);
    if (!kScriptPubKeyReadResult)
        return false;

    return true;
}

//================================================================================
// Method: ParseWitness
// Description: Parses witness data associated with a segwit transaction. Reads
//              a CompactSize counter for the number of witness elements, then
//              for each element reads its length (CompactSize) and data bytes.
//              Appends all witness data into the output buffer. Returns true
//              if all witness elements are parsed successfully, false otherwise.
//================================================================================
bool BlockParser::parseWitness(OUT std::vector<uint8_t>& out)
{
    uint64_t witnessCounter;
    const bool kWitCountReadResult = readDataCompactSize(witnessCounter);
    if (!kWitCountReadResult)
        return false;

    out.clear();
    out.reserve(witnessCounter);
    for (size_t i = 0; i < witnessCounter; ++i)
    {
        uint64_t witnessLength;
        const bool kWitLenReadResult = readDataCompactSize(witnessLength);
        if (!kWitLenReadResult)
            return false;

        std::vector<uint8_t> data;
        const bool kReadWitReadResult = readData(witnessLength, data);
        if (!kReadWitReadResult)
            return false;

        out.insert(out.end(), data.begin(), data.end());
    }

    return true;
}

//================================================================================
// Method: getCurrentTransactionHash
// Description: Computes the transaction hash (double SHA-256, reversed) for the
//              transaction defined by start and finish positions. Returns the 
//              computed hash on success, otherwise std::nullopt.
//================================================================================
std::optional<std::array<uint8_t, 32>> BlockParser::getCurrentTransactionHash(
    IN const size_t& startReadPosition,
    IN const size_t& finishReadPosition
)
{
    const size_t kTransactionSize = finishReadPosition - startReadPosition;
    const size_t kSavedReadPosition = finishReadPosition;

    // Get reading position back to the start of transaction
    const bool kSetPositionResult = updateReadPosition(startReadPosition);
    if (!kSetPositionResult)
        return std::nullopt;

    // Read raw full transaction
    std::vector<uint8_t> fullTransaction;
    const bool kReadResult = readData(kTransactionSize, fullTransaction);
    
    // Check current read position
    if (m_readPosition != kSavedReadPosition)
        return std::nullopt;

    // Generate and return hash
    return getReversedDoubleSHA256(fullTransaction);
}

//================================================================================
// Method: XorObfuscation
// Description: Applies XOR-based obfuscation to the provided data buffer using 
//              the stored XOR key. Each byte of the data is XOR'ed with a 
//              corresponding key byte, chosen by using the absolute read position.
//              Returns true on successful obfuscation, false otherwise.
//================================================================================
bool BlockParser::xorObfuscation(IN const size_t readPosition, INOUT std::vector<uint8_t>& data)
{
    if (data.empty())
        return false;

    if (xorKey.empty())
        return false;

    constexpr size_t kXorKeyBytes = 8;
    bool isCorrectSize = xorKey.size() == kXorKeyBytes;
    if (!isCorrectSize)
        return false;

    size_t keySize = xorKey.size();
    for (size_t i = 0; i < data.size(); ++i)
    {
        const size_t offset = i + readPosition;
        const size_t index = offset % keySize;
        data[i] ^= xorKey[index];
    }

    return true;
}

//================================================================================
// Method: ReadData
// Description: Reads a specified number of bytes from the block file into the
//              output vector. Updates the internal read position after a
//              successful read. If XOR obfuscation is enabled, applies it to
//              the data using the original pre-read file position. Returns true
//              on success, false if the file read or obfuscation fails.
//================================================================================
bool BlockParser::readData(IN const size_t bytes, OUT std::vector<uint8_t>& out)
{
    if (bytes == 0)
        return true;

    // needed for xor obfuscation
    const size_t kPreReadPosition = m_fileReader.getReadPosition();

    const bool kReadResult = m_fileReader.readBytes(bytes, out);
    if (!kReadResult)
        return false;

    m_readPosition = m_fileReader.getReadPosition();

    if (!m_doXorObfuscation)
        return true;

    const bool kObfuscationResult = xorObfuscation(kPreReadPosition, out);
    if (!kObfuscationResult)
        return false;

    return true;
}

//================================================================================
// Method: ReadDataUint16
// Description: Reads sizeof(uint16_t) bytes from the block file and converts them 
//              from little-endian encoding into uint16_t value. Uses LeUint8ToUint16 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
bool BlockParser::readDataUint16(OUT uint16_t& out)
{
    constexpr size_t outSize = sizeof(out);

    std::vector<uint8_t> buffer;
    const bool kReadResult = readData(outSize, buffer);
    if (!kReadResult)
        return false;

    const bool kConvertResult = leUint8ToUint16(buffer, out);
    if (!kConvertResult)
        return false;

    return true;
}

//================================================================================
// Method: ReadDataUint32
// Description: Reads sizeof(uint32_t) bytes from the block file and converts them 
//              from little-endian encoding into uint32_t value. Uses LeUint8ToUint32 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
bool BlockParser::readDataUint32(OUT uint32_t& out)
{
    constexpr size_t outSize = sizeof(out);

    std::vector<uint8_t> buffer;
    const bool kReadResult = readData(outSize, buffer);
    if (!kReadResult)
        return false;

    const bool kConvertResult = leUint8ToUint32(buffer, out);
    if (!kConvertResult)
        return false;

    return true;
}

//================================================================================
// Method: ReadDataUint64
// Description: Reads sizeof(uint64_t) bytes from the block file and converts them 
//              from little-endian encoding into uint64_t value. Uses LeUint8ToUint64 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
bool BlockParser::readDataUint64(OUT uint64_t& out)
{
    constexpr size_t outSize = sizeof(out);

    std::vector<uint8_t> buffer;
    const bool kReadResult = readData(outSize, buffer);
    if (!kReadResult)
        return false;

    const bool kConvertResult = leUint8ToUint64(buffer, out);
    if (!kConvertResult)
        return false;

    return true;
}

//================================================================================
// Method: ReadDataInt32
// Description: Reads sizeof(int32_t) bytes from the block file and converts them 
//              from little-endian encoding into int32_t value. Uses LeUint8ToInt32 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
bool BlockParser::readDataInt32(OUT int32_t& out)
{
    constexpr size_t outSize = sizeof(out);

    std::vector<uint8_t> buffer;
    const bool kReadResult = readData(outSize, buffer);
    if (!kReadResult)
        return false;

    const bool kConvertResult = leUint8ToInt32(buffer, out);
    if (!kConvertResult)
        return false;

    return true;
}

//================================================================================
// Method: ReadDataArray32
// Description: Reads 32 bytes from the block file and stores them in
//              std::array<uint8_t, 32>. Performs reverse_copy to account 
//              little-endian encoding. Returns true if 32 bytes are successfully 
//              read and copied, false if insufficient data is available.
//================================================================================
bool BlockParser::readDataArray32(OUT std::array<uint8_t, 32>& out)
{
    constexpr size_t kOutSize = sizeof(out);

    std::vector<uint8_t> buffer;
    const bool kReadResult = readData(kOutSize, buffer);
    if (!kReadResult)
        return false;

    if (buffer.size() < kOutSize)
        return false;

    std::reverse_copy(buffer.begin(), buffer.begin() + kOutSize, out.begin());
    return true;
}

//================================================================================
// Method: ReadDataCompactSize
// Description: Decodes a CompactSize-encoded integer from the given first byte
//              and reads the remaining bytes if needed. 
//              CompactSize is the Bitcoin variable-length integer encoding:
//              - If firstByte < 0xFD: value is the byte itself
//              - If firstByte == 0xFD: next 2 bytes (uint16_t) are read
//              - If firstByte == 0xFE: next 4 bytes (uint32_t) are read
//              - If firstByte == 0xFF: next 8 bytes (uint64_t) are read
//              Returns true on successful decoding, false if reading or 
//              validation fails.
//================================================================================
bool BlockParser::readDataCompactSize(IN const uint8_t firstByte, OUT uint64_t& out)
{
    if (firstByte < 0xFD)
    {
        out = firstByte;
        return true;
    }

    bool kConvertionResult = false;
    switch (firstByte)
    {
    case 0xFD:
        uint16_t tmp16;
        kConvertionResult = readDataUint16(tmp16);
        if (!kConvertionResult)
            return false;

        out = tmp16;
        break;

    case 0xFE:
        uint32_t tmp32;
        kConvertionResult = readDataUint32(tmp32);
        if (!kConvertionResult)
            return false;

        out = tmp32;
        break;

    case 0xFF:
        kConvertionResult = readDataUint64(out);
        if (!kConvertionResult)
            return false;
        break;

    default:
        return false;
    }

    return true;
}

//================================================================================
// Method: ReadDataCompactSize
// Description: Reads the first byte from the file and call overloaded 
//              ReadDataCompactSize to decode the Bitcoin variable-length integer.
//              Returns true on successful decoding, false if reading or 
//              validation fails.
//================================================================================
bool BlockParser::readDataCompactSize(OUT uint64_t& out)
{
    std::vector<uint8_t> buffer;
    const bool kReadBufferResult = readData(1, buffer);
    if (!kReadBufferResult)
        return false;

    const uint8_t kFirstByte = buffer[0];
    const bool kReadCompactResult = readDataCompactSize(kFirstByte, out);
    if (!kReadCompactResult)
        return false;

    return true;
}

//================================================================================
// Method: updateReadPosition
// Description: Updates file read position to the input variable. 
//              Returns true on success, false otherwise.
//================================================================================
bool BlockParser::updateReadPosition(IN const size_t& newPos)
{
    const bool kSetPosResult = m_fileReader.setReadPosition(newPos);
    if (!kSetPosResult)
        return false;

    m_readPosition = newPos;
    return true;
}
