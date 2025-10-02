// Precompiled headers
#include "pch.h"

// Project headers
#include "Logger.hpp"
#include "TransactionParser.h"
#include "TransactionScriptParser.h"
#include "Utils.h"


//================================================================================
// Method: TransactionParser
// Description: Constructor. Stores a non-owning reference to a BitcoinReader
//              instance, which provides raw data for transaction parsing.
//================================================================================
TransactionParser::TransactionParser(IN BitcoinReader& reader)
    : m_reader(reader)
{
}

//================================================================================
// Method: parseAllTransactions
// Description: Reads the transaction counter and sequentially parses that many
//              transactions. Returns a vector of parsed Transaction objects on
//              success, or an AppErrorCode if any parsing step fails.
//================================================================================
std::expected<std::vector<Transaction>, AppErrorCode> TransactionParser::parseAllTransactions()
{
    LOG_VERBOSE("Parsing number of transactions...");
    const auto kTrxCounterParseResult = parseTransactionCounter();
    RETURN_IF_UNEXPECTED(kTrxCounterParseResult);

    uint64_t transactionCounter = kTrxCounterParseResult.value();
    LOG_VERBOSE("Number of transactions: {}", transactionCounter);

    std::vector<Transaction> out;
    out.reserve(transactionCounter);
    for (size_t i = 0; i < transactionCounter; ++i)
    {
        LOG_VERBOSE("Parsing transaction #{}...", i + 1);
        auto kTrxParseResult = parseOneTransaction();
        if (!kTrxParseResult.has_value())
        {
            LOG_ERROR("Failed to parse transaction #{}", i + 1);
            return std::unexpected(kTrxParseResult.error());
        }

        out.push_back(std::move(kTrxParseResult).value());
        LOG_VERBOSE("Successfuly parsed transaction #{}", i + 1);
    }

    LOG_VERBOSE("Successfuly parsed all transactions.");

    return out;
}

//================================================================================
// Method: parseOneTransaction
// Description: Parses a single Bitcoin transaction from the reader. Extracts the
//              version, optional segwit marker/flag, inputs, outputs, optional
//              witness data, and lock time. Also computes and stores the
//              transaction hash. Returns the populated Transaction on success or
//              an AppErrorCode on failure.
//================================================================================
std::expected<Transaction, AppErrorCode> TransactionParser::parseOneTransaction()
{
    Transaction out;

    // needed for transaction hash
    const size_t kStartReadPosition = m_reader.getReadPosition();

    // Version
    LOG_VERBOSE("Parsing version...");
    const auto kVersionReadResult = m_reader.readDataInt32();
    RETURN_IF_UNEXPECTED(kVersionReadResult);

    out.m_version = kVersionReadResult.value();

    // Flag
    LOG_VERBOSE("Parsing witness flag...");
    const auto kFlagParseResult = parseTransactionFlag();
    RETURN_IF_UNEXPECTED(kFlagParseResult);

    out.m_segwitFlag = kFlagParseResult.value();

    // TxIn
    auto txInputParseResult = parseAllTxIn();
    RETURN_IF_UNEXPECTED(txInputParseResult);

    out.m_vIn = std::move(txInputParseResult).value();

    // TxOut
    auto txOutputParseResult = parseAllTxOut();
    RETURN_IF_UNEXPECTED(txOutputParseResult);

    out.m_vOut = std::move(txOutputParseResult).value();

    // Witness
    if (out.m_segwitFlag)
    {
        const size_t kTxInCount = out.m_vIn.size();
        auto witnessParseResult = parseAllWitnesses(kTxInCount);
        RETURN_IF_UNEXPECTED(witnessParseResult);

        out.m_witnesses = std::move(witnessParseResult).value();
    }

    // Lock time
    LOG_VERBOSE("Parsing lock time...");
    const auto kLockTimeReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kLockTimeReadResult);

    out.m_lockTime = kLockTimeReadResult.value();

    // Convert Transaction into hash
    const size_t kCurrentReadPos = m_reader.getReadPosition();
    auto transactionHashResult = getCurrentTransactionHash(kStartReadPosition, kCurrentReadPos);
    RETURN_IF_UNEXPECTED(transactionHashResult);

    out.m_currentTransactionHash = std::move(transactionHashResult).value();

    return out;
}

//================================================================================
// Method: parseTransactionCounter
// Description: Reads a CompactSize-encoded integer representing the number of
//              transactions in the current block. Returns the count on success or
//              AppErrorCode on failure.
//================================================================================
std::expected<uint64_t, AppErrorCode> TransactionParser::parseTransactionCounter()
{
    const auto kTrxCounterResult = m_reader.readDataCompactSize();
    RETURN_IF_UNEXPECTED(kTrxCounterResult);

    return kTrxCounterResult.value();
}

//================================================================================
// Method: parseTransactionFlag
// Description: Attempts to read the optional segwit flag sequence (0x00 0x01). 
//              Returns true if present, false if absent, and rewinds the reader 
//              if not found. Returns AppErrorCode on error.
//================================================================================
std::expected<bool, AppErrorCode> TransactionParser::parseTransactionFlag()
{
    // always 00 01 , but optional (may not be there)
    enum WitFlag : uint8_t
    {
        FIRST_BYTE = 0x00,
        SECOND_BYTE = 0x01
    };

    // save pre-read position
    const size_t kCurrentPos = m_reader.getReadPosition();

    // read two bytes
    auto flagReadResult = m_reader.readData(2);
    RETURN_IF_UNEXPECTED(flagReadResult);

    const std::vector<uint8_t> flagBuffer = std::move(flagReadResult).value();

    if (flagBuffer[0] == FIRST_BYTE)
    {
        if (flagBuffer[1] == SECOND_BYTE)
        {
            LOG_VERBOSE("Witness flag is present.");
            return true;
        }
    }

    // no flag, return to pre-read position
    LOG_VERBOSE("Witness flag is not present.");
    const bool kSetPosResult = m_reader.setReadPosition(kCurrentPos);
    if (!kSetPosResult)
    {
        LOG_ERROR("setReadPosition failure.");
        return std::unexpected(AppErrorCode::FileError);
    }

    return false;
}

//================================================================================
// Method: parseAllTxIn
// Description: Reads the input counter and parses each transaction input. Returns
//              a vector of TxIn objects on success or AppErrorCode on failure.
//================================================================================
std::expected<std::vector<TxIn>, AppErrorCode> TransactionParser::parseAllTxIn()
{
    // In-counter
    LOG_VERBOSE("Parsing number of transaction inputs...");
    const auto kInCounterReadResult = m_reader.readDataCompactSize();
    RETURN_IF_UNEXPECTED(kInCounterReadResult);

    const uint64_t kInputCounter = kInCounterReadResult.value();
    LOG_VERBOSE("Input Counter = {}.", kInputCounter);

    // TxIn
    std::vector<TxIn> out;
    out.reserve(kInputCounter);
    for (size_t i = 0; i < kInputCounter; ++i)
    {
        LOG_VERBOSE("Parsing input #{}...", i + 1);
        auto parseResult = parseTxIn();
        RETURN_IF_UNEXPECTED(parseResult);

        out.push_back(std::move(parseResult).value());
        LOG_VERBOSE("Successfully parsed input #{}.", i + 1);
    }

    LOG_VERBOSE("Successfully parsed all inputs");
    return out;
}

