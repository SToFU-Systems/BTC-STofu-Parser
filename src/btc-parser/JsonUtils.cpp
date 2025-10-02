// Precompiled headers
#include "pch.h"

// Project headers
#include "Common.h"
#include "DataTypes.h"
#include "JsonUtils.h"
#include "Utils.h"

// for easier switch to nlohmann::json (if needed)
using jsonType = nlohmann::ordered_json;

namespace
{
    //================================================================================
    // Function: blockHeaderToJson
    // Description: Serializes a BlockHeader structure to a JSON object.
    //================================================================================
    void blockHeaderToJson(IN const BlockHeader& header, OUT jsonType& jsonOut)
    {
        constexpr const char* kHeaderKey = "blockHeader";
        constexpr const char* kVersionSubKey = "version";
        constexpr const char* kPrevBlockHashSubKey = "previousBlockHash";
        constexpr const char* kMerkleRootHashSubKey = "merkleRootHash";
        constexpr const char* kTimeSubKey = "time";
        constexpr const char* kBitsSubKey = "bits";
        constexpr const char* kNonceSubKey = "nonce";

        jsonOut[kHeaderKey] = {
            {kVersionSubKey, header.m_version},
            {kPrevBlockHashSubKey, hashToHexString(header.m_hashPrevBlock)},
            {kMerkleRootHashSubKey, hashToHexString(header.m_hashMerkleRoot)},
            {kTimeSubKey, toTime(header.m_time)},
            {kBitsSubKey, header.m_bits},
            {kNonceSubKey, header.m_nonce}
        };
    }

    //================================================================================
    // Function: txInToJson
    // Description: Serializes a single Input Transaction structure to a JSON object.
    //================================================================================
    void txInToJson(IN const TxIn& input, IN const size_t index, OUT jsonType& jsonOut)
    {
        constexpr const char* kTxInNumber = "txInNumber";
        constexpr const char* kPrevTxHashKey = "previousTransactionHash";
        constexpr const char* kPrevTxIndexKey = "previousTransactionIndex";
        constexpr const char* kScriptSignatureKey = "scriptSignature";
        constexpr const char* kInputAddress = "inputAddress";
        constexpr const char* kSequenceNumberKey = "sequenceNumber";

        jsonOut = {
            {kTxInNumber, index},
            {kPrevTxHashKey, hashToHexString(input.m_hashPreviousTx)},
            {kPrevTxIndexKey, input.m_prevTxOutIndex},
            {kScriptSignatureKey, hashToHexString(input.m_scriptSignature)},
            {kInputAddress, input.m_inputAddress},
            {kSequenceNumberKey, input.m_sequenceNumber}
        };
    }

    //================================================================================
    // Function: allTxInToJson
    // Description: Serializes all Input Transactions to a JSON object using the
    //              txInToJson function. 
    //================================================================================
    void allTxInToJson(IN const std::vector<TxIn>& inputs, OUT jsonType& jsonOut)
    {
        constexpr const char* kTxInKey = "txIn";
        const size_t kInputsCount = inputs.size();

        jsonOut[kTxInKey] = jsonType::array();
        for (size_t i = 0; i < kInputsCount; ++i)
        {
            jsonType txIn = jsonType::object();
            txInToJson(inputs[i], i + 1, txIn);

            jsonOut[kTxInKey].push_back(std::move(txIn));
        }
    }

    //================================================================================
    // Function: txOutToJson
    // Description: Serializes a single Output Transaction structure to a JSON object.
    //================================================================================
    void txOutToJson(IN const TxOut& output, IN const size_t index, OUT jsonType& jsonOut)
    {
        constexpr const char* kTxOutNumberKey = "txOutNumber";
        constexpr const char* kValueKey = "satoshiValue";
        constexpr const char* kScriptPubKey = "scriptPubKey";
        constexpr const char* kOutputAddress = "outputAddress";

        jsonOut = {
            {kTxOutNumberKey, index},
            {kValueKey, output.m_value},
            {kScriptPubKey, hashToHexString(output.m_scriptPubKey)},
            {kOutputAddress, output.m_outputAddress}
        };
    }

    //================================================================================
    // Function: allTxOutToJson
    // Description: Serializes all Output Transactions to a JSON object using the 
    //              txOutToJson function. 
    //================================================================================
    void allTxOutToJson(IN const std::vector<TxOut>& outputs, OUT jsonType& jsonOut)
    {
        constexpr const char* kTxInKey = "txOut";
        const size_t kOutputsCount = outputs.size();

        jsonOut[kTxInKey] = jsonType::array();
        for (size_t i = 0; i < kOutputsCount; ++i)
        {
            jsonType txIn = jsonType::object();
            txOutToJson(outputs[i], i + 1, txIn);

            jsonOut[kTxInKey].push_back(std::move(txIn));
        }
    }

