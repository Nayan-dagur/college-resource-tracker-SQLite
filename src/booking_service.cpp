#include "booking_service.hpp"
#include "db_manager.hpp"
#include "resource_service.hpp"
#include <iostream>

namespace tracker {

bool BookingService::hasScheduleConflict(int resource_id, const std::string& start_time, const std::string& end_time, int exclude_booking_id) {
    auto& db = DatabaseManager::getInstance();
    // Overlap condition: start_time < existing.end_time AND end_time > existing.start_time
    const std::string sql = 
        "SELECT COUNT(*) FROM bookings "
        "WHERE resource_id = ? AND status = 'active' AND booking_id != ? "
        "  AND (? < end_time AND ? > start_time);";

    auto stmt = db.prepare(sql);
    if (!stmt) return true; // Fail safe on error

    DatabaseManager::bindInt(stmt, 1, resource_id);
    DatabaseManager::bindInt(stmt, 2, exclude_booking_id);
    DatabaseManager::bindText(stmt, 3, start_time);
    DatabaseManager::bindText(stmt, 4, end_time);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = DatabaseManager::getColumnInt(stmt, 0);
        return count > 0;
    }
    return true;
}

bool BookingService::createBooking(int resource_id, int user_id,
                                  const std::string& start_time, const std::string& end_time,
                                  const std::string& purpose, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();

    // 1. Check if resource exists and its current status
    auto resOpt = ResourceService::getResourceById(resource_id);
    if (!resOpt.has_value()) {
        err_msg = "Resource with ID " + std::to_string(resource_id) + " not found.";
        return false;
    }

    if (resOpt->status == "under_maintenance") {
        err_msg = "Resource is currently under maintenance and cannot be booked.";
        return false;
    }
    if (resOpt->status == "decommissioned") {
        err_msg = "Resource has been decommissioned.";
        return false;
    }

    // 2. Validate timing input
    if (start_time >= end_time) {
        err_msg = "Invalid booking schedule: start time must be before end time.";
        return false;
    }

    // 3. Conflict Detection Query
    if (hasScheduleConflict(resource_id, start_time, end_time)) {
        err_msg = "Booking conflict! The selected resource is already booked during this time window.";
        return false;
    }

    // 4. ACID Transaction: Insert booking record and update resource status
    if (!db.beginTransaction()) {
        err_msg = "Failed to start database transaction.";
        return false;
    }

    const std::string insert_sql = 
        "INSERT INTO bookings (resource_id, user_id, start_time, end_time, purpose, status) "
        "VALUES (?, ?, ?, ?, ?, 'active');";

    auto stmt = db.prepare(insert_sql);
    if (!stmt) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    DatabaseManager::bindInt(stmt, 1, resource_id);
    DatabaseManager::bindInt(stmt, 2, user_id);
    DatabaseManager::bindText(stmt, 3, start_time);
    DatabaseManager::bindText(stmt, 4, end_time);
    DatabaseManager::bindText(stmt, 5, purpose);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    // Update resource status to 'booked'
    std::string update_err;
    if (!ResourceService::updateResourceStatus(resource_id, "booked", update_err)) {
        err_msg = "Failed to update resource status: " + update_err;
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        err_msg = "Failed to commit database transaction.";
        return false;
    }

    return true;
}

bool BookingService::completeBooking(int booking_id, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    auto bookingOpt = getBookingById(booking_id);
    if (!bookingOpt.has_value()) {
        err_msg = "Booking ID not found.";
        return false;
    }

    if (bookingOpt->status != "active") {
        err_msg = "Booking is already " + bookingOpt->status + ".";
        return false;
    }

    if (!db.beginTransaction()) {
        err_msg = "Failed to begin transaction.";
        return false;
    }

    const std::string sql = "UPDATE bookings SET status = 'completed' WHERE booking_id = ?;";
    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }
    DatabaseManager::bindInt(stmt, 1, booking_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    // Check if there are other active bookings for this resource
    int res_id = bookingOpt->resource_id;
    auto resOpt = ResourceService::getResourceById(res_id);
    if (resOpt.has_value() && resOpt->status != "under_maintenance" && resOpt->status != "decommissioned") {
        const std::string check_sql = "SELECT COUNT(*) FROM bookings WHERE resource_id = ? AND status = 'active';";
        auto check_stmt = db.prepare(check_sql);
        DatabaseManager::bindInt(check_stmt, 1, res_id);
        int active_count = 0;
        if (sqlite3_step(check_stmt) == SQLITE_ROW) {
            active_count = DatabaseManager::getColumnInt(check_stmt, 0);
        }
        if (active_count == 0) {
            std::string sub_err;
            ResourceService::updateResourceStatus(res_id, "available", sub_err);
        }
    }

    db.commit();
    return true;
}

