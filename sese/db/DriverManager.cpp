#include <sese/db/DriverManager.h>
#include <sese/db/Util.h>

#ifdef HAS_MARIADB
#include <sese/internal/db/impl/maria/MariaDriverInstanceImpl.h>
#endif

#ifdef HAS_SQLITE
#include <sese/internal/db/impl/sqlite/SqliteDriverInstanceImpl.h>
#endif

#ifdef HAS_POSTGRES
#include <sese/internal/db/impl/pgsql/PostgresDriverInstanceImpl.h>
#endif

using namespace sese::db;

DriverInstance::Ptr DriverManager::getInstance(sese::db::DatabaseType type, const char *connectionString) noexcept {
    switch (type) {
#ifdef HAS_MARIADB
        case DatabaseType::MySql:
        case DatabaseType::Maria: {
            auto config = tokenize(connectionString);

            auto hostIterator = config.find("host");
            auto userIterator = config.find("user");
            auto pwdIterator = config.find("pwd");
            auto dbIterator = config.find("db");
            auto portIterator = config.find("port");
            if (hostIterator == config.end() ||
                userIterator == config.end() ||
                pwdIterator == config.end() ||
                dbIterator == config.end() ||
                portIterator == config.end()) {
                // 缺少必要字段
                return nullptr;
            }

            MYSQL *conn = mysql_init(nullptr);
            if (conn == nullptr) {
                // 此处通常是不可恢复错误触发，例如内存不足
                return nullptr;
            }

            mysql_real_connect(
                    conn,
                    hostIterator->second.c_str(),
                    userIterator->second.c_str(),
                    pwdIterator->second.c_str(),
                    dbIterator->second.c_str(),
                    std::stoi(portIterator->second),
                    nullptr,
                    0
            );

            return std::make_unique<impl::MariaDriverInstanceImpl>(conn);
        }
#endif
#ifdef HAS_SQLITE
        case DatabaseType::Sqlite: {
            sqlite3 *conn;
            sqlite3_open_v2(connectionString, &conn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
            return std::make_unique<impl::SqliteDriverInstanceImpl>(conn);
        }
#endif
#ifdef HAS_POSTGRES
        case DatabaseType::Postgres: {
            auto config = tokenize(connectionString);

            auto hostIterator = config.find("host");
            auto userIterator = config.find("user");
            auto pwdIterator = config.find("pwd");
            auto dbIterator = config.find("db");
            auto portIterator = config.find("port");
            if (hostIterator == config.end() ||
                userIterator == config.end() ||
                pwdIterator == config.end() ||
                dbIterator == config.end() ||
                portIterator == config.end()) {
                // 缺少必要字段
                return nullptr;
            }
            const char *keywords[] = {"host", "user", "password", "dbname", "port", nullptr};
            const char *values[] = {
                    hostIterator->second.c_str(),
                    userIterator->second.c_str(),
                    pwdIterator->second.c_str(),
                    dbIterator->second.c_str(),
                    portIterator->second.c_str(),
                    nullptr};

            PGconn *conn = PQconnectdbParams(keywords, values, 0);
            if (conn == nullptr) {
                return nullptr;
            }
            return std::make_unique<impl::PostgresDriverInstanceImpl>(conn);
        }
#endif
        default:
            return nullptr;
    }
}