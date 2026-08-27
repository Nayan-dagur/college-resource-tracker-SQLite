#include "db_manager.hpp"
#include "ui.hpp"
#include "resource_service.hpp"
#include "booking_service.hpp"
#include "maintenance_service.hpp"

#include <iostream>
#include <string>
#include <cassert>

using namespace tracker;

void runAutomatedTests() {
    std::cout << "\n=======================================================\n";
    std::cout << "  RUNNING AUTOMATED TEST SUITE FOR COLLEGE RESOURCE TRACKER\n";
    std::cout << "=======================================================\n\n";

    auto& db = DatabaseManager::getInstance();
    assert(db.isConnected() && "Database must be connected.");

    // Test 1: Query All Resources
    std::cout << "[TEST 1] Fetching resources...";
    auto resList = ResourceService::getAllResources();
    assert(!resList.empty() && "Resources list should not be empty after seeding.");
    std::cout << " PASSED (Found " << resList.size() << " items)\n";

    // Test 2: Add New Resource (CRUD - Create)
    std::cout << "[TEST 2] Registering test resource...";
    std::string err;
    std::string test_tag = "EQ-TEST-999";
    bool added = ResourceService::addResource(test_tag, "Unit Test Robotic Arm", 1, 1, "2026-08-25", err);
    assert(added && "Should register new test resource.");
    auto found = ResourceService::getResourceByTag(test_tag);
    assert(found.has_value() && found->resource_name == "Unit Test Robotic Arm");
    std::cout << " PASSED (ID: " << found->resource_id << ")\n";

    // Test 3: Update Resource (CRUD - Update)
    std::cout << "[TEST 3] Updating test resource...";
    bool updated = ResourceService::updateResource(found->resource_id, "Updated Robotic Arm V2", 1, 1, "available", err);
    assert(updated && "Should update resource.");
    auto foundUpdated = ResourceService::getResourceById(found->resource_id);
    assert(foundUpdated.has_value() && foundUpdated->resource_name == "Updated Robotic Arm V2");
    std::cout << " PASSED\n";

    // Test 4: Create Booking with Conflict Prevention (Transactions & Queries)
    std::cout << "[TEST 4] Testing booking lifecycle & overlap prevention...";
    int test_res_id = found->resource_id;
    bool b1 = BookingService::createBooking(test_res_id, 3, "2026-09-01 10:00", "2026-09-01 12:00", "Robotics Lab Trial", err);
    assert(b1 && "Booking 1 should succeed.");

    // Conflicting booking (overlapping time window)
    bool b2 = BookingService::createBooking(test_res_id, 4, "2026-09-01 11:00", "2026-09-01 13:00", "Conflict Attempt", err);
    assert(!b2 && "Conflicting booking must be rejected!");
    std::cout << " PASSED (Conflict properly caught and blocked)\n";

    // Test 5: Maintenance Ticket & Status Locking
    std::cout << "[TEST 5] Testing maintenance logging & status locking...";
    bool m1 = MaintenanceService::logMaintenanceTicket(test_res_id, 5, "Joint calibration motor loose", 45.0, err);
    assert(m1 && "Maintenance ticket creation should succeed.");
    auto resAfterMaint = ResourceService::getResourceById(test_res_id);
    assert(resAfterMaint->status == "under_maintenance" && "Resource status must lock to under_maintenance.");
    
    // Booking a resource under maintenance should fail
    bool bUnderMaint = BookingService::createBooking(test_res_id, 3, "2026-09-05 10:00", "2026-09-05 12:00", "Trial", err);
    assert(!bUnderMaint && "Cannot book equipment under maintenance.");
    std::cout << " PASSED (Status lock verified)\n";

    // Test 6: Resolving Maintenance Ticket
    std::cout << "[TEST 6] Resolving maintenance ticket...";
    auto openLogs = MaintenanceService::getOpenLogs();
    int test_log_id = 0;
    for (const auto& l : openLogs) {
        if (l.resource_id == test_res_id) {
            test_log_id = l.log_id;
            break;
        }
    }
    assert(test_log_id != 0 && "Found open maintenance log.");
    bool mResolve = MaintenanceService::updateMaintenanceStatus(test_log_id, "resolved", "Re-tightened servo gears", 45.0, err);
    assert(mResolve && "Maintenance resolution should succeed.");
    auto resAfterResolved = ResourceService::getResourceById(test_res_id);
    assert(resAfterResolved->status == "available" && "Resource status must return to available.");
    std::cout << " PASSED (Status restored to available)\n";

    // Test 7: Parameterized Query Defense against SQL Injection
    std::cout << "[TEST 7] Testing parameterized query SQL injection immunity...";
    std::string sqli = "EQ-TEST' OR '1'='1";
    auto sqliResult = ResourceService::getResourceByTag(sqli);
    assert(!sqliResult.has_value() && "SQL Injection payload must NOT return unauthorized records.");
    std::cout << " PASSED (Zero injection vulnerabilities)\n";

    // Test 8: Clean up test resource
    std::cout << "[TEST 8] Cleaning up test records...";
    // Note: Cancel/complete booking first
    auto userBookings = BookingService::getBookingsByResource(test_res_id);
    for (const auto& ub : userBookings) {
        BookingService::cancelBooking(ub.booking_id, err);
    }
    // Delete test record from maintenance & bookings to allow clean resource deletion
    db.executeScript("DELETE FROM maintenance_logs WHERE resource_id = " + std::to_string(test_res_id) + ";");
    db.executeScript("DELETE FROM bookings WHERE resource_id = " + std::to_string(test_res_id) + ";");
    bool del = ResourceService::deleteResource(test_res_id, err);
    assert(del && "Should delete test resource.");
    std::cout << " PASSED\n";

    std::cout << "\n=======================================================\n";
    std::cout << "  ALL 8 TEST SUITES COMPLETED SUCCESSFULLY! (100% PASS)\n";
    std::cout << "=======================================================\n\n";
}

int main(int argc, char* argv[]) {
    std::string db_file = "college_resources.db";
    std::string schema_file = "db/schema.sql";
    std::string seed_file = "db/seed_data.sql";

    bool test_mode = false;
    bool force_init = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test") {
            test_mode = true;
        } else if (arg == "--init-db" || arg == "--reset-db") {
            force_init = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "College Resource Tracker CLI\n"
                      << "Usage:\n"
                      << "  ./resource_tracker              Launch interactive console\n"
                      << "  ./resource_tracker --test       Run automated unit and security tests\n"
                      << "  ./resource_tracker --init-db    Initialize / seed the SQLite database\n";
            return 0;
        }
    }

    auto& db = DatabaseManager::getInstance();
    if (!db.open(db_file)) {
        std::cerr << "Fatal error: Could not open database file " << db_file << std::endl;
        return 1;
    }

    // Check if tables exist, otherwise initialize schema & seeds
    {
        auto stmt = db.prepare("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='resources';");
        int count = 0;
        if (stmt && sqlite3_step(stmt) == SQLITE_ROW) {
            count = DatabaseManager::getColumnInt(stmt, 0);
        }
        if (count == 0 || force_init) {
            std::cout << "[System] Initializing database tables and seed dataset...\n";
            if (!db.initializeSchemaAndSeeds(schema_file, seed_file)) {
                std::cerr << "Fatal error: Failed to initialize schema from " << schema_file << std::endl;
                return 1;
            }
            std::cout << "[System] Database initialization complete.\n";
        }
    }

    if (test_mode) {
        runAutomatedTests();
        return 0;
    }

    // Launch Interactive UI
    UI::runMainMenu();

    return 0;
}
