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

/// \file SqlitePreparedStatementImpl.h
/// \brief SQLite prepared statement implementation
/// \author kaoru
/// \date April 10, 2024

#pragma once

#include <sese/util/Util.h>
#include <sese/db/PreparedStatement.h>
#include <sese/internal/db/sqlite/SqliteStmtResultSetImpl.h>

#include <set>

namespace sese::db::impl {

    /// \brief SQLite prepared statement implementation
    class  SqlitePreparedStatementImpl final : public PreparedStatement {
    public:
        explicit SqlitePreparedStatementImpl(sqlite3_stmt *stmt, size_t count) noexcept;
        ~SqlitePreparedStatementImpl() noexcept override;

        ResultSet::Ptr executeQuery() noexcept override;
        int64_t executeUpdate() noexcept override;
        bool setDouble(uint32_t index, const double &value) noexcept override;
        bool setFloat(uint32_t index, const float &value) noexcept override;
        bool setLong(uint32_t index, const int64_t &value) noexcept override;
        bool setInteger(uint32_t index, const int32_t &value) noexcept override;
        bool setText(uint32_t index, const char *value) noexcept override;
        bool setNull(uint32_t index) noexcept override;
        bool setDateTime(uint32_t index, const sese::DateTime &value) noexcept override;

        bool getColumnType(uint32_t index, MetadataType &type) noexcept override;
        int64_t getColumnSize(uint32_t index) noexcept override;

        [[nodiscard]] int getLastError() const noexcept override;
        [[nodiscard]] const char *getLastErrorMessage() const noexcept override;

    protected:
        sqlite3_stmt *stmt;
        bool stmtStatus = false;
        bool *isManual;
        size_t count = 0;
        void **buffer;

        static std::string splitBefore(const std::string &str);
        static const char *INTEGER_AFFINITY_SET[];
        static const char *TEXT_AFFINITY_SET[];
        static const char *REAL_AFFINITY_SET[];
    };

}// namespace sese::db::impl