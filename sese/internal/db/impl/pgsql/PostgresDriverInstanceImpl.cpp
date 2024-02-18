#include <sese/internal/db/impl/pgsql/PostgresDriverInstanceImpl.h>
#include <sese/internal/db/impl/pgsql/PostgresResultSetImpl.h>
#include <sstream>

using namespace sese::db;

impl::PostgresDriverInstanceImpl::PostgresDriverInstanceImpl(PGconn *conn) noexcept {
    this->conn = conn;
    auto status = PQstatus(conn);
    if (status != CONNECTION_OK) {
        error = (int) status;
    } else {
        error = 0;
    }
    result = nullptr;
}

impl::PostgresDriverInstanceImpl::~PostgresDriverInstanceImpl() noexcept {
    if (result) {
        PQclear(result);
        result = nullptr;
    }
    PQfinish(conn);
}

ResultSet::Ptr impl::PostgresDriverInstanceImpl::executeQuery(const char *sql) noexcept {
    if (result) {
        PQclear(result);
        result = nullptr;
    }
    result = PQexec(conn, sql);
    if (result == nullptr) {
        error = (int) PQstatus(conn);
        return nullptr;
    }
    auto status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK) {
        error = (int) status;
        return nullptr;
    } else {
        auto rt = std::make_unique<PostgresResultSetImpl>(result);
        error = 0;
        result = nullptr;
        return rt;
    }
}

int64_t impl::PostgresDriverInstanceImpl::executeUpdate(const char *sql) noexcept {
    if (result) {
        PQclear(result);
        result = nullptr;
    }
    result = PQexec(conn, sql);
    if (result == nullptr) {
        return -1;
    }
    auto status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK) {
        error = (int) status;
        return -1;
    } else {
        auto rt = (int64_t) PQcmdTuples(result);
        error = 0;
        PQclear(result);
        result = nullptr;
        return rt;
    }
}

PreparedStatement::Ptr impl::PostgresDriverInstanceImpl::createStatement(const char *sql) noexcept {
    int count = 0;
    std::stringstream stringBuilder;
    for (size_t i = 0; i < strlen(sql); ++i) {
        if (sql[i] == '?') {
            count++;
            stringBuilder << '$' << count;
        } else {
            stringBuilder << sql[i];
        }
    }

    std::string stmtString = stringBuilder.str();
    std::uniform_int_distribution<int> discreteDistribution(1, 65535);
    auto stmtName = "sese_stmt_" + std::to_string(discreteDistribution(rd));

    auto res = PQprepare(conn, stmtName.c_str(), stmtString.c_str(), count, nullptr);
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        this->error = 0;
        PQclear(res);
        return std::make_unique<impl::PostgresPreparedStatementImpl>(stmtName, stmtString, count, conn);
    } else {
        this->error = static_cast<int>(PQresultStatus(res));
        PQclear(res);
        return nullptr;
    }

}

int impl::PostgresDriverInstanceImpl::getLastError() const noexcept {
    return error;
}

const char *impl::PostgresDriverInstanceImpl::getLastErrorMessage() const noexcept {
    if (result) {
        return PQresultErrorMessage(result);
    } else {
        return PQerrorMessage(conn);
    }
}

bool impl::PostgresDriverInstanceImpl::setAutoCommit(bool enable) noexcept {
    return true;
}

bool impl::PostgresDriverInstanceImpl::getAutoCommit(bool &status) noexcept {
    return true;
}

bool impl::PostgresDriverInstanceImpl::commit() noexcept {
    if (result) {
        PQclear(result);
        result = nullptr;
    }

    result = PQexec(conn, "COMMIT;");
    if (result == nullptr) {
        return false;
    }

    auto status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK) {
        error = (int) status;
        return false;
    } else {
        error = 0;
        PQclear(result);
        result = nullptr;
        return true;
    }
}


bool impl::PostgresDriverInstanceImpl::rollback() noexcept {
    if (result) {
        PQclear(result);
        result = nullptr;
    }

    result = PQexec(conn, "rollback;");
    if (result == nullptr) {
        return false;
    }

    auto status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK) {
        error = (int) status;
        return false;
    } else {
        error = 0;
        PQclear(result);
        result = nullptr;
        return true;
    }
}


bool impl::PostgresDriverInstanceImpl::getInsertId(int64_t &id) const noexcept {
    return true;
}

bool impl::PostgresDriverInstanceImpl::begin() noexcept {
    if (result) {
        PQclear(result);
        result = nullptr;
    }

    result = PQexec(conn, "BEGIN;");

    auto status = PQresultStatus(result);

    if (status != PGRES_COMMAND_OK) {
        error = (int) status;
        return false;
    } else {
        error = 0;
        PQclear(result);
        result = nullptr;
        return true;
    }
}
