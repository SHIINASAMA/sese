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

/// \file MariaPreparedStatementImpl.h
/// \brief Maria prepared statement implementation
/// \author kaoru
/// \date April 10, 2024

#pragma once

#include <sese/db/PreparedStatement.h>
#include <sese/internal/db/maria/MariaStmtResultSetImpl.h>

namespace sese::db::impl {

    /// \brief Maria prepared statement implementation
    class  MariaPreparedStatementImpl final : public PreparedStatement {
    public:
        explicit MariaPreparedStatementImpl(MYSQL_STMT *stmt, MYSQL_RES *meta, size_t count) noexcept;
        ~MariaPreparedStatementImpl() noexcept override;

        ResultSet::Ptr executeQuery() noexcept override;
        int64_t executeUpdate() noexcept override;

        bool setDouble(uint32_t index, const double &value) noexcept override;
        bool setFloat(uint32_t index, const float &value) noexcept override;
        bool setInteger(uint32_t index, const int32_t &value) noexcept override;
        bool setLong(uint32_t index, const int64_t &value) noexcept override;
        bool setText(uint32_t index, const char *value) noexcept override;
        bool setNull(uint32_t index) noexcept override;
        bool setDateTime(uint32_t index, const sese::DateTime &value) noexcept override;

        bool getColumnType(uint32_t index, MetadataType &type) noexcept override;
        int64_t getColumnSize(uint32_t index) noexcept override;

        [[nodiscard]] int getLastError() const noexcept override;
        [[nodiscard]] const char *getLastErrorMessage() const noexcept override;

        static bool mallocBindStruct(MYSQL_RES *meta, MYSQL_BIND **bind) noexcept;
        static void freeBindStruct(MYSQL_BIND *bind, size_t count) noexcept;
        static void reinterpret(MYSQL_BIND *target, enum_field_types expece_type, const void *buffer, size_t size) noexcept;

    protected:
        MYSQL_STMT *stmt;
        size_t count = 0;
        MYSQL_BIND *param;
        MYSQL_RES *meta;
    };

}// namespace sese::db