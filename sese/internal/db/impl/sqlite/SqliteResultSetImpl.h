#pragma once
#include <sese/db/ResultSet.h>
#include <sqlite3.h>

namespace sese::db::impl {

    class SESE_DB_API SqliteResultSetImpl final : public ResultSet {
    public:
        SqliteResultSetImpl(char **table, size_t r, size_t c, char *error) noexcept;
        ~SqliteResultSetImpl() noexcept override;

        void reset() noexcept override;
        [[nodiscard]] bool next() noexcept override;
        [[nodiscard]] bool isNull(size_t index) const noexcept override;
        [[nodiscard]] size_t getColumns() const noexcept override;

        [[nodiscard]] int32_t getInteger(size_t index) const noexcept override;
        [[nodiscard]] int64_t getLong(size_t index) const noexcept override;
        [[nodiscard]] std::string_view getString(size_t index) const noexcept override;
        [[nodiscard]] double getDouble(size_t index) const noexcept override;
        [[nodiscard]] float getFloat(size_t index) const noexcept override;
        [[nodiscard]] std::optional<sese::DateTime> getDateTime(size_t index) const noexcept override;

    protected:
        size_t rows;
        size_t columns;
        char **table;
        char *error = nullptr;
        size_t current = 0;
    };

}// namespace sese::db::impl