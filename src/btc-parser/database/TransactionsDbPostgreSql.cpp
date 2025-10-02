// Precompiled headers
#include "pch.h"

// Project headers
#include "TransactionsDbPostgreSql.h"
#include "PostgreQueryCommands.h"
#include "Logger.hpp"


//================================================================================
// Method: ~TransactionDbPostgreSql
// Description: Destructor. Calls disconnect to an active PostgreSQL connection.
//================================================================================
TransactionDbPostgreSql::~TransactionDbPostgreSql()
{
    disconnect();
}

//================================================================================
// Method: connect
// Description: Establishes a new PostgreSQL connection using the provided
//              connection URL. Constructs the connection object m_connection, 
//              validates connectivity via isConnected, catches pqxx::sql_error 
//              and logs the details.  Returns true on successful connection, 
//              false otherwise.
//================================================================================
bool TransactionDbPostgreSql::connect(const std::string& connectionUrl)
{
    try
    {
        m_connection = std::make_unique<pqxx::connection>(connectionUrl);
    }
    catch (const pqxx::sql_error& e)
    {
        logSqlError(e);
        return false;
    }

    if (!isConnected())
        return false;

    return true;
}

//================================================================================
// Method: isConnected
// Description: Checks whether the underlying pqxx::connection exists and is
//              currently open. Returns true if so, false otherwise.
//================================================================================
bool TransactionDbPostgreSql::isConnected() noexcept
{
    return m_connection->is_open();
}

//================================================================================
// Method: disconnect
// Description: Closes the active PostgreSQL connection if it's open.
//================================================================================
void TransactionDbPostgreSql::disconnect()
{
    if (isConnected())
        m_connection->close();
}

std::optional<TransactionDbData> TransactionDbPostgreSql::findByTransactionHash(IN const std::string& hash)
{
    const std::string kCommand = PostgreQueryCommands::findByTransactionHash(hash);
    
    pqxx::result result = findQuery(kCommand);

    // one_row() throws if more than one rows in pqxx::result
    // transaction hash is primary key => unique, but just in case
    try
    {
        TransactionDbData parsedRow = parseRow(result.one_row());
        return parsedRow;
    }
    catch (const pqxx::unexpected_rows& e)
    {
        LOG_ERROR("Find by Transaction hash error: {}", e.what());
        return std::nullopt;
    }

    return std::nullopt;
}

