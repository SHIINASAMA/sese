// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sese/internal/db/sqlite/SqliteDriverInstanceImpl.h>
#include <sese/internal/db/sqlite/SqliteResultSetImpl.h>

using namespace sese::db;

impl::SqliteDriverInstanceImpl::SqliteDriverInstanceImpl(sqlite3 *conn) noexcept {
    this->conn = conn;
}

impl::SqliteDriverInstanceImpl::~SqliteDriverInstanceImpl() noexcept {
    sqlite3_close_v2(conn);
}

ResultSet::Ptr impl::SqliteDriverInstanceImpl::executeQuery(const char *sql) noexcept {
    int rows;
    int columns;
    char **table;
    char *error = nullptr;
    int rt = sqlite3_get_table(conn, sql, &table, &rows, &columns, &error);
    if (0 != rt) return nullptr;
    return std::make_unique<SqliteResultSetImpl>(table, static_cast<size_t>(rows), static_cast<size_t>(columns), error);
}

int64_t impl::SqliteDriverInstanceImpl::executeUpdate(const char *sql) noexcept {
    char *error = nullptr;
    auto rt = sqlite3_exec(conn, sql, nullptr, nullptr, &error);
    if (error) sqlite3_free(error);
    if (rt == 0) {
        return sqlite3_changes(conn);
    } else {
        return -1;
    }
}

PreparedStatement::Ptr impl::SqliteDriverInstanceImpl::createStatement(const char *sql) noexcept {
    sqlite3_stmt *stmt;
    size_t len = strlen(sql);
    size_t count = 0;
    for (size_t i = 0; i < len; ++i) {
        if (sql[i] == '?') count++;
    }
    if (SQLITE_OK == sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr)) {
        return std::make_unique<impl::SqlitePreparedStatementImpl>(stmt, count);
    } else {
        return nullptr;
    }
}

int impl::SqliteDriverInstanceImpl::getLastError() const noexcept {
    return sqlite3_errcode(conn);
}

const char *impl::SqliteDriverInstanceImpl::getLastErrorMessage() const noexcept {
    return sqlite3_errmsg(conn);
}

bool impl::SqliteDriverInstanceImpl::setAutoCommit(bool enable) noexcept {
    return true;
}

bool impl::SqliteDriverInstanceImpl::getAutoCommit(bool &status) noexcept {
    status = sqlite3_get_autocommit(conn) != 0;
    return true;
}

bool impl::SqliteDriverInstanceImpl::commit() noexcept {
    char *error = nullptr;
    auto rt = sqlite3_exec(conn, "COMMIT;", nullptr, nullptr, &error);
    if (error) sqlite3_free(error);
    return rt == 0;
}

bool impl::SqliteDriverInstanceImpl::rollback() noexcept {
    char *error = nullptr;
    auto rt = sqlite3_exec(conn, "ROLLBACK;", nullptr, nullptr, &error);
    if (error) sqlite3_free(error);
    return rt == 0;
}

bool impl::SqliteDriverInstanceImpl::getInsertId(int64_t &id) const noexcept {
    id = (int64_t) sqlite3_last_insert_rowid(conn);
    if (id) return true;
    return false;

}

bool impl::SqliteDriverInstanceImpl::begin() noexcept {
    char *error = nullptr;
    auto rt = sqlite3_exec(conn, "BEGIN;", nullptr, nullptr, &error);
    if (error) sqlite3_free(error);
    return rt == 0;
}