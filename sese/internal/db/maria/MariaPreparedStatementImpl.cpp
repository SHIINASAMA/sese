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

#include <sese/internal/db/maria/MariaPreparedStatementImpl.h>
#include <sese/log/Marco.h>

sese::db::impl::MariaPreparedStatementImpl::MariaPreparedStatementImpl(
        MYSQL_STMT *stmt,
        MYSQL_RES *meta,
        size_t count
) noexcept {
    this->stmt = stmt;
    this->meta = meta;
    this->count = count;

    this->param = static_cast<MYSQL_BIND *>(malloc(sizeof(MYSQL_BIND) * count));
    memset(this->param, 0, sizeof(MYSQL_BIND) * count);
}

sese::db::impl::MariaPreparedStatementImpl::~MariaPreparedStatementImpl() noexcept {
    mysql_stmt_close(stmt);

    for (int i = 0; i < count; ++i) {
        if (this->param[i].buffer) {
            free(this->param[i].buffer);
        }
    }

    free(param);
    if (meta) {
        mysql_free_result(meta);
    }
}

// Allocate memory for the possible data types of the result set
bool sese::db::impl::MariaPreparedStatementImpl::mallocBindStruct(MYSQL_RES *meta, MYSQL_BIND **bind) noexcept {
    *bind = static_cast<MYSQL_BIND *>(malloc(sizeof(MYSQL_BIND) * meta->field_count));
    memset(*bind, 0, sizeof(MYSQL_BIND) * meta->field_count);
    for (unsigned int i = 0; i < meta->field_count; i++) {
        auto &&item = (*bind)[i];
        item.is_null = static_cast<decltype(item.is_null)>(malloc(sizeof(char)));
        *item.is_null = 0;

        // Non-fixed-length data
        switch (meta->fields[i].type) {
            case MYSQL_TYPE_BLOB:
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VARCHAR:
            case MYSQL_TYPE_VAR_STRING:
                item.buffer = malloc(meta->fields[i].length);
                item.buffer_length = meta->fields[i].length;
                break;
            case MYSQL_TYPE_TINY:
                item.buffer = malloc(1);
                break;
            case MYSQL_TYPE_SHORT:
                item.buffer = malloc(2);
                break;
            case MYSQL_TYPE_LONG:
                item.buffer = malloc(4);
                break;
            case MYSQL_TYPE_LONGLONG:
                item.buffer = malloc(8);
                break;
            case MYSQL_TYPE_FLOAT:
                item.buffer = malloc(4);
                break;
            case MYSQL_TYPE_DOUBLE:
                item.buffer = malloc(8);
                break;
            case MYSQL_TYPE_TIME:
            case MYSQL_TYPE_DATE:
            case MYSQL_TYPE_DATETIME: {
                item.buffer = malloc(sizeof(MYSQL_TIME));
                memset(item.buffer, 0, sizeof(MYSQL_TIME));
                break;
            }
            default:
                return false;
        }
        item.buffer_type = meta->fields[i].type;
    }
    return true;
}

void sese::db::impl::MariaPreparedStatementImpl::freeBindStruct(MYSQL_BIND *bind, size_t count) noexcept {
    for (auto i = 0; i < count; ++i) {
        free(bind[i].buffer);
        free(bind[i].is_null);
    }
    free(bind);
}

sese::db::ResultSet::Ptr sese::db::impl::MariaPreparedStatementImpl::executeQuery() noexcept {
    if (!meta) return nullptr;

    if (mysql_stmt_bind_param(stmt, this->param)) {
        return nullptr;
    }
    MYSQL_BIND *result;
    if (!mallocBindStruct(meta, &result)) {
        goto freeResult;
    }
    if (mysql_stmt_bind_result(stmt, result)) {
        goto freeResult;
    }
    if (mysql_stmt_execute(stmt)) {
        goto freeResult;
    }
    if (mysql_stmt_store_result(stmt)) {
        goto freeResult;
    }
    return std::make_unique<impl::MariaStmtResultSet>(stmt, result, meta->field_count);

freeResult:
    freeBindStruct(result, meta->field_count);
    return nullptr;
}

int64_t sese::db::impl::MariaPreparedStatementImpl::executeUpdate() noexcept {
    if (mysql_stmt_bind_param(stmt, this->param)) {
        return -1;
    }

    if (mysql_stmt_execute(stmt)) {
        return -1;
    }
    return static_cast<int64_t>(mysql_stmt_affected_rows(stmt));
}

void sese::db::impl::MariaPreparedStatementImpl::reinterpret(MYSQL_BIND *target, enum_field_types expece_type, const void *buffer, size_t size) noexcept {
    if (target->buffer_type == expece_type) {
        memcpy(target->buffer, buffer, size);
        return;
    }

    if (target->buffer) {
        free(target->buffer);
    }
    target->buffer = malloc(size);
    target->buffer_type = expece_type;
    target->buffer_length = static_cast<decltype(target->buffer_length)>(size);
    memcpy(target->buffer, buffer, size);
}

bool sese::db::impl::MariaPreparedStatementImpl::setDouble(uint32_t index, const double &value) noexcept {
    if (index - 1 >= count) return false;
    reinterpret(this->param + index - 1, MYSQL_TYPE_DOUBLE, &value, sizeof(double));
    return true;
}