std::vector<std::string> TransactionDbPostgreSql::findInputsByTransactionHash(IN const std::string& hash)
{
    constexpr int kInColumnNumber = std::to_underlying(TransactionColumns::InAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kInColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findByTransactionHash(hash, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<std::string> TransactionDbPostgreSql::findOutputsByTransactionHash(IN const std::string& hash)
{
    constexpr int kOutColumnNumber = std::to_underlying(TransactionColumns::OutAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kOutColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findByTransactionHash(hash, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<TransactionDbData> TransactionDbPostgreSql::findByWallet(IN const std::string& address)
{
    const std::string kCommand = PostgreQueryCommands::findByWallet(address);

    pqxx::result result = findQuery(kCommand);
    std::vector<TransactionDbData> parsedRows = parseRows(result);
    return parsedRows;
}

std::vector<TransactionDbData> TransactionDbPostgreSql::findByWalletTimeRange(
    IN const std::string& address, 
    IN const std::string& startDate, 
    IN const std::string& endDate
)
{
    const std::string kCommand = PostgreQueryCommands::findByWalletTimeRange(address, startDate, endDate);

    pqxx::result result = findQuery(kCommand);
    std::vector<TransactionDbData> parsedRows = parseRows(result);
    return parsedRows;
}

std::vector<TransactionDbData> TransactionDbPostgreSql::findByTimeRange(
    IN const std::string& startDate, 
    IN const std::string& endDate
)
{
    const std::string kCommand = PostgreQueryCommands::findByTimeRange(startDate, endDate);

    pqxx::result result = findQuery(kCommand);
    std::vector<TransactionDbData> parsedRows = parseRows(result);
    return parsedRows;
}

std::vector<std::string> TransactionDbPostgreSql::findInputsByTimeRange(
    IN const std::string& startDate, 
    IN const std::string& endDate
)
{
    constexpr int kInColumnNumber = std::to_underlying(TransactionColumns::InAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kInColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findByTimeRange(startDate, endDate, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<std::string> TransactionDbPostgreSql::findOutputsByTimeRange(
    IN const std::string& startDate, 
    IN const std::string& endDate
)
{
    constexpr int kOutColumnNumber = std::to_underlying(TransactionColumns::OutAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kOutColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findByTimeRange(startDate, endDate, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<TransactionDbData> TransactionDbPostgreSql::findBySatoshi(IN const uint64_t satoshi)
{
    const std::string kCommand = PostgreQueryCommands::findBySatoshi(satoshi);

    pqxx::result result = findQuery(kCommand);
    std::vector<TransactionDbData> parsedRows = parseRows(result);
    return parsedRows;
}

std::vector<std::string> TransactionDbPostgreSql::findInputsBySatoshi(IN const uint64_t satoshi)
{
    constexpr int kInColumnNumber = std::to_underlying(TransactionColumns::InAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kInColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findBySatoshi(satoshi, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<std::string> TransactionDbPostgreSql::findOutputsBySatoshi(IN const uint64_t satoshi)
{
    constexpr int kOutColumnNumber = std::to_underlying(TransactionColumns::OutAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kOutColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findBySatoshi(satoshi, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<TransactionDbData> TransactionDbPostgreSql::findBySatoshiTimeRange(
    IN const uint64_t satoshi,
    IN const std::string& startDate,
    IN const std::string& endDate
)
{
    const std::string kCommand = PostgreQueryCommands::findBySatoshiTimeRange(satoshi, startDate, endDate);

    pqxx::result result = findQuery(kCommand);
    std::vector<TransactionDbData> parsedRows = parseRows(result);
    return parsedRows;
}

std::vector<std::string> TransactionDbPostgreSql::findInputsBySatoshiTimeRange(
    IN const uint64_t satoshi,
    IN const std::string& startDate,
    IN const std::string& endDate
)
{
    constexpr int kInColumnNumber = std::to_underlying(TransactionColumns::InAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kInColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findBySatoshiTimeRange(satoshi, startDate, endDate, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

std::vector<std::string> TransactionDbPostgreSql::findOutputsBySatoshiTimeRange(
    IN const uint64_t satoshi,
    IN const std::string& startDate,
    IN const std::string& endDate
)
{
    constexpr int kOutColumnNumber = std::to_underlying(TransactionColumns::OutAddress);
    const std::string kReturnColumn = m_connection->quote_name(kColumnNames[kOutColumnNumber]);
    const std::string kCommand = PostgreQueryCommands::findBySatoshiTimeRange(satoshi, startDate, endDate, kReturnColumn);

    pqxx::result result = findQuery(kCommand);
    std::vector<std::string> parsedArray = parseTextArrayColumn(result);
    return parsedArray;
}

//================================================================================
//  Method: insert
// Description: Inserts a single transaction record into the PostgreSQL database.
//================================================================================
bool TransactionDbPostgreSql::insert(IN const TransactionDbData& row)
{
    const std::array<TransactionDbData, 1> kInRow(row);
    const bool kInsertResult = insert(kInRow);
    return kInsertResult;
}

//================================================================================
// Method: insert
// Description: Inserts multiple transaction records into the PostgreSQL  database
//              in a single operation. Uses pqxx::stream_to for efficient insertion.
//================================================================================
bool TransactionDbPostgreSql::insert(IN std::span<const TransactionDbData> rows)
{
    if (!isConnected())
    {
        LOG_ERROR("Failed to insert data. Reason: not connected to any table.");
        return false;
    }

    const std::string kQuotedTableName = m_connection->quote_table(kTableName);
    const std::string kQuotedColumns = m_connection->quote_columns(kColumnNames);

    try
    {
        pqxx::work inserter(*m_connection);
        pqxx::stream_to stream = pqxx::stream_to::raw_table(
            inserter,
            kQuotedTableName,
            kQuotedColumns
        );

        for (const auto& row : rows)
        {
            auto rowTuple = std::make_tuple(
                row.txHash,
                row.txIn,
                row.txOut,
                row.time,
                row.satoshi
            );
            stream.write_values(std::move(rowTuple));
        }

        stream.complete();
        inserter.commit();
    }
    catch (const pqxx::sql_error& e)
    {
        logSqlError(e);
        return false;
    }

    return true;
}

//================================================================================
// Method: logSqlError
// Description: Logs the received pqxx::sql_error.
//================================================================================
void TransactionDbPostgreSql::logSqlError(const pqxx::sql_error& error) const
{
    LOG_ERROR("SQL error {}: {}. Query: {}", 
        error.sqlstate(), error.what(), error.query());
}

pqxx::result TransactionDbPostgreSql::findQuery(std::string_view queryCommand)
{
    if (!isConnected())
        return pqxx::result();

    try
    {
        pqxx::work transaction(*m_connection);
        pqxx::result result = transaction.exec(queryCommand);
        return result;
    }
    catch (const pqxx::sql_error& e)
    {
        logSqlError(e);
        return pqxx::result();
    }

    return pqxx::result();
}

// 1) needs refactoring
// 2) std::find instead of std::getline
// 3) logs
std::vector<std::string> TransactionDbPostgreSql::parseTextArray(std::string_view row) const
{
    if (row.empty() || row == "{}")
        return {};

    if (row.front() != '{' && row.back() != '}')
        return {};

    const size_t kRowSize = row.size();
    std::string inner = std::string(row.substr(1, kRowSize - 2));
    std::stringstream ss(inner);
    std::string item;
    std::vector<std::string> parsedArray;

    while (std::getline(ss, item, ','))
    {
        if (!item.empty() && item.front() == '"')
            item.erase(0, 1);

        if (!item.empty() && item.back() == '"')
            item.pop_back();

        parsedArray.push_back(item);
    }

    return parsedArray;
}

std::vector<std::string> TransactionDbPostgreSql::parseTextArrayColumn(const pqxx::result& queryResult) const
{
    std::vector<std::string> parsedResult;
    for (const auto& it : queryResult)
    {
        for (const auto& data : it)
        {
            std::vector<std::string> parsedArray = parseTextArray(data.c_str());
            parsedResult.insert(parsedResult.end(), parsedArray.begin(), parsedArray.end());
        }
    }

    return parsedResult;
}

TransactionDbData TransactionDbPostgreSql::parseRow(const pqxx::row& row) const
{
    constexpr int inColumnNumber = std::to_underlying(TransactionColumns::InAddress);
    std::string in = row[inColumnNumber].as<std::string>();
    std::vector<std::string> inParsed = parseTextArray(in);

    constexpr int outColumnNumber = std::to_underlying(TransactionColumns::OutAddress);
    std::string out = row[outColumnNumber].as<std::string>();
    std::vector<std::string> outParsed = parseTextArray(out);

    return TransactionDbData{
        row[std::to_underlying(TransactionColumns::TransactionHash)].as<std::string>(),
        inParsed,
        outParsed,
        row[std::to_underlying(TransactionColumns::Time)].as<std::string>(),
        row[std::to_underlying(TransactionColumns::Satoshi)].as<uint64_t>()
    };
}

std::vector<TransactionDbData> TransactionDbPostgreSql::parseRows(const pqxx::result& rows) const
{
    std::vector<TransactionDbData> output;
    output.reserve(rows.size());
    for (const auto& row : rows)
    {
        TransactionDbData parsedRow = parseRow(row);
        output.push_back(std::move(parsedRow));
    }

    return output;
}