//================================================================================
// Method: parseTxIn
// Description: Parses a single transaction input (TxIn). Steps:
//              - Reads the previous transaction hash.
//              - Reads the output index being referenced.
//              - Reads the script length.
//              - Reads the scriptSig data.
//              - Reads the sequence number.  
//              Returns a TxIn on success or AppErrorCode on failure.
//================================================================================
std::expected<TxIn, AppErrorCode> TransactionParser::parseTxIn()
{
    TxIn out;

    // Prevhash
    LOG_VERBOSE("Parsing previous transaction hash...");
    auto outHashReadResult = m_reader.readDataArray32();
    RETURN_IF_UNEXPECTED(outHashReadResult);

    out.m_hashPreviousTx = std::move(outHashReadResult).value();

    // TxOut index
    LOG_VERBOSE("Parsing previous Txout index...");
    const auto kPrevIndexReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kPrevIndexReadResult);

    out.m_prevTxOutIndex = kPrevIndexReadResult.value();

    // Script length
    LOG_VERBOSE("Parsing script length...");
    const auto kScriptLengthReadResult = m_reader.readDataCompactSize();
    RETURN_IF_UNEXPECTED(kScriptLengthReadResult);

    const uint64_t kScriptLength = kScriptLengthReadResult.value();
    LOG_VERBOSE("Script length = {}.", kScriptLength);

    // ScriptSig
    LOG_VERBOSE("Parsing ScriptSig...");
    auto scriptSigReadResult = m_reader.readData(kScriptLength);
    RETURN_IF_UNEXPECTED(scriptSigReadResult);

    out.m_scriptSignature = std::move(scriptSigReadResult).value();

    // Get address from ScriptSig
    out.m_inputAddress = std::move(m_scriptParser.parseInputScript(out.m_scriptSignature));

    // Sequence number, normally 0xFFFFFFFF
    LOG_VERBOSE("Parsing sequence number...");
    const auto kSequenceNumberReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kSequenceNumberReadResult);

    out.m_sequenceNumber = kSequenceNumberReadResult.value();

    return out;
}

//================================================================================
// Method: parseAllTxOut
// Description: Reads the output counter and parses each transaction output.
//              Returns a vector of TxOut objects on success or AppErrorCode on
//              failure.
//================================================================================
std::expected<std::vector<TxOut>, AppErrorCode> TransactionParser::parseAllTxOut()
{
    // Out-counter
    LOG_VERBOSE("Parsing number of transaction outputs...");
    const auto kOutCounterReadResult = m_reader.readDataCompactSize();
    RETURN_IF_UNEXPECTED(kOutCounterReadResult);

    const uint64_t kOutCounter = kOutCounterReadResult.value();
    LOG_VERBOSE("Output Counter = {}.", kOutCounter);

    // TxOut
    std::vector<TxOut> out;
    out.reserve(kOutCounter);
    for (size_t i = 0; i < kOutCounter; ++i)
    {
        LOG_VERBOSE("Parsing output #{}", i + 1);
        auto kParseResult = parseTxOut();
        RETURN_IF_UNEXPECTED(kParseResult);

        out.push_back(std::move(kParseResult).value());
        LOG_VERBOSE("Successfully parsed output #{}", i + 1);
    }

    LOG_VERBOSE("Successfully parsed all outputs");
    return out;
}

//================================================================================
// Method: parseTxOut
// Description: Parses a single transaction output (TxOut). Steps:
//              - Reads the output value in satoshis (uint64).
//              - Reads the script length (CompactSize).
//              - Reads the scriptPubKey data.  
//              Returns a TxOut on success or AppErrorCode on failure.
//================================================================================
std::expected<TxOut, AppErrorCode> TransactionParser::parseTxOut()
{
    TxOut out;

    // Value
    LOG_VERBOSE("Parsing satoshis value...");
    const auto kValueReadResult = m_reader.readDataUint64();
    RETURN_IF_UNEXPECTED(kValueReadResult);

    out.m_value = kValueReadResult.value();

    // Script length
    LOG_VERBOSE("Parsing script length...");
    const auto kScriptLenReadResult = m_reader.readDataCompactSize();
    RETURN_IF_UNEXPECTED(kScriptLenReadResult);

    const uint64_t kScriptLength = kScriptLenReadResult.value();
    LOG_VERBOSE("Script length = {}.", kScriptLength);

    // ScriptPubKey
    LOG_VERBOSE("Parsing ScriptPubKey...");
    auto scriptPubKeyReadResult = m_reader.readData(kScriptLength);
    RETURN_IF_UNEXPECTED(scriptPubKeyReadResult);

    out.m_scriptPubKey = std::move(scriptPubKeyReadResult).value();

    // Get address from ScriptPubKey
    out.m_outputAddress = std::move(m_scriptParser.parseOutputScript(out.m_scriptPubKey));

    return out;
}

