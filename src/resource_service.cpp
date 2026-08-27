#include "resource_service.hpp"
#include "db_manager.hpp"
#include <iostream>

namespace tracker {

bool ResourceService::addResource(const std::string& asset_tag, const std::string& name,
                                  int category_id, int location_id, const std::string& purchase_date,
                                  std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "INSERT INTO resources (asset_tag, resource_name, category_id, location_id, status, purchase_date) "
        "VALUES (?, ?, ?, ?, 'available', ?);";

    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }

    DatabaseManager::bindText(stmt, 1, asset_tag);
    DatabaseManager::bindText(stmt, 2, name);
    DatabaseManager::bindInt(stmt, 3, category_id);
    DatabaseManager::bindInt(stmt, 4, location_id);
    DatabaseManager::bindText(stmt, 5, purchase_date);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

std::vector<Resource> ResourceService::getAllResources() {
    std::vector<Resource> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT r.resource_id, r.asset_tag, r.resource_name, r.category_id, c.category_name, "
        "       r.location_id, l.room_number, l.building, r.status, r.purchase_date "
        "FROM resources r "
        "JOIN categories c ON r.category_id = c.category_id "
        "JOIN locations l ON r.location_id = l.location_id "
        "ORDER BY r.resource_id ASC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Resource r;
        r.resource_id = DatabaseManager::getColumnInt(stmt, 0);
        r.asset_tag = DatabaseManager::getColumnText(stmt, 1);
        r.resource_name = DatabaseManager::getColumnText(stmt, 2);
        r.category_id = DatabaseManager::getColumnInt(stmt, 3);
        r.category_name = DatabaseManager::getColumnText(stmt, 4);
        r.location_id = DatabaseManager::getColumnInt(stmt, 5);
        r.room_number = DatabaseManager::getColumnText(stmt, 6);
        r.building = DatabaseManager::getColumnText(stmt, 7);
        r.status = DatabaseManager::getColumnText(stmt, 8);
        r.purchase_date = DatabaseManager::getColumnText(stmt, 9);
        list.push_back(std::move(r));
    }
    return list;
}

std::optional<Resource> ResourceService::getResourceById(int resource_id) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT r.resource_id, r.asset_tag, r.resource_name, r.category_id, c.category_name, "
        "       r.location_id, l.room_number, l.building, r.status, r.purchase_date "
        "FROM resources r "
        "JOIN categories c ON r.category_id = c.category_id "
        "JOIN locations l ON r.location_id = l.location_id "
        "WHERE r.resource_id = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) return std::nullopt;

    DatabaseManager::bindInt(stmt, 1, resource_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Resource r;
        r.resource_id = DatabaseManager::getColumnInt(stmt, 0);
        r.asset_tag = DatabaseManager::getColumnText(stmt, 1);
        r.resource_name = DatabaseManager::getColumnText(stmt, 2);
        r.category_id = DatabaseManager::getColumnInt(stmt, 3);
        r.category_name = DatabaseManager::getColumnText(stmt, 4);
        r.location_id = DatabaseManager::getColumnInt(stmt, 5);
        r.room_number = DatabaseManager::getColumnText(stmt, 6);
        r.building = DatabaseManager::getColumnText(stmt, 7);
        r.status = DatabaseManager::getColumnText(stmt, 8);
        r.purchase_date = DatabaseManager::getColumnText(stmt, 9);
        return r;
    }
    return std::nullopt;
}