bool sese::db::impl::MariaPreparedStatementImpl::setFloat(uint32_t index, const float &value) noexcept {
    if (index - 1 >= count) return false;
    reinterpret(this->param + index - 1, MYSQL_TYPE_FLOAT, &value, sizeof(float));
    return true;
}

bool sese::db::impl::MariaPreparedStatementImpl::setInteger(uint32_t index, const int32_t &value) noexcept {
    if (index - 1 >= count) return false;
    reinterpret(this->param + index - 1, MYSQL_TYPE_LONG, &value, sizeof(int32_t));
    return true;
}

bool sese::db::impl::MariaPreparedStatementImpl::setLong(uint32_t index, const int64_t &value) noexcept {
    if (index - 1 >= count) return false;
    reinterpret(this->param + index - 1, MYSQL_TYPE_LONGLONG, &value, sizeof(int64_t));
    return true;
}

bool sese::db::impl::MariaPreparedStatementImpl::setText(uint32_t index, const char *value) noexcept {
    if (index - 1 >= count) return false;
    if (this->param[index - 1].buffer) {
        free(this->param[index - 1].buffer);
    }
    this->param[index - 1].buffer_length = static_cast<unsigned long>(strlen(value));
    this->param[index - 1].buffer = malloc(this->param[index - 1].buffer_length + 1);
    static_cast<char *>(this->param[index - 1].buffer)[this->param[index - 1].buffer_length] = 0;
    memcpy(this->param[index - 1].buffer, value, this->param[index - 1].buffer_length);
    this->param[index - 1].buffer_type = MYSQL_TYPE_VAR_STRING;
    return true;
}

bool sese::db::impl::MariaPreparedStatementImpl::setNull(uint32_t index) noexcept {
    if (index - 1 >= count) return false;
    if (this->param[index - 1].buffer_type == MYSQL_TYPE_NULL) {
        return true;
    }

    if (this->param[index - 1].buffer) {
        free(this->param[index - 1].buffer);
    }
    this->param[index - 1].buffer = nullptr;
    this->param[index - 1].buffer_type = MYSQL_TYPE_NULL;
    return true;
}

bool sese::db::impl::MariaPreparedStatementImpl::setDateTime(uint32_t index, const sese::DateTime &value) noexcept {
    if (index - 1 >= count) return false;
    if (this->param[index - 1].buffer_type != MYSQL_TYPE_DATETIME) {
        if (this->param[index - 1].buffer) {
            free(this->param[index - 1].buffer);
        }
        this->param[index - 1].buffer = malloc(sizeof(MYSQL_TIME));
        this->param[index - 1].buffer_length = sizeof(MYSQL_TIME);
        this->param[index - 1].buffer_type = MYSQL_TYPE_DATETIME;
    }

    memset(this->param[index - 1].buffer, 0, sizeof(MYSQL_TIME));
    auto time = static_cast<MYSQL_TIME *>(this->param[index - 1].buffer);
    time->year = value.getYears();
    time->month = value.getMonths();
    time->day = value.getDays();
    time->hour = value.getHours();
    time->minute = value.getMinutes();
    time->second = value.getSeconds();
    time->time_type = MYSQL_TIMESTAMP_DATETIME;

    return true;
}

int sese::db::impl::MariaPreparedStatementImpl::getLastError() const noexcept {
    return static_cast<int>(mysql_stmt_errno(stmt));
}

const char *sese::db::impl::MariaPreparedStatementImpl::getLastErrorMessage() const noexcept {
    return mysql_stmt_error(stmt);
}

bool sese::db::impl::MariaPreparedStatementImpl::getColumnType(uint32_t index, sese::db::MetadataType &type) noexcept {
    if ((nullptr == meta) || (meta->field_count <= index)) {
        return false;
    }

    switch (meta->fields[index].type) {
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
            type = MetadataType::TEXT;
            break;
        case MYSQL_TYPE_TINY:
            type = MetadataType::CHAR;
            break;
        case MYSQL_TYPE_SHORT:
            type = MetadataType::SHORT;
            break;
        case MYSQL_TYPE_LONG:
            type = MetadataType::INTEGER;
            break;
        case MYSQL_TYPE_LONGLONG:
            type = MetadataType::LONG;
            break;
        case MYSQL_TYPE_FLOAT:
            type = MetadataType::FLOAT;
            break;
        case MYSQL_TYPE_DOUBLE:
            type = MetadataType::DOUBLE;
            break;
        case MYSQL_TYPE_TIME:
            type = MetadataType::TIME;
            break;
        case MYSQL_TYPE_DATE:
            type = MetadataType::DATE;
            break;
        case MYSQL_TYPE_DATETIME:
            type = MetadataType::DATE_TIME;
            break;
        default:
            type = MetadataType::UNKNOWN;
            break;
    }
    return true;
}

int64_t sese::db::impl::MariaPreparedStatementImpl::getColumnSize(uint32_t index) noexcept {
    // When the type is binary data or string, the value has reference meaning
    // Note that the unit is bytes, and strings need to consider the character set
    if ((nullptr == meta) || (meta->field_count <= index)) {
        return -1;
    }
    return static_cast<int64_t>(meta->fields[index].length);
}
