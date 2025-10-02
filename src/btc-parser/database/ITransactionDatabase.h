#pragma once

// Standard Library headers
#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Project headers
#include "Common.h"
#include "IDatabase.h"


struct TransactionDbData
{
    std::string txHash;
    std::vector<std::string> txIn;
    std::vector<std::string> txOut;
    std::string time;
    uint64_t satoshi;
};


//================================================================================
// Class: ITransactionDatabase
// Description: Interface for Bitcoin Transaction Database. Defines operations for 
//              inserting and querying transaction records without binding to a 
//              specific database technology.
//================================================================================
class ITransactionDatabase : public IDatabase
{
public:
    virtual ~ITransactionDatabase() = default;


    //================================================================================
    // Method: insert
    // Description: Inserts a single transaction record into the database.
    //================================================================================
    virtual bool insert(IN const TransactionDbData& row) = 0;


    //================================================================================
    // Method: insert
    // Description: Inserts multiple transaction records into the database in a 
    //              single operation.
    //================================================================================
    virtual bool insert(IN std::span<const TransactionDbData> rows) = 0;


    //================================================================================
    // Method: findByTransactionHash
    // Description: Retrieves a transaction record matching the given transaction hash.
    //================================================================================
    virtual std::optional<TransactionDbData> findByTransactionHash(IN const std::string& hash) = 0;
    
    
    //================================================================================
    // Method: findInputsByTransactionHash
    // Description: Retrieves all input addresses from the given transaction hash.
    //================================================================================
    virtual std::vector<std::string> findInputsByTransactionHash(IN const std::string& hash) = 0;
    
    
    //================================================================================
    // Method: findOutputsByTransactionHash
    // Description: Retrieves all output addresses from the given transaction hash.
    //================================================================================
    virtual std::vector<std::string> findOutputsByTransactionHash(IN const std::string& hash) = 0;

    
    //================================================================================
    // Method: findByWallet
    // Description: Retrieves all transaction records for the specified wallet address. 
    //================================================================================
    virtual std::vector<TransactionDbData> findByWallet(IN const std::string& address) = 0;
    
    
    //================================================================================
    // Method: findByWalletTimeRange
    // Description: Retrieves all transaction records for the specified wallet address
    //              that occurred within the given time range.
    //================================================================================
    virtual std::vector<TransactionDbData> findByWalletTimeRange(
        IN const std::string& address,
        IN const std::string& startDate,
        IN const std::string& endDate
    ) = 0;


    //================================================================================
    // Method: findByTimeRange
    // Description: Retrieves all transaction records that occurred within the 
    //              specified time range.
    //================================================================================
    virtual std::vector<TransactionDbData> findByTimeRange(IN const std::string& startDate, IN const std::string& endDate) = 0;
    
    
    //================================================================================
    // Method: findInputsByTimeRange
    // Description: Retrieves all input addresses from transactions that occurred 
    //              within the specified time range.
    //================================================================================
    virtual std::vector<std::string> findInputsByTimeRange(IN const std::string& startDate, IN const std::string& endDate) = 0;
    
    
    //================================================================================
    // Method: findOutputsByTimeRange
    // Description: Retrieves all output addresses from transactions that occurred 
    //              within the specified time range.
    //================================================================================
    virtual std::vector<std::string> findOutputsByTimeRange(IN const std::string& startDate, IN const std::string& endDate) = 0;

    
    //================================================================================
    // Method: findBySatoshi
    // Description: Retrieves all transaction records involving the specified amount 
    //              of satoshis.
    //================================================================================
    virtual std::vector<TransactionDbData> findBySatoshi(IN const uint64_t satoshi) = 0;
    
    
    //================================================================================
    // Method: findInputsBySatoshi
    // Description: Retrieves all input addresses from transactions involving the 
    //              specified amount of satoshis.
    //================================================================================
    virtual std::vector<std::string> findInputsBySatoshi(IN const uint64_t satoshi) = 0;
    
    
    //================================================================================
    // Method: findOutputsBySatoshi
    // Description: Retrieves all output addresses from transactions involving the 
    //              specified amount of satoshis.
    //================================================================================
    virtual std::vector<std::string> findOutputsBySatoshi(IN const uint64_t satoshi) = 0;

    
    //================================================================================
    // Method: findBySatoshiTimeRange
    // Description: Retrieves all transaction records involving the specified amount
    //              of satoshis that occurred within the specified time range.
    //================================================================================
    virtual std::vector<TransactionDbData> findBySatoshiTimeRange(
        IN const uint64_t satoshi, 
        IN const std::string& startDate,
        IN const std::string& endDate
    ) = 0;
    
    
    //================================================================================
    // Method: findInputsBySatoshiTimeRange
    // Description: Retrieves all input addresses from transactions involving the 
    //              specified amount of satoshis that occurred within the specified 
    //              time range.
    //================================================================================
    virtual std::vector<std::string> findInputsBySatoshiTimeRange(
        IN const uint64_t satoshi, 
        IN const std::string& startDate,
        IN const std::string& endDate
    ) = 0;
    
    
    //================================================================================
    // Method: findOutputsBySatoshiTimeRange
    // Description: Retrieves all output addresses from transactions involving the 
    //              specified amount of satoshis that occurred within the specified 
    //              time range.
    //================================================================================
    virtual std::vector<std::string> findOutputsBySatoshiTimeRange(
        IN const uint64_t satoshi, 
        IN const std::string& startDate,
        IN const std::string& endDate
    ) = 0;

protected:
    //================================================================================
    // Enum: TransactionColumns
    // Description: Represents columns in the database table.
    //================================================================================
    enum class TransactionColumns
    {
        TransactionHash = 0, // column representing unique transaction hash (primary key) 
        InAddress       = 1, // column representing all input addresses 
        OutAddress      = 2, // column representing all output addresses
        Time            = 3, // column representing time of transaction
        Satoshi         = 4, // column representing amount of satoshis transferred in transaction
        ColumnCount     = 5  // total number of columns
    };

protected:
    // Table name. For now hardcode, but can also be extracted from connection url
    inline static constexpr const char* kTableName = "Transactions";

    inline static constexpr const int kColumnCount = std::to_underlying(TransactionColumns::ColumnCount);

    // Actual column names, potentially will be better to extract them from db itself
    inline static constexpr std::array<std::string, kColumnCount> kColumnNames = {
        "transactionHash", 
        "txInAddress", 
        "txOutAddress", 
        "time", 
        "satoshi"
    };
};