std::optional<Resource> ResourceService::getResourceByTag(const std::string& asset_tag) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT r.resource_id, r.asset_tag, r.resource_name, r.category_id, c.category_name, "
        "       r.location_id, l.room_number, l.building, r.status, r.purchase_date "
        "FROM resources r "
        "JOIN categories c ON r.category_id = c.category_id "
        "JOIN locations l ON r.location_id = l.location_id "
        "WHERE r.asset_tag = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) return std::nullopt;

    DatabaseManager::bindText(stmt, 1, asset_tag);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Resource r;
        r.resource_id = DatabaseManager::getColumnInt(stmt, 0);
        r.asset_tag = DatabaseManager::getColumnText(stmt, 1);
        r.resource_name = DatabaseManager::getColumnText(stmt, 2);
        r.category_id = DatabaseManager::getColumnInt(stmt, 3);
        r.category_name = DatabaseManager::getColumnText(stmt, 4);
        r.location_id = DatabaseManager::getColumnInt(stmt, 5);
        r.room_number = DatabaseManager::getColumnText(stmt, 6);
        r.building = DatabaseManager::getColumnText(stmt, 7);
        r.status = DatabaseManager::getColumnText(stmt, 8);
        r.purchase_date = DatabaseManager::getColumnText(stmt, 9);
        return r;
    }
    return std::nullopt;
}

std::vector<Resource> ResourceService::filterResources(std::optional<int> category_id,
                                                      std::optional<int> location_id,
                                                      const std::string& status_filter) {
    std::vector<Resource> list;
    auto& db = DatabaseManager::getInstance();
    
    std::string sql = 
        "SELECT r.resource_id, r.asset_tag, r.resource_name, r.category_id, c.category_name, "
        "       r.location_id, l.room_number, l.building, r.status, r.purchase_date "
        "FROM resources r "
        "JOIN categories c ON r.category_id = c.category_id "
        "JOIN locations l ON r.location_id = l.location_id "
        "WHERE (1=1) ";

    if (category_id.has_value()) sql += " AND r.category_id = ? ";
    if (location_id.has_value()) sql += " AND r.location_id = ? ";
    if (!status_filter.empty() && status_filter != "all") sql += " AND r.status = ? ";
    sql += " ORDER BY r.resource_id ASC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    int param_idx = 1;
    if (category_id.has_value()) {
        DatabaseManager::bindInt(stmt, param_idx++, category_id.value());
    }
    if (location_id.has_value()) {
        DatabaseManager::bindInt(stmt, param_idx++, location_id.value());
    }
    if (!status_filter.empty() && status_filter != "all") {
        DatabaseManager::bindText(stmt, param_idx++, status_filter);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Resource r;
        r.resource_id = DatabaseManager::getColumnInt(stmt, 0);
        r.asset_tag = DatabaseManager::getColumnText(stmt, 1);
        r.resource_name = DatabaseManager::getColumnText(stmt, 2);
        r.category_id = DatabaseManager::getColumnInt(stmt, 3);
        r.category_name = DatabaseManager::getColumnText(stmt, 4);
        r.location_id = DatabaseManager::getColumnInt(stmt, 5);
        r.room_number = DatabaseManager::getColumnText(stmt, 6);
        r.building = DatabaseManager::getColumnText(stmt, 7);
        r.status = DatabaseManager::getColumnText(stmt, 8);
        r.purchase_date = DatabaseManager::getColumnText(stmt, 9);
        list.push_back(std::move(r));
    }
    return list;
}

