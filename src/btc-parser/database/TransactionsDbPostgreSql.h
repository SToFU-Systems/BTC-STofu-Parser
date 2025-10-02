#pragma once

// Standard Library headers
#include <memory>
#include <string>
#include <string_view>
#include <span>
#include <vector>

// 3rd-party libraries
#include "pqxx/pqxx"

// Project headers
#include "Common.h"
#include "ITransactionDatabase.h"


//================================================================================
// Class: TransactionDbPostgreSql
// Description: PostgreSQL implementation of Transaction database.
//================================================================================
class TransactionDbPostgreSql final : public ITransactionDatabase
{
    friend class PostgreQueryCommands;

public:
    TransactionDbPostgreSql() = default;


    //================================================================================
    // Method: ~TransactionDbPostgreSql
    // Description: Destructor. Calls disconnect to an active PostgreSQL connection.
    //================================================================================
    ~TransactionDbPostgreSql();


    //================================================================================
    // Method: connect
    // Description: Establishes a new PostgreSQL connection using the provided
    //              connection URL. Constructs the connection object m_connection, 
    //              validates connectivity via isConnected, catches pqxx::sql_error 
    //              and logs the details.  Returns true on successful connection, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] bool connect(const std::string& connectionUrl) override;


    //================================================================================
    // Method: isConnected
    // Description: Checks whether the underlying pqxx::connection exists and is
    //              currently open. Returns true if so, false otherwise.
    //================================================================================
    [[nodiscard]] bool isConnected() noexcept override;


    //================================================================================
    // Method: disconnect
    // Description: Closes the active PostgreSQL connection if it's open.
    //================================================================================
    void disconnect() override;


    //================================================================================
    // Method: insert
    // Description: Inserts a single transaction record into the PostgreSQL database.
    //================================================================================
    bool insert(IN const TransactionDbData& row) override;


    //================================================================================
    // Method: insert
    // Description: Inserts multiple transaction records into the PostgreSQL  database
    //              in a single operation. Uses pqxx::stream_to for efficient insertion.
    //================================================================================
    bool insert(IN std::span<const TransactionDbData> rows) override;


    std::optional<TransactionDbData> findByTransactionHash(IN const std::string& hash) override;

    std::vector<std::string> findInputsByTransactionHash(IN const std::string& hash) override;

    std::vector<std::string> findOutputsByTransactionHash(IN const std::string& hash) override;

    std::vector<TransactionDbData> findByWallet(IN const std::string& address) override;

    std::vector<TransactionDbData> findByWalletTimeRange(
        IN const std::string& address,
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

    std::vector<TransactionDbData> findByTimeRange(
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

    std::vector<std::string> findInputsByTimeRange(
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

    std::vector<std::string> findOutputsByTimeRange(
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

    std::vector<TransactionDbData> findBySatoshi(IN const uint64_t satoshi) override;

    std::vector<std::string> findInputsBySatoshi(IN const uint64_t satoshi) override;

    std::vector<std::string> findOutputsBySatoshi(IN const uint64_t satoshi) override;

    std::vector<TransactionDbData> findBySatoshiTimeRange(
        IN const uint64_t satoshi,
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

    std::vector<std::string> findInputsBySatoshiTimeRange(
        IN const uint64_t satoshi,
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

    std::vector<std::string> findOutputsBySatoshiTimeRange(
        IN const uint64_t satoshi,
        IN const std::string& startDate,
        IN const std::string& endDate
    ) override;

private:
    //================================================================================
    // Method: logSqlError
    // Description: Logs the received pqxx::sql_error.
    //================================================================================
    void logSqlError(const pqxx::sql_error& error) const;


    pqxx::result findQuery(std::string_view queryCondition);

    std::vector<std::string> parseTextArray(std::string_view row) const;

    std::vector<std::string> parseTextArrayColumn(const pqxx::result& queryResult) const;

    TransactionDbData parseRow(const pqxx::row& row) const;

    std::vector<TransactionDbData> parseRows(const pqxx::result& rows) const;

private:
    std::unique_ptr<pqxx::connection> m_connection;
};
