#include "pch.h"

#include "PostgreQueryCommands.h"

namespace
{
    inline static std::string singleQuote(IN const std::string& str)
    {
        return "'" + str + "'";
    }

    inline static std::string doubleQuote(IN const std::string& str)
    {
        return '"' + str + '"';
    }
}

std::string PostgreQueryCommands::findByTransactionHash(IN const std::string& hash, IN const std::string& returnColumns)
{
    constexpr int kHashColNumber = std::to_underlying(colsTrans::TransactionHash);

    const std::string kSelect = selectCommand(dbTrans::kTableName, returnColumns);
    const std::string kCondition = equalColumnValueCommand(dbTrans::kColumnNames[kHashColNumber], hash);

    const std::string kResult = kSelect + " " + kCondition;
    return kResult;
}

std::string PostgreQueryCommands::findByWallet(IN const std::string& address, IN const std::string& returnColumns)
{
    constexpr int kInputColNumber = std::to_underlying(colsTrans::InAddress);
    constexpr int kOutputColNumber = std::to_underlying(colsTrans::OutAddress);

    constexpr std::array<std::string, 2> addressColumns = {
        dbTrans::kColumnNames[kInputColNumber],
        dbTrans::kColumnNames[kOutputColNumber]
    };

    const std::string kStatement = anyArrayCommand(addressColumns);

    const std::string kSelect = selectCommand(dbTrans::kTableName, returnColumns);
    const std::string kCondition = equalValueStatementCommand(address, kStatement);

    const std::string kResult = kSelect + " " + kCondition;
    return kResult;
}

std::string PostgreQueryCommands::findByWalletTimeRange(
    IN const std::string& address,
    IN const std::string& startDate,
    IN const std::string& endDate,
    IN const std::string& returnColumns
)
{
    constexpr int kTimeColNumber = std::to_underlying(colsTrans::Time);

    const std::string kFindWallet = findByWallet(address, returnColumns);
    const std::string kCondition = rangeCommand(dbTrans::kColumnNames[kTimeColNumber], startDate, endDate);

    const std::string kResult = kFindWallet + " AND " + kCondition;
    return kResult;
}

std::string PostgreQueryCommands::findByTimeRange(
    IN const std::string& startDate,
    IN const std::string& endDate,
    IN const std::string& returnColumns
)
{
    constexpr int kTimeColNumber = std::to_underlying(colsTrans::Time);

    const std::string kSelect = selectCommand(dbTrans::kTableName, returnColumns);
    const std::string kCondition = rangeCommand(dbTrans::kColumnNames[kTimeColNumber], startDate, endDate);

    const std::string kResult = kSelect + " " + kCondition;
    return kResult;
}

std::string PostgreQueryCommands::findBySatoshi(IN const uint64_t satoshi, IN const std::string& returnColumns)
{
    constexpr int kSatoshiColNumber = std::to_underlying(colsTrans::Satoshi);

    const std::string kSelect = selectCommand(dbTrans::kTableName, returnColumns);
    const std::string kCondition = equalColumnValueCommand(
        dbTrans::kColumnNames[kSatoshiColNumber],
        std::to_string(satoshi)
    );

    const std::string kResult = kSelect + " " + kCondition;
    return kResult;
}

std::string PostgreQueryCommands::findBySatoshiTimeRange(
    IN const uint64_t satoshi,
    IN const std::string& startDate,
    IN const std::string& endDate,
    IN const std::string& returnColumns
)
{
    constexpr int kTimeColNumber = std::to_underlying(colsTrans::Time);

    const std::string kFindWallet = findBySatoshi(satoshi, returnColumns);
    const std::string kCondition = rangeCommand(dbTrans::kColumnNames[kTimeColNumber], startDate, endDate);

    const std::string kResult = kFindWallet + " AND " + kCondition;
    return kResult;
}

std::string PostgreQueryCommands::selectCommand(IN const std::string& tableName, IN const std::string& capture)
{
    const std::string kCommand = "SELECT " + capture + " FROM "
        + doubleQuote(tableName) + " WHERE";
    return kCommand;
}

std::string PostgreQueryCommands::rangeCommand(
    IN const std::string& column,
    IN const std::string& startRange,
    IN const std::string& endRange
)
{
    const std::string kCommand = doubleQuote(column) + " BETWEEN " 
        + singleQuote(startRange) + " AND " + singleQuote(endRange);
    return kCommand;
}

std::string PostgreQueryCommands::anyArrayCommand(IN std::span<const std::string> columns)
{
    const size_t kColumnCount = columns.size();

    std::string command = "ANY (ARRAY[";
    for (size_t i = 0; i < kColumnCount; ++i)
    {
        std::string quotedCol = doubleQuote(columns[i]);
        if (i < kColumnCount - 1)
            quotedCol += ", ";

        command.append(std::move(quotedCol));
    }

    command.append("])");
    return command;
}

std::string PostgreQueryCommands::equalColumnValueCommand(IN const std::string& column, IN const std::string& value)
{
    const std::string kCommand = doubleQuote(column) + " = " + singleQuote(value);
    return kCommand;
}

std::string PostgreQueryCommands::equalValueStatementCommand(IN const std::string& value, IN const std::string& statement)
{
    const std::string kCommand = singleQuote(value) + " = " + statement;
    return kCommand;
}