bool BookingService::cancelBooking(int booking_id, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    auto bookingOpt = getBookingById(booking_id);
    if (!bookingOpt.has_value()) {
        err_msg = "Booking ID not found.";
        return false;
    }

    if (bookingOpt->status != "active") {
        err_msg = "Booking is already " + bookingOpt->status + ".";
        return false;
    }

    if (!db.beginTransaction()) {
        err_msg = "Failed to begin transaction.";
        return false;
    }

    const std::string sql = "UPDATE bookings SET status = 'cancelled' WHERE booking_id = ?;";
    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }
    DatabaseManager::bindInt(stmt, 1, booking_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        db.rollback();
        return false;
    }

    // Restore resource availability if no other active bookings
    int res_id = bookingOpt->resource_id;
    auto resOpt = ResourceService::getResourceById(res_id);
    if (resOpt.has_value() && resOpt->status != "under_maintenance" && resOpt->status != "decommissioned") {
        const std::string check_sql = "SELECT COUNT(*) FROM bookings WHERE resource_id = ? AND status = 'active';";
        auto check_stmt = db.prepare(check_sql);
        DatabaseManager::bindInt(check_stmt, 1, res_id);
        int active_count = 0;
        if (sqlite3_step(check_stmt) == SQLITE_ROW) {
            active_count = DatabaseManager::getColumnInt(check_stmt, 0);
        }
        if (active_count == 0) {
            std::string sub_err;
            ResourceService::updateResourceStatus(res_id, "available", sub_err);
        }
    }

    db.commit();
    return true;
}

std::optional<Booking> BookingService::getBookingById(int booking_id) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT b.booking_id, b.resource_id, r.resource_name, r.asset_tag, "
        "       b.user_id, u.full_name, u.email, "
        "       b.start_time, b.end_time, b.purpose, b.status, b.created_at "
        "FROM bookings b "
        "JOIN resources r ON b.resource_id = r.resource_id "
        "JOIN users u ON b.user_id = u.user_id "
        "WHERE b.booking_id = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) return std::nullopt;

    DatabaseManager::bindInt(stmt, 1, booking_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Booking b;
        b.booking_id = DatabaseManager::getColumnInt(stmt, 0);
        b.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        b.resource_name = DatabaseManager::getColumnText(stmt, 2);
        b.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        b.user_id = DatabaseManager::getColumnInt(stmt, 4);
        b.user_name = DatabaseManager::getColumnText(stmt, 5);
        b.user_email = DatabaseManager::getColumnText(stmt, 6);
        b.start_time = DatabaseManager::getColumnText(stmt, 7);
        b.end_time = DatabaseManager::getColumnText(stmt, 8);
        b.purpose = DatabaseManager::getColumnText(stmt, 9);
        b.status = DatabaseManager::getColumnText(stmt, 10);
        b.created_at = DatabaseManager::getColumnText(stmt, 11);
        return b;
    }
    return std::nullopt;
}

std::vector<Booking> BookingService::getAllBookings() {
    std::vector<Booking> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT b.booking_id, b.resource_id, r.resource_name, r.asset_tag, "
        "       b.user_id, u.full_name, u.email, "
        "       b.start_time, b.end_time, b.purpose, b.status, b.created_at "
        "FROM bookings b "
        "JOIN resources r ON b.resource_id = r.resource_id "
        "JOIN users u ON b.user_id = u.user_id "
        "ORDER BY b.booking_id DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Booking b;
        b.booking_id = DatabaseManager::getColumnInt(stmt, 0);
        b.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        b.resource_name = DatabaseManager::getColumnText(stmt, 2);
        b.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        b.user_id = DatabaseManager::getColumnInt(stmt, 4);
        b.user_name = DatabaseManager::getColumnText(stmt, 5);
        b.user_email = DatabaseManager::getColumnText(stmt, 6);
        b.start_time = DatabaseManager::getColumnText(stmt, 7);
        b.end_time = DatabaseManager::getColumnText(stmt, 8);
        b.purpose = DatabaseManager::getColumnText(stmt, 9);
        b.status = DatabaseManager::getColumnText(stmt, 10);
        b.created_at = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(b));
    }
    return list;
}

std::vector<Booking> BookingService::getActiveBookings() {
    std::vector<Booking> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT b.booking_id, b.resource_id, r.resource_name, r.asset_tag, "
        "       b.user_id, u.full_name, u.email, "
        "       b.start_time, b.end_time, b.purpose, b.status, b.created_at "
        "FROM bookings b "
        "JOIN resources r ON b.resource_id = r.resource_id "
        "JOIN users u ON b.user_id = u.user_id "
        "WHERE b.status = 'active' "
        "ORDER BY b.start_time ASC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Booking b;
        b.booking_id = DatabaseManager::getColumnInt(stmt, 0);
        b.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        b.resource_name = DatabaseManager::getColumnText(stmt, 2);
        b.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        b.user_id = DatabaseManager::getColumnInt(stmt, 4);
        b.user_name = DatabaseManager::getColumnText(stmt, 5);
        b.user_email = DatabaseManager::getColumnText(stmt, 6);
        b.start_time = DatabaseManager::getColumnText(stmt, 7);
        b.end_time = DatabaseManager::getColumnText(stmt, 8);
        b.purpose = DatabaseManager::getColumnText(stmt, 9);
        b.status = DatabaseManager::getColumnText(stmt, 10);
        b.created_at = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(b));
    }
    return list;
}

