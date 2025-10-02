#pragma once

// Standard Library headers
#include <string>
#include <string_view>
#include <span>

#include "TransactionsDbPostgreSql.h"
#include "Common.h"


class PostgreQueryCommands
{
    using dbTrans = TransactionDbPostgreSql;
    using colsTrans = TransactionDbPostgreSql::TransactionColumns;

public:
    static std::string findByTransactionHash(IN const std::string& hash, IN const std::string& returnColumns = "*");

    static std::string findByWallet(IN const std::string& address, IN const std::string& returnColumns = "*");

    static std::string findByWalletTimeRange(
        IN const std::string& address,
        IN const std::string& startDate,
        IN const std::string& endDate,
        IN const std::string& returnColumns = "*"
    );

    static std::string findByTimeRange(
        IN const std::string& startDate,
        IN const std::string& endDate,
        IN const std::string& returnColumns = "*"
    );

    static std::string findBySatoshi(IN const uint64_t satoshi, IN const std::string& returnColumns = "*");

    static std::string findBySatoshiTimeRange(
        IN const uint64_t satoshi,
        IN const std::string& startDate,
        IN const std::string& endDate,
        IN const std::string& returnColumns = "*"
    );

private:
    static std::string selectCommand(IN const std::string& tableName, IN const std::string& capture = "*");

    static std::string rangeCommand(
        IN const std::string& column,
        IN const std::string& startRange,
        IN const std::string& endRange
    );

    static std::string anyArrayCommand(IN std::span<const std::string> columns);

    static std::string equalColumnValueCommand(
        IN const std::string& column,
        IN const std::string& value
    );

    static std::string equalValueStatementCommand(
        IN const std::string& value,
        IN const std::string& statement
    );
};