//================================================================================
// Method: parseAllWitnesses
// Description: Parses all witness data for a segwit transaction. Reads the given
//              number of witness stacks, parsing each one in sequence, and stores
//              them in the output vector. Returns the vector on success or
//              AppErrorCode if any witness fails to parse.
//================================================================================
std::expected<std::vector<Witness>, AppErrorCode> TransactionParser::parseAllWitnesses(IN const size_t witCount)
{
    std::vector<Witness> out;
    out.reserve(witCount);
    for (size_t i = 0; i < witCount; ++i)
    {
        LOG_VERBOSE("Parsing witness #{}...", i + 1);
        auto witnessParseResult = parseWitness();
        RETURN_IF_UNEXPECTED(witnessParseResult);

        out.push_back(std::move(witnessParseResult).value());
        LOG_VERBOSE("Successfully parsed witness #{}...", i + 1);
    }

    return out;
}

//================================================================================
// Method: parseWitness
// Description: Parses a single witness stack. Reads the element count, then for
//              each element reads its length and data bytes. Returns a Witness on
//              success or an AppErrorCode on failure.
//================================================================================
std::expected<Witness, AppErrorCode> TransactionParser::parseWitness()
{
    LOG_VERBOSE("Parsing witness items count...");
    const auto kWitCountReadResult = m_reader.readDataCompactSize();
    RETURN_IF_UNEXPECTED(kWitCountReadResult);

    const uint64_t kWitnessStackItems = kWitCountReadResult.value();
    LOG_VERBOSE("Witness Items: {}.", kWitnessStackItems);

    Witness out;
    out.m_witnessData.reserve(kWitnessStackItems);
    for (size_t i = 0; i < kWitnessStackItems; ++i)
    {
        LOG_VERBOSE("Parsing witness item #{}...", i + 1);
        const auto kWitLenReadResult = m_reader.readDataCompactSize();
        RETURN_IF_UNEXPECTED(kWitLenReadResult);

        const uint64_t kWitnessItemLength = kWitLenReadResult.value();

        auto readWitReadResult = m_reader.readData(kWitnessItemLength);
        RETURN_IF_UNEXPECTED(readWitReadResult);

        std::vector<uint8_t> item = std::move(readWitReadResult).value();
        std::reverse(item.begin(), item.end());

        out.m_witnessData.push_back(std::move(item));
        LOG_VERBOSE("Successfully parsed witness item #{}...", i + 1);
    }

    LOG_VERBOSE("Successfully parsed all witness items.");

    return out;
}

//================================================================================
// Method: getCurrentTransactionHash
// Description: Computes the transaction hash (double SHA-256, reversed) for the
//              transaction defined by start and finish positions. Returns the 
//              computed hash on success, otherwise AppErrorCode.
//================================================================================
std::expected<std::array<uint8_t, 32>, AppErrorCode> TransactionParser::getCurrentTransactionHash(
    IN const size_t& startReadPosition,
    IN const size_t& finishReadPosition
)
{
    const size_t kTransactionSize = finishReadPosition - startReadPosition;
    const size_t kSavedReadPosition = finishReadPosition;

    LOG_VERBOSE("Generating current transaction hash...");

    // Get reading position back to the start of transaction
    const bool kSetPositionResult = m_reader.setReadPosition(startReadPosition);
    if (!kSetPositionResult)
    {
        LOG_ERROR("setReadPosition failure.");
        return std::unexpected(AppErrorCode::FileError);
    }

    // Read raw full transaction
    auto kReadResult = m_reader.readData(kTransactionSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> fullTransaction = std::move(kReadResult).value();

    // Check current read position
    const size_t kCurrentReadPos = m_reader.getReadPosition();
    if (kCurrentReadPos != kSavedReadPosition)
    {
        LOG_ERROR("Wrong read position after generation. Expected: {}, instead received: {}.", kSavedReadPosition, kCurrentReadPos);
        return std::unexpected(AppErrorCode::FileReadingFailure);
    }

    // Generate and return hash
    std::array<uint8_t, 32> hashResult = getDoubleSHA256(fullTransaction);
    std::reverse(hashResult.begin(), hashResult.end());
    return hashResult;
}