std::vector<Booking> BookingService::getBookingsByUser(int user_id) {
    std::vector<Booking> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT b.booking_id, b.resource_id, r.resource_name, r.asset_tag, "
        "       b.user_id, u.full_name, u.email, "
        "       b.start_time, b.end_time, b.purpose, b.status, b.created_at "
        "FROM bookings b "
        "JOIN resources r ON b.resource_id = r.resource_id "
        "JOIN users u ON b.user_id = u.user_id "
        "WHERE b.user_id = ? "
        "ORDER BY b.booking_id DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    DatabaseManager::bindInt(stmt, 1, user_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Booking b;
        b.booking_id = DatabaseManager::getColumnInt(stmt, 0);
        b.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        b.resource_name = DatabaseManager::getColumnText(stmt, 2);
        b.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        b.user_id = DatabaseManager::getColumnInt(stmt, 4);
        b.user_name = DatabaseManager::getColumnText(stmt, 5);
        b.user_email = DatabaseManager::getColumnText(stmt, 6);
        b.start_time = DatabaseManager::getColumnText(stmt, 7);
        b.end_time = DatabaseManager::getColumnText(stmt, 8);
        b.purpose = DatabaseManager::getColumnText(stmt, 9);
        b.status = DatabaseManager::getColumnText(stmt, 10);
        b.created_at = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(b));
    }
    return list;
}

std::vector<Booking> BookingService::getBookingsByResource(int resource_id) {
    std::vector<Booking> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT b.booking_id, b.resource_id, r.resource_name, r.asset_tag, "
        "       b.user_id, u.full_name, u.email, "
        "       b.start_time, b.end_time, b.purpose, b.status, b.created_at "
        "FROM bookings b "
        "JOIN resources r ON b.resource_id = r.resource_id "
        "JOIN users u ON b.user_id = u.user_id "
        "WHERE b.resource_id = ? "
        "ORDER BY b.booking_id DESC;";

    auto stmt = db.prepare(sql);
    if (!stmt) return list;

    DatabaseManager::bindInt(stmt, 1, resource_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Booking b;
        b.booking_id = DatabaseManager::getColumnInt(stmt, 0);
        b.resource_id = DatabaseManager::getColumnInt(stmt, 1);
        b.resource_name = DatabaseManager::getColumnText(stmt, 2);
        b.asset_tag = DatabaseManager::getColumnText(stmt, 3);
        b.user_id = DatabaseManager::getColumnInt(stmt, 4);
        b.user_name = DatabaseManager::getColumnText(stmt, 5);
        b.user_email = DatabaseManager::getColumnText(stmt, 6);
        b.start_time = DatabaseManager::getColumnText(stmt, 7);
        b.end_time = DatabaseManager::getColumnText(stmt, 8);
        b.purpose = DatabaseManager::getColumnText(stmt, 9);
        b.status = DatabaseManager::getColumnText(stmt, 10);
        b.created_at = DatabaseManager::getColumnText(stmt, 11);
        list.push_back(std::move(b));
    }
    return list;
}

std::vector<User> BookingService::getAllUsers() {
    std::vector<User> list;
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT u.user_id, u.full_name, u.email, u.role, u.department_id, d.dept_name "
        "FROM users u "
        "JOIN departments d ON u.department_id = d.department_id "
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

std::optional<User> BookingService::getUserById(int user_id) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = 
        "SELECT u.user_id, u.full_name, u.email, u.role, u.department_id, d.dept_name "
        "FROM users u "
        "JOIN departments d ON u.department_id = d.department_id "
        "WHERE u.user_id = ?;";

    auto stmt = db.prepare(sql);
    if (!stmt) return std::nullopt;

    DatabaseManager::bindInt(stmt, 1, user_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.user_id = DatabaseManager::getColumnInt(stmt, 0);
        u.full_name = DatabaseManager::getColumnText(stmt, 1);
        u.email = DatabaseManager::getColumnText(stmt, 2);
        u.role = DatabaseManager::getColumnText(stmt, 3);
        u.department_id = DatabaseManager::getColumnInt(stmt, 4);
        u.dept_name = DatabaseManager::getColumnText(stmt, 5);
        return u;
    }
    return std::nullopt;
}

bool BookingService::addUser(const std::string& full_name, const std::string& email,
                            const std::string& role, int department_id, std::string& err_msg) {
    auto& db = DatabaseManager::getInstance();
    const std::string sql = "INSERT INTO users (full_name, email, role, department_id) VALUES (?, ?, ?, ?);";
    auto stmt = db.prepare(sql);
    if (!stmt) {
        err_msg = db.getLastError();
        return false;
    }
    DatabaseManager::bindText(stmt, 1, full_name);
    DatabaseManager::bindText(stmt, 2, email);
    DatabaseManager::bindText(stmt, 3, role);
    DatabaseManager::bindInt(stmt, 4, department_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        err_msg = db.getLastError();
        return false;
    }
    return true;
}

} // namespace tracker