bool ResourceService::updateResource(int resource_id, const std::string& name,
                                    int category_id, int location_id, const std::string& status,
                                    std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "UPDATE resources SET resource_name = ?, category_id = ?, location_id = ?, status = ? "
        "WHERE resource_id = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }

    DatabaseManager::bindText(stmt, 1, name);
    DatabaseManager::bindInt(stmt, 2, category_id);
    DatabaseManager::bindInt(stmt, 3, location_id);
    DatabaseManager::bindText(stmt, 4, status);
    DatabaseManager::bindInt(stmt, 5, resource_id);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

bool ResourceService::updateResourceStatus(int resource_id, const std::string& new_status, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "UPDATE resources SET status = ? WHERE resource_id = ?;";
    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }
    DatabaseManager::bindText(stmt, 1, new_status);
    DatabaseManager::bindInt(stmt, 2, resource_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

bool ResourceService::deleteResource(int resource_id, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "DELETE FROM resources WHERE resource_id = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }

    DatabaseManager::bindInt(stmt, 1, resource_id);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

std::vector<Category> ResourceService::getAllCategories() {
    std::vector<Category> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "SELECT category_id, category_name, description FROM categories ORDER BY category_id ASC;";
    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Category c;
        c.category_id = DatabaseManager::getColumnInt(stmt, 0);
        c.category_name = DatabaseManager::getColumnText(stmt, 1);
        c.description = DatabaseManager::getColumnText(stmt, 2);
        list.push_back(std::move(c));
    }
    return list;
}

bool ResourceService::addCategory(const std::string& name, const std::string& description, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "INSERT INTO categories (category_name, description) VALUES (?, ?);";
    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }
    DatabaseManager::bindText(stmt, 1, name);
    DatabaseManager::bindText(stmt, 2, description);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

std::vector<Location> ResourceService::getAllLocations() {
    std::vector<Location> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT l.location_id, l.room_number, l.building, l.department_id, d.dept_name "
        "FROM locations l "
        "JOIN departments d ON l.department_id = d.department_id "
        "ORDER BY l.location_id ASC;";
    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Location l;
        l.location_id = DatabaseManager::getColumnInt(stmt, 0);
        l.room_number = DatabaseManager::getColumnText(stmt, 1);
        l.building = DatabaseManager::getColumnText(stmt, 2);
        l.department_id = DatabaseManager::getColumnInt(stmt, 3);
        l.dept_name = DatabaseManager::getColumnText(stmt, 4);
        list.push_back(std::move(l));
    }
    return list;
}

bool ResourceService::addLocation(const std::string& room_number, const std::string& building, int department_id, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "INSERT INTO locations (room_number, building, department_id) VALUES (?, ?, ?);";
    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }
    DatabaseManager::bindText(stmt, 1, room_number);
    DatabaseManager::bindText(stmt, 2, building);
    DatabaseManager::bindInt(stmt, 3, department_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

std::vector<Department> ResourceService::getAllDepartments() {
    std::vector<Department> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "SELECT department_id, dept_name, building, contact_email FROM departments ORDER BY department_id ASC;";
    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Department d;
        d.department_id = DatabaseManager::getColumnInt(stmt, 0);
        d.dept_name = DatabaseManager::getColumnText(stmt, 1);
        d.building = DatabaseManager::getColumnText(stmt, 2);
        d.contact_email = DatabaseManager::getColumnText(stmt, 3);
        list.push_back(std::move(d));
    }
    return list;
}

std::vector<DepartmentSummary> ResourceService::getDepartmentSummary() {
    std::vector<DepartmentSummary> summaries;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT d.dept_name, "
        "       COUNT(r.resource_id) AS total_res, "
        "       SUM(CASE WHEN r.status = 'available' THEN 1 ELSE 0 END) AS avail_res, "
        "       SUM(CASE WHEN r.status = 'booked' THEN 1 ELSE 0 END) AS booked_res, "
        "       SUM(CASE WHEN r.status = 'under_maintenance' THEN 1 ELSE 0 END) AS maint_res "
        "FROM departments d "
        "LEFT JOIN locations l ON d.department_id = l.department_id "
        "LEFT JOIN resources r ON l.location_id = r.location_id "
        "GROUP BY d.department_id, d.dept_name "
        "ORDER BY total_res DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return summaries;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DepartmentSummary ds;
        ds.dept_name = DatabaseManager::getColumnText(stmt, 0);
        ds.total_resources = DatabaseManager::getColumnInt(stmt, 1);
        ds.available_resources = DatabaseManager::getColumnInt(stmt, 2);
        ds.booked_resources = DatabaseManager::getColumnInt(stmt, 3);
        ds.maintenance_resources = DatabaseManager::getColumnInt(stmt, 4);
        summaries.push_back(std::move(ds));
    }
    return summaries;
}

} // namespace tracker
