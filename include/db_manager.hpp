#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <iostream>

namespace tracker {

/**
 * @brief RAII Wrapper for sqlite3_stmt to guarantee statement cleanup and prevent memory leaks.
 */
class StatementHandle {
public:
    explicit StatementHandle(sqlite3_stmt* stmt = nullptr) : stmt_(stmt) {}
    ~StatementHandle() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
    }

    // Move-only semantics
    StatementHandle(const StatementHandle&) = delete;
    StatementHandle& operator=(const StatementHandle&) = delete;
    StatementHandle(StatementHandle&& other) noexcept : stmt_(other.stmt_) {
        other.stmt_ = nullptr;
    }
    StatementHandle& operator=(StatementHandle&& other) noexcept {
        if (this != &other) {
            if (stmt_) sqlite3_finalize(stmt_);
            stmt_ = other.stmt_;
            other.stmt_ = nullptr;
        }
        return *this;
    }

    sqlite3_stmt* get() const { return stmt_; }
    sqlite3_stmt** getAddressOf() { return &stmt_; }
    operator sqlite3_stmt*() const { return stmt_; }

private:
    sqlite3_stmt* stmt_{nullptr};
};

/**
 * @brief DatabaseManager handles SQLite connections, transactions, schema initialization,
 * and parameterized query preparation with full protection against SQL injection.
 */
class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    ~DatabaseManager();

    bool open(const std::string& db_path = "college_resources.db");
    void close();

    bool isConnected() const { return db_ != nullptr; }
    sqlite3* getRawDb() const { return db_; }

    // Execute multi-statement SQL strings (for schema migrations and seeds)
    bool executeScript(const std::string& sql);
    bool executeFile(const std::string& filepath);
    bool initializeSchemaAndSeeds(const std::string& schema_path, const std::string& seed_path);

    // Transaction Management for ACID Compliance
    bool beginTransaction();
    bool commit();
    bool rollback();

    // Prepare statement helper
    StatementHandle prepare(const std::string& sql);

    // Secure Parameter Binding Helpers (Uses 1-based indexing as per SQLite specification)
    static bool bindInt(sqlite3_stmt* stmt, int index, int value);
    static bool bindDouble(sqlite3_stmt* stmt, int index, double value);
    static bool bindText(sqlite3_stmt* stmt, int index, const std::string& value);
    static bool bindNull(sqlite3_stmt* stmt, int index);

    // Helper Column Getters
    static int getColumnInt(sqlite3_stmt* stmt, int col);
    static double getColumnDouble(sqlite3_stmt* stmt, int col);
    static std::string getColumnText(sqlite3_stmt* stmt, int col);

    // Diagnostics & Meta
    int64_t getLastInsertRowId() const;
    int getChangesCount() const;
    std::string getLastError() const;

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    sqlite3* db_{nullptr};
    std::string current_db_path_;
};

} // namespace tracker
