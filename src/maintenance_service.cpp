#include "maintenance_service.hpp"
#include "db_manager.hpp"
#include "resource_service.hpp"
#include <iostream>

namespace tracker {

bool MaintenanceService::logMaintenanceTicket(int resource_id, int technician_id,
                                             const std::string& issue_description, double estimated_cost,
                                             std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();

    auto resOpt = ResourceService::getResourceById(resource_id);
    if (!resOpt.has_value()) {
        err_msg = "Resource not found.";
        return false;
    }

    if (!db.beginTransaction()) {
        err_msg = "Failed to begin transaction.";
        return false;
    }

    const std::string insert_sql = 
        "INSERT INTO maintenance_logs (resource_id, technician_id, issue_description, cost, status) "
        "VALUES (?, ?, ?, ?, 'open');";

    auto stmt = db.prepare(insert_sql);
    if (!stmt) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    DatabaseManager::bindInt(stmt, 1, resource_id);
    DatabaseManager::bindInt(stmt, 2, technician_id);
    DatabaseManager::bindText(stmt, 3, issue_description);
    DatabaseManager::bindDouble(stmt, 4, estimated_cost);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    // Update resource status to 'under_maintenance'
    std::string sub_err;
    if (!ResourceService::updateResourceStatus(resource_id, "under_maintenance", sub_err)) {
        err_msg = "Failed to update resource status: " + sub_err;
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

bool MaintenanceService::updateMaintenanceStatus(int log_id, const std::string& status,
                                                const std::string& resolution_notes, double actual_cost,
                                                std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();

    auto logOpt = getLogById(log_id);
    if (!logOpt.has_value()) {
        err_msg = "Maintenance log ID not found.";
        return false;
    }

    if (!db.beginTransaction()) {
        err_msg = "Failed to begin transaction.";
        return false;
    }

    std::string sql;
    if (status == "resolved") {
        sql = "UPDATE maintenance_logs SET status = ?, resolution_notes = ?, cost = ?, "
              "end_date = datetime('now', 'localtime') WHERE log_id = ?;";
    } else {
        sql = "UPDATE maintenance_logs SET status = ?, resolution_notes = ?, cost = ? WHERE log_id = ?;";
    }

    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    DatabaseManager::bindText(stmt, 1, status);
    DatabaseManager::bindText(stmt, 2, resolution_notes);
    DatabaseManager::bindDouble(stmt, 3, actual_cost);
    DatabaseManager::bindInt(stmt, 4, log_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    // If resolved, restore the resource to 'available'
    if (status == "resolved") {
        std::string sub_err;
        if (!ResourceService::updateResourceStatus(logOpt->resource_id, "available", sub_err)) {
            err_msg = "Failed to restore resource status: " + sub_err;
            db.rollback();
            return false;
        }
    }

    db.commit();
    return true;
}

std::optional<MaintenanceLog> MaintenanceService::getLogById(int log_id) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT m.log_id, m.resource_id, r.resource_name, r.asset_tag, "
        "       m.technician_id, u.full_name, m.issue_description, "
        "       COALESCE(m.resolution_notes, ''), m.cost, m.start_date, "
        "       COALESCE(m.end_date, 'In Progress'), m.status "
        "FROM maintenance_logs m "
        "JOIN resources r ON m.resource_id = r.resource_id "
        "JOIN users u ON m.technician_id = u.user_id "
        "WHERE m.log_id = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) return std::nullopt;

    DatabaseManager::bindInt(stmt, 1, log_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        MaintenanceLog m;
        m.log_id = DatabaseManager::getColumnInt(stmt, 0);
        m.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        m.resource_name = DatabaseManager::getColumnText(stmt, 2);
        m.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        m.technician_id = DatabaseManager::getColumnInt(stmt, 4);
        m.technician_name = DatabaseManager::getColumnText(stmt, 5);
        m.issue_description = DatabaseManager::getColumnText(stmt, 6);
        m.resolution_notes = DatabaseManager::getColumnText(stmt, 7);
        m.cost = DatabaseManager::getColumnDouble(stmt, 8);
        m.start_date = DatabaseManager::getColumnText(stmt, 9);
        m.end_date = DatabaseManager::getColumnText(stmt, 10);
        m.status = DatabaseManager::getColumnText(stmt, 11);
        return m;
    }
    return std::nullopt;
}

std::vector<MaintenanceLog> MaintenanceService::getAllLogs() {
    std::vector<MaintenanceLog> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT m.log_id, m.resource_id, r.resource_name, r.asset_tag, "
        "       m.technician_id, u.full_name, m.issue_description, "
        "       COALESCE(m.resolution_notes, ''), m.cost, m.start_date, "
        "       COALESCE(m.end_date, 'In Progress'), m.status "
        "FROM maintenance_logs m "
        "JOIN resources r ON m.resource_id = r.resource_id "
        "JOIN users u ON m.technician_id = u.user_id "
        "ORDER BY m.log_id DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MaintenanceLog m;
        m.log_id = DatabaseManager::getColumnInt(stmt, 0);
        m.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        m.resource_name = DatabaseManager::getColumnText(stmt, 2);
        m.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        m.technician_id = DatabaseManager::getColumnInt(stmt, 4);
        m.technician_name = DatabaseManager::getColumnText(stmt, 5);
        m.issue_description = DatabaseManager::getColumnText(stmt, 6);
        m.resolution_notes = DatabaseManager::getColumnText(stmt, 7);
        m.cost = DatabaseManager::getColumnDouble(stmt, 8);
        m.start_date = DatabaseManager::getColumnText(stmt, 9);
        m.end_date = DatabaseManager::getColumnText(stmt, 10);
        m.status = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(m));
    }
    return list;
}

std::vector<MaintenanceLog> MaintenanceService::getOpenLogs() {
    std::vector<MaintenanceLog> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT m.log_id, m.resource_id, r.resource_name, r.asset_tag, "
        "       m.technician_id, u.full_name, m.issue_description, "
        "       COALESCE(m.resolution_notes, ''), m.cost, m.start_date, "
        "       COALESCE(m.end_date, 'In Progress'), m.status "
        "FROM maintenance_logs m "
        "JOIN resources r ON m.resource_id = r.resource_id "
        "JOIN users u ON m.technician_id = u.user_id "
        "WHERE m.status IN ('open', 'in_progress') "
        "ORDER BY m.log_id DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MaintenanceLog m;
        m.log_id = DatabaseManager::getColumnInt(stmt, 0);
        m.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        m.resource_name = DatabaseManager::getColumnText(stmt, 2);
        m.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        m.technician_id = DatabaseManager::getColumnInt(stmt, 4);
        m.technician_name = DatabaseManager::getColumnText(stmt, 5);
        m.issue_description = DatabaseManager::getColumnText(stmt, 6);
        m.resolution_notes = DatabaseManager::getColumnText(stmt, 7);
        m.cost = DatabaseManager::getColumnDouble(stmt, 8);
        m.start_date = DatabaseManager::getColumnText(stmt, 9);
        m.end_date = DatabaseManager::getColumnText(stmt, 10);
        m.status = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(m));
    }
    return list;
}