    //================================================================================
    // Function: witnessToJson
    // Description: Serializes a single Witness structure to a JSON object.
    //================================================================================
    void witnessToJson(IN const Witness& witness, IN const size_t index, OUT jsonType& jsonOut)
    {
        constexpr const char* kWitnessNumberKey = "witnessNumber";
        constexpr const char* kItemKey = "items";

        jsonOut[kWitnessNumberKey] = index;
        jsonOut[kItemKey] = jsonType::array();
        for (const auto& chunk : witness.m_witnessData)
        {
            std::string item = hashToHexString(chunk);
            jsonOut[kItemKey].push_back(std::move(item));
        }
    }

    //================================================================================
    // Function: allWitnessToJson
    // Description: Serializes all Witnesses to a JSON object using the witnessToJson 
    //              function. 
    //================================================================================
    void allWitnessToJson(IN const std::vector<Witness>& witness, OUT jsonType& jsonOut)
    {
        constexpr const char* kWitnessKey = "witness";
        const size_t kWitnessCount = witness.size();

        jsonOut[kWitnessKey] = jsonType::array();
        for (size_t i = 0; i < kWitnessCount; ++i)
        {
            jsonType witnessJson = jsonType::object();
            witnessToJson(witness[i], i + 1, witnessJson);

            jsonOut[kWitnessKey].push_back(std::move(witnessJson));
        }
    }

    //================================================================================
    // Function: transactionToJson
    // Description: Serializes a single Transaction structure to a JSON object.
    //================================================================================
    void transactionToJson(IN const Transaction& tx, IN const size_t index, OUT jsonType& jsonOut)
    {
        constexpr const char* kTransactionNumberKey = "transactionNumber";
        constexpr const char* kTransactionHashKey = "transactionHash";
        constexpr const char* kVersionKey = "version";
        constexpr const char* kSegwitKey = "segwit";
        constexpr const char* kInputCountKey = "inputCount";
        constexpr const char* kOutputCountKey = "outputCount";
        constexpr const char* kLockTimeKey = "lockTime";

        jsonOut = {
            {kTransactionNumberKey, index},
            {kTransactionHashKey, hashToHexString(tx.m_currentTransactionHash)},
            {kVersionKey, tx.m_version},
            {kSegwitKey, tx.m_segwitFlag},
            {kInputCountKey, tx.m_vIn.size()},
            {kOutputCountKey, tx.m_vOut.size()},
            {kLockTimeKey, tx.m_lockTime}
        };

        // TxIn
        allTxInToJson(tx.m_vIn, jsonOut);

        // TxOut
        allTxOutToJson(tx.m_vOut, jsonOut);

        // Witness (if present)
        if (tx.m_segwitFlag)
            allWitnessToJson(tx.m_witnesses, jsonOut);
    }

    //================================================================================
    // Function: allTransactionsToJson
    // Description: Serializes all Block Transactions to a JSON object using the 
    //              transactionToJson function. 
    //================================================================================
    void allTransactionsToJson(
        IN const std::vector<Transaction>& transactions,
        OUT jsonType& jsonOut
    )
    {
        constexpr const char* kTotalNumberKey = "transactionsTotalNumber";
        constexpr const char* kTransactionKey = "transactions";
        const size_t kTransactionsCount = transactions.size();

        jsonOut[kTotalNumberKey] = kTransactionsCount;
        jsonOut[kTransactionKey] = jsonType::array();
        for (size_t i = 0; i < kTransactionsCount; ++i)
        {
            jsonType txJson = jsonType::object();
            transactionToJson(transactions[i], i + 1, txJson);

            jsonOut[kTransactionKey].push_back(std::move(txJson));
        }
    }
}

//================================================================================
// Function: blockToJson
// Description: Produces a complete JSON representation of a BTC Block. Initializes
//              the destination object, writes the current block hash (hex),
//              serializes the header via blockHeaderToJson function, and 
//              serializes all transactions via allTransactionsToJson function. 
//              Outputs the result into the provided json object.
//================================================================================
void blockToJson(IN const Block& block, OUT nlohmann::ordered_json& json)
{
    constexpr const char* kBlockHashKey = "blockHash";

    json = jsonType::object();

    // Current Block Hash
    json[kBlockHashKey] = hashToHexString(block.m_currentBlockHash);

    // Header
    blockHeaderToJson(block.m_blockHeader, json);

    // Transactions
    allTransactionsToJson(block.m_transactions, json);
}
