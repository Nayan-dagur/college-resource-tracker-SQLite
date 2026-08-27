#include "db_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace tracker {

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::open(const std::string& db_path) {
    if (db_) {
        close();
    }
    current_db_path_ = db_path;
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database Error] Could not open database: " 
                  << (db_ ? sqlite3_errmsg(db_) : "Unknown error") << std::endl;
        close();
        return false;
    }

    // Always enforce Foreign Key constraints in SQLite
    char* errmsg = nullptr;
    rc = sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database Error] Enabling foreign keys failed: " 
                  << (errmsg ? errmsg : "") << std::endl;
        sqlite3_free(errmsg);
    }
    return true;
}

void DatabaseManager::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::executeScript(const std::string& sql) {
    if (!db_) return false;
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database Error] Execution failed: " 
                  << (errmsg ? errmsg : "Unknown error") << std::endl;
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

bool DatabaseManager::executeFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Database Error] Cannot open SQL file: " << filepath << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return executeScript(buffer.str());
}

bool DatabaseManager::initializeSchemaAndSeeds(const std::string& schema_path, const std::string& seed_path) {
    if (!executeFile(schema_path)) {
        std::cerr << "[Database Error] Failed to initialize schema from " << schema_path << std::endl;
        return false;
    }
    if (!seed_path.empty()) {
        if (!executeFile(seed_path)) {
            std::cerr << "[Database Error] Failed to seed data from " << seed_path << std::endl;
            return false;
        }
    }
    return true;
}

bool DatabaseManager::beginTransaction() {
    return executeScript("BEGIN TRANSACTION;");
}

bool DatabaseManager::commit() {
    return executeScript("COMMIT;");
}

bool DatabaseManager::rollback() {
    return executeScript("ROLLBACK;");
}

StatementHandle DatabaseManager::prepare(const std::string& sql) {
    if (!db_) return StatementHandle(nullptr);
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), static_cast<int>(sql.length()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database Error] Failed to prepare statement: " 
                  << sqlite3_errmsg(db_) << "\nSQL: " << sql << std::endl;
        return StatementHandle(nullptr);
    }
    return StatementHandle(stmt);
}

bool DatabaseManager::bindInt(sqlite3_stmt* stmt, int index, int value) {
    if (!stmt) return false;
    return sqlite3_bind_int(stmt, index, value) == SQLITE_OK;
}

bool DatabaseManager::bindDouble(sqlite3_stmt* stmt, int index, double value) {
    if (!stmt) return false;
    return sqlite3_bind_double(stmt, index, value) == SQLITE_OK;
}

bool DatabaseManager::bindText(sqlite3_stmt* stmt, int index, const std::string& value) {
    if (!stmt) return false;
    // SQLITE_TRANSIENT makes SQLite make its own copy of the string data
    return sqlite3_bind_text(stmt, index, value.c_str(), static_cast<int>(value.length()), SQLITE_TRANSIENT) == SQLITE_OK;
}

bool DatabaseManager::bindNull(sqlite3_stmt* stmt, int index) {
    if (!stmt) return false;
    return sqlite3_bind_null(stmt, index) == SQLITE_OK;
}

int DatabaseManager::getColumnInt(sqlite3_stmt* stmt, int col) {
    return sqlite3_column_int(stmt, col);
}

double DatabaseManager::getColumnDouble(sqlite3_stmt* stmt, int col) {
    return sqlite3_column_double(stmt, col);
}

std::string DatabaseManager::getColumnText(sqlite3_stmt* stmt, int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

int64_t DatabaseManager::getLastInsertRowId() const {
    return db_ ? sqlite3_last_insert_rowid(db_) : 0;
}

int DatabaseManager::getChangesCount() const {
    return db_ ? sqlite3_changes(db_) : 0;
}

std::string DatabaseManager::getLastError() const {
    return db_ ? sqlite3_errmsg(db_) : "No database connection";
}

} // namespace tracker