std::vector<MaintenanceLog> MaintenanceService::getLogsByResource(int resource_id) {
    std::vector<MaintenanceLog> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT m.log_id, m.resource_id, r.resource_name, r.asset_tag, "
        "       m.technician_id, u.full_name, m.issue_description, "
        "       COALESCE(m.resolution_notes, ''), m.cost, m.start_date, "
        "       COALESCE(m.end_date, 'In Progress'), m.status "
        "FROM maintenance_logs m "
        "JOIN resources r ON m.resource_id = r.resource_id "
        "JOIN users u ON m.technician_id = u.user_id "
        "WHERE m.resource_id = ? "
        "ORDER BY m.log_id DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    DatabaseManager::bindInt(stmt, 1, resource_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MaintenanceLog m;
        m.log_id = DatabaseManager::getColumnInt(stmt, 0);
        m.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        m.resource_name = DatabaseManager::getColumnText(stmt, 2);
        m.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        m.technician_id = DatabaseManager::getColumnInt(stmt, 4);
        m.technician_name = DatabaseManager::getColumnText(stmt, 5);
        m.issue_description = DatabaseManager::getColumnText(stmt, 6);
        m.resolution_notes = DatabaseManager::getColumnText(stmt, 7);
        m.cost = DatabaseManager::getColumnDouble(stmt, 8);
        m.start_date = DatabaseManager::getColumnText(stmt, 9);
        m.end_date = DatabaseManager::getColumnText(stmt, 10);
        m.status = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(m));
    }
    return list;
}

CostSummary MaintenanceService::getCostSummary() {
    CostSummary summary;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT COALESCE(SUM(cost), 0.0), "
        "       COUNT(log_id), "
        "       SUM(CASE WHEN status IN ('open', 'in_progress') THEN 1 ELSE 0 END), "
        "       SUM(CASE WHEN status = 'resolved' THEN 1 ELSE 0 END) "
        "FROM maintenance_logs;";

    auto stmt = db.prepare(sql);
    if (!stmt) return summary;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        summary.total_cost = DatabaseManager::getColumnDouble(stmt, 0);
        summary.total_tickets = DatabaseManager::getColumnInt(stmt, 1);
        summary.open_tickets = DatabaseManager::getColumnInt(stmt, 2);
        summary.resolved_tickets = DatabaseManager::getColumnInt(stmt, 3);
    }
    return summary;
}

std::vector<User> MaintenanceService::getTechnicians() {
    std::vector<User> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT u.user_id, u.full_name, u.email, u.role, u.department_id, d.dept_name "
        "FROM users u "
        "JOIN departments d ON u.department_id = d.department_id "
        "WHERE u.role = 'technician' "
        "ORDER BY u.user_id ASC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.user_id = DatabaseManager::getColumnInt(stmt, 0);
        u.full_name = DatabaseManager::getColumnText(stmt, 1);
        u.email = DatabaseManager::getColumnText(stmt, 2);
        u.role = DatabaseManager::getColumnText(stmt, 3);
        u.department_id = DatabaseManager::getColumnInt(stmt, 4);
        u.dept_name = DatabaseManager::getColumnText(stmt, 5);
        list.push_back(std::move(u));
    }
    return list;
}

} // namespace tracker
