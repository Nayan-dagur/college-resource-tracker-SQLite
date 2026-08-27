#include "ui.hpp"
#include "db_manager.hpp"
#include "resource_service.hpp"
#include "booking_service.hpp"
#include "maintenance_service.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>

namespace tracker {

// ANSI Terminal Color Codes
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    const std::string GRAY    = "\033[90m";
}

void UI::printHeader(const std::string& title) {
    std::cout << "\n" << Color::BOLD << Color::CYAN 
              << "================================================================================" 
              << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::WHITE << "  " << title << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::CYAN 
              << "================================================================================" 
              << Color::RESET << "\n";
}

void UI::printSuccess(const std::string& msg) {
    std::cout << Color::BOLD << Color::GREEN << "[SUCCESS] " << Color::RESET << msg << "\n";
}

void UI::printError(const std::string& msg) {
    std::cout << Color::BOLD << Color::RED << "[ERROR] " << Color::RESET << msg << "\n";
}

void UI::printWarning(const std::string& msg) {
    std::cout << Color::BOLD << Color::YELLOW << "[WARNING] " << Color::RESET << msg << "\n";
}

void UI::printInfo(const std::string& msg) {
    std::cout << Color::BOLD << Color::BLUE << "[INFO] " << Color::RESET << msg << "\n";
}

static std::string truncateString(const std::string& str, size_t max_len) {
    if (str.length() <= max_len) return str;
    return str.substr(0, max_len - 3) + "...";
}

void UI::displayResourcesTable(const std::vector<Resource>& resources) {
    if (resources.empty()) {
        printWarning("No equipment records found.");
        return;
    }

    std::cout << Color::GRAY << "+-----+-------------+--------------------------------+--------------------+------------------------+-------------------+------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(3)  << "ID" << " | "
              << std::setw(11) << "Asset Tag" << " | "
              << std::setw(30) << "Equipment Name" << " | "
              << std::setw(18) << "Category" << " | "
              << std::setw(22) << "Location / Lab" << " | "
              << std::setw(17) << "Status" << " | "
              << std::setw(10) << "Purchased" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+-----+-------------+--------------------------------+--------------------+------------------------+-------------------+------------+" << Color::RESET << "\n";

    for (const auto& r : resources) {
        std::string status_color = Color::GREEN;
        if (r.status == "booked") status_color = Color::YELLOW;
        else if (r.status == "under_maintenance") status_color = Color::RED;
        else if (r.status == "decommissioned") status_color = Color::GRAY;

        std::string loc_str = r.room_number + " (" + r.building + ")";

        std::cout << "| " << std::left
                  << std::setw(3)  << r.resource_id << " | "
                  << std::setw(11) << truncateString(r.asset_tag, 11) << " | "
                  << std::setw(30) << truncateString(r.resource_name, 30) << " | "
                  << std::setw(18) << truncateString(r.category_name, 18) << " | "
                  << std::setw(22) << truncateString(loc_str, 22) << " | "
                  << status_color << std::setw(17) << r.status << Color::RESET << " | "
                  << std::setw(10) << r.purchase_date << " |\n";
    }
    std::cout << Color::GRAY << "+-----+-------------+--------------------------------+--------------------+------------------------+-------------------+------------+" << Color::RESET << "\n";
}

void UI::displayBookingsTable(const std::vector<Booking>& bookings) {
    if (bookings.empty()) {
        printWarning("No booking records found.");
        return;
    }

    std::cout << Color::GRAY << "+----+--------------------------------+----------------------+--------------------+--------------------+-----------+--------------------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(2)  << "ID" << " | "
              << std::setw(30) << "Resource Name" << " | "
              << std::setw(20) << "Booked By" << " | "
              << std::setw(18) << "Start Time" << " | "
              << std::setw(18) << "End Time" << " | "
              << std::setw(9)  << "Status" << " | "
              << std::setw(30) << "Purpose" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+----+--------------------------------+----------------------+--------------------+--------------------+-----------+--------------------------------+" << Color::RESET << "\n";

    for (const auto& b : bookings) {
        std::string status_color = Color::GREEN;
        if (b.status == "completed") status_color = Color::BLUE;
        else if (b.status == "cancelled") status_color = Color::RED;

        std::cout << "| " << std::left
                  << std::setw(2)  << b.booking_id << " | "
                  << std::setw(30) << truncateString(b.resource_name, 30) << " | "
                  << std::setw(20) << truncateString(b.user_name, 20) << " | "
                  << std::setw(18) << b.start_time << " | "
                  << std::setw(18) << b.end_time << " | "
                  << status_color << std::setw(9) << b.status << Color::RESET << " | "
                  << std::setw(30) << truncateString(b.purpose, 30) << " |\n";
    }
    std::cout << Color::GRAY << "+----+--------------------------------+----------------------+--------------------+--------------------+-----------+--------------------------------+" << Color::RESET << "\n";
}

void UI::displayMaintenanceTable(const std::vector<MaintenanceLog>& logs) {
    if (logs.empty()) {
        printWarning("No maintenance records found.");
        return;
    }

    std::cout << Color::GRAY << "+----+---------------------------+-------------------+--------------------------------+-------------+------------+--------------------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(2)  << "ID" << " | "
              << std::setw(25) << "Equipment" << " | "
              << std::setw(17) << "Technician" << " | "
              << std::setw(30) << "Reported Issue" << " | "
              << std::setw(11) << "Status" << " | "
              << std::setw(10) << "Cost ($)" << " | "
              << std::setw(30) << "Resolution Notes" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+----+---------------------------+-------------------+--------------------------------+-------------+------------+--------------------------------+" << Color::RESET << "\n";

    for (const auto& l : logs) {
        std::string status_color = Color::RED;
        if (l.status == "in_progress") status_color = Color::YELLOW;
        else if (l.status == "resolved") status_color = Color::GREEN;

        std::stringstream cost_ss;
        cost_ss << std::fixed << std::setprecision(2) << l.cost;

        std::cout << "| " << std::left
                  << std::setw(2)  << l.log_id << " | "
                  << std::setw(25) << truncateString(l.resource_name, 25) << " | "
                  << std::setw(17) << truncateString(l.technician_name, 17) << " | "
                  << std::setw(30) << truncateString(l.issue_description, 30) << " | "
                  << status_color << std::setw(11) << l.status << Color::RESET << " | "
                  << std::setw(10) << cost_ss.str() << " | "
                  << std::setw(30) << truncateString(l.resolution_notes.empty() ? "-" : l.resolution_notes, 30) << " |\n";
    }
    std::cout << Color::GRAY << "+----+---------------------------+-------------------+--------------------------------+-------------+------------+--------------------------------+" << Color::RESET << "\n";
}

void UI::displayDepartmentsTable(const std::vector<Department>& depts) {
    std::cout << Color::GRAY << "+----+------------------------------------+------------------------------+---------------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(2)  << "ID" << " | "
              << std::setw(34) << "Department Name" << " | "
              << std::setw(28) << "Campus Building" << " | "
              << std::setw(25) << "Contact Email" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+----+------------------------------------+------------------------------+---------------------------+" << Color::RESET << "\n";
    for (const auto& d : depts) {
        std::cout << "| " << std::left
                  << std::setw(2)  << d.department_id << " | "
                  << std::setw(34) << truncateString(d.dept_name, 34) << " | "
                  << std::setw(28) << truncateString(d.building, 28) << " | "
                  << std::setw(25) << truncateString(d.contact_email, 25) << " |\n";
    }
    std::cout << Color::GRAY << "+----+------------------------------------+------------------------------+---------------------------+" << Color::RESET << "\n";
}

void UI::displayCategoriesTable(const std::vector<Category>& categories) {
    std::cout << Color::GRAY << "+----+---------------------------+---------------------------------------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(2)  << "ID" << " | "
              << std::setw(25) << "Category Name" << " | "
              << std::setw(49) << "Description" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+----+---------------------------+---------------------------------------------------+" << Color::RESET << "\n";
    for (const auto& c : categories) {
        std::cout << "| " << std::left
                  << std::setw(2)  << c.category_id << " | "
                  << std::setw(25) << truncateString(c.category_name, 25) << " | "
                  << std::setw(49) << truncateString(c.description, 49) << " |\n";
    }
    std::cout << Color::GRAY << "+----+---------------------------+---------------------------------------------------+" << Color::RESET << "\n";
}

void UI::displayLocationsTable(const std::vector<Location>& locations) {
    std::cout << Color::GRAY << "+----+------------------------------------+------------------------------+----------------------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(2)  << "ID" << " | "
              << std::setw(34) << "Room / Laboratory" << " | "
              << std::setw(28) << "Building" << " | "
              << std::setw(32) << "Affiliated Department" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+----+------------------------------------+------------------------------+----------------------------------+" << Color::RESET << "\n";
    for (const auto& l : locations) {
        std::cout << "| " << std::left
                  << std::setw(2)  << l.location_id << " | "
                  << std::setw(34) << truncateString(l.room_number, 34) << " | "
                  << std::setw(28) << truncateString(l.building, 28) << " | "
                  << std::setw(32) << truncateString(l.dept_name, 32) << " |\n";
    }
    std::cout << Color::GRAY << "+----+------------------------------------+------------------------------+----------------------------------+" << Color::RESET << "\n";
}

void UI::displayUsersTable(const std::vector<User>& users) {
    std::cout << Color::GRAY << "+----+----------------------+--------------------------------+--------------+----------------------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(2)  << "ID" << " | "
              << std::setw(20) << "Full Name" << " | "
              << std::setw(30) << "Email" << " | "
              << std::setw(12) << "Role" << " | "
              << std::setw(32) << "Department" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+----+----------------------+--------------------------------+--------------+----------------------------------+" << Color::RESET << "\n";
    for (const auto& u : users) {
        std::cout << "| " << std::left
                  << std::setw(2)  << u.user_id << " | "
                  << std::setw(20) << truncateString(u.full_name, 20) << " | "
                  << std::setw(30) << truncateString(u.email, 30) << " | "
                  << std::setw(12) << u.role << " | "
                  << std::setw(32) << truncateString(u.dept_name, 32) << " |\n";
    }
    std::cout << Color::GRAY << "+----+----------------------+--------------------------------+--------------+----------------------------------+" << Color::RESET << "\n";
}

void UI::displayDepartmentSummary(const std::vector<DepartmentSummary>& summaries) {
    std::cout << Color::GRAY << "+------------------------------------+-------+-----------+--------+-------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << "| " << std::left 
              << std::setw(34) << "Department Name" << " | "
              << std::setw(5)  << "Total" << " | "
              << std::setw(9)  << "Available" << " | "
              << std::setw(6)  << "Booked" << " | "
              << std::setw(11) << "Maintenance" << " |" << Color::RESET << "\n";
    std::cout << Color::GRAY << "+------------------------------------+-------+-----------+--------+-------------+" << Color::RESET << "\n";
    for (const auto& ds : summaries) {
        std::cout << "| " << std::left
                  << std::setw(34) << truncateString(ds.dept_name, 34) << " | "
                  << std::setw(5)  << ds.total_resources << " | "
                  << Color::GREEN << std::setw(9) << ds.available_resources << Color::RESET << " | "
                  << Color::YELLOW << std::setw(6) << ds.booked_resources << Color::RESET << " | "
                  << Color::RED << std::setw(11) << ds.maintenance_resources << Color::RESET << " |\n";
    }
    std::cout << Color::GRAY << "+------------------------------------+-------+-----------+--------+-------------+" << Color::RESET << "\n";
}

void UI::displayCostSummary(const CostSummary& summary) {
    std::cout << "\n" << Color::BOLD << Color::MAGENTA << ">> Maintenance & Expense Overview:" << Color::RESET << "\n";
    std::cout << "  - Total Maintenance Expenditure : " << Color::BOLD << Color::GREEN << "$" << std::fixed << std::setprecision(2) << summary.total_cost << Color::RESET << "\n";
    std::cout << "  - Total Service Tickets Logged  : " << Color::BOLD << summary.total_tickets << Color::RESET << "\n";
    std::cout << "  - Currently Open / In-Progress  : " << Color::BOLD << Color::RED << summary.open_tickets << Color::RESET << "\n";
    std::cout << "  - Successfully Resolved Tickets : " << Color::BOLD << Color::GREEN << summary.resolved_tickets << Color::RESET << "\n";
}

std::string UI::readLine(const std::string& prompt) {
    std::cout << Color::BOLD << Color::CYAN << prompt << Color::RESET;
    std::string line;
    std::getline(std::cin, line);
    // Trim leading/trailing whitespace
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, (last - first + 1));
}

int UI::readInt(const std::string& prompt, int min_val, int max_val) {
    while (true) {
        std::cout << Color::BOLD << Color::CYAN << prompt << Color::RESET;
        std::string input;
        std::getline(std::cin, input);
        try {
            int val = std::stoi(input);
            if (val >= min_val && val <= max_val) {
                return val;
            }
            printError("Value must be between " + std::to_string(min_val) + " and " + std::to_string(max_val));
        } catch (...) {
            printError("Invalid integer input. Please try again.");
        }
    }
}

double UI::readDouble(const std::string& prompt, double min_val) {
    while (true) {
        std::cout << Color::BOLD << Color::CYAN << prompt << Color::RESET;
        std::string input;
        std::getline(std::cin, input);
        try {
            double val = std::stod(input);
            if (val >= min_val) {
                return val;
            }
            printError("Value must be >= " + std::to_string(min_val));
        } catch (...) {
            printError("Invalid numeric input. Please try again.");
        }
    }
}

void UI::pressEnterToContinue() {
    std::cout << "\n" << Color::GRAY << "Press [Enter] to continue..." << Color::RESET;
    std::string dummy;
    std::getline(std::cin, dummy);
}

// ----------------------------------------------------------------------------
// Resource Management Menu
// ----------------------------------------------------------------------------
void UI::handleResourceMenu() {
    while (true) {
        printHeader("Resource & Equipment Management");
        std::cout << "1. View All Equipment Inventory\n";
        std::cout << "2. Filter Equipment by Category / Status\n";
        std::cout << "3. Register New Equipment (CRUD: Create)\n";
        std::cout << "4. Update Equipment Details (CRUD: Update)\n";
        std::cout << "5. Decommission / Delete Equipment (CRUD: Delete)\n";
        std::cout << "0. Back to Main Menu\n\n";

        int choice = readInt("Select an option [0-5]: ", 0, 5);
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                auto list = ResourceService::getAllResources();
                std::cout << "\n";
                displayResourcesTable(list);
                pressEnterToContinue();
                break;
            }
            case 2: {
                std::cout << "\n" << Color::BOLD << "Filter Options:" << Color::RESET << "\n";
                std::cout << "Categories:\n";
                auto cats = ResourceService::getAllCategories();
                for (const auto& c : cats) {
                    std::cout << "  " << c.category_id << ". " << c.category_name << "\n";
                }
                int cat_choice = readInt("Select Category ID (or 0 for All): ", 0, 999);
                std::optional<int> cat_opt = (cat_choice == 0) ? std::nullopt : std::make_optional(cat_choice);

                std::cout << "Status options: all, available, booked, under_maintenance, decommissioned\n";
                std::string status = readLine("Enter status filter (or press Enter for all): ");
                if (status.empty()) status = "all";

                auto filtered = ResourceService::filterResources(cat_opt, std::nullopt, status);
                std::cout << "\n";
                displayResourcesTable(filtered);
                pressEnterToContinue();
                break;
            }
            case 3: {
                printHeader("Register New Equipment");
                std::string tag = readLine("Enter unique Asset Tag (e.g. EQ-CSE-003): ");
                if (tag.empty()) {
                    printError("Asset Tag cannot be empty.");
                    pressEnterToContinue();
                    break;
                }
                std::string name = readLine("Enter Equipment Name: ");
                
                std::cout << "\nAvailable Categories:\n";
                auto cats = ResourceService::getAllCategories();
                displayCategoriesTable(cats);
                int cat_id = readInt("Enter Category ID: ", 1, 999);

                std::cout << "\nAvailable Locations/Labs:\n";
                auto locs = ResourceService::getAllLocations();
                displayLocationsTable(locs);
                int loc_id = readInt("Enter Location ID: ", 1, 999);

                std::string p_date = readLine("Enter Purchase Date (YYYY-MM-DD): ");
                if (p_date.empty()) p_date = "2026-01-01";

                std::string err;
                if (ResourceService::addResource(tag, name, cat_id, loc_id, p_date, err)) {
                    printSuccess("Equipment '" + name + "' [" + tag + "] registered successfully!");
                } else {
                    printError("Failed to register equipment: " + err);
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                printHeader("Update Equipment");
                auto list = ResourceService::getAllResources();
                displayResourcesTable(list);
                int res_id = readInt("Enter Resource ID to update: ", 1, 99999);
                auto resOpt = ResourceService::getResourceById(res_id);
                if (!resOpt.has_value()) {
                    printError("Resource not found.");
                    pressEnterToContinue();
                    break;
                }

                std::cout << "Updating resource: " << resOpt->resource_name << " (" << resOpt->asset_tag << ")\n";
                std::string new_name = readLine("New Name [" + resOpt->resource_name + "]: ");
                if (new_name.empty()) new_name = resOpt->resource_name;

                int new_cat = readInt("New Category ID [" + std::to_string(resOpt->category_id) + "]: ", 1, 999);
                int new_loc = readInt("New Location ID [" + std::to_string(resOpt->location_id) + "]: ", 1, 999);

                std::cout << "Status options: available, booked, under_maintenance, decommissioned\n";
                std::string new_status = readLine("New Status [" + resOpt->status + "]: ");
                if (new_status.empty()) new_status = resOpt->status;

                std::string err;
                if (ResourceService::updateResource(res_id, new_name, new_cat, new_loc, new_status, err)) {
                    printSuccess("Equipment details updated successfully!");
                } else {
                    printError("Update failed: " + err);
                }
                pressEnterToContinue();
                break;
            }
            case 5: {
                printHeader("Decommission / Delete Equipment");
                auto list = ResourceService::getAllResources();
                displayResourcesTable(list);
                int res_id = readInt("Enter Resource ID to delete: ", 1, 99999);
                std::string confirm = readLine("Are you sure you want to permanently delete resource ID " + std::to_string(res_id) + "? (yes/no): ");
                if (confirm == "yes") {
                    std::string err;
                    if (ResourceService::deleteResource(res_id, err)) {
                        printSuccess("Resource deleted successfully.");
                    } else {
                        printError("Cannot delete resource: " + err + "\n(Note: If resource has bookings/maintenance history, foreign key constraints prevent orphan records. You can update status to 'decommissioned' instead.)");
                    }
                } else {
                    printInfo("Deletion cancelled.");
                }
                pressEnterToContinue();
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Booking Management Menu
// ----------------------------------------------------------------------------
void UI::handleBookingMenu() {
    while (true) {
        printHeader("Equipment Booking & Scheduling");
        std::cout << "1. View Active Bookings\n";
        std::cout << "2. View All Booking History\n";
        std::cout << "3. Create New Equipment Booking (ACID Transaction)\n";
        std::cout << "4. Mark Booking Completed / Return Equipment\n";
        std::cout << "5. Cancel Active Booking\n";
        std::cout << "0. Back to Main Menu\n\n";

        int choice = readInt("Select an option [0-5]: ", 0, 5);
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                auto list = BookingService::getActiveBookings();
                std::cout << "\n";
                displayBookingsTable(list);
                pressEnterToContinue();
                break;
            }
            case 2: {
                auto list = BookingService::getAllBookings();
                std::cout << "\n";
                displayBookingsTable(list);
                pressEnterToContinue();
                break;
            }
            case 3: {
                printHeader("Create New Equipment Booking");
                std::cout << "\nAvailable Equipment Inventory:\n";
                auto resList = ResourceService::filterResources(std::nullopt, std::nullopt, "available");
                displayResourcesTable(resList);

                int res_id = readInt("Enter Resource ID to book: ", 1, 99999);

                std::cout << "\nRegistered Users:\n";
                auto users = BookingService::getAllUsers();
                displayUsersTable(users);
                int user_id = readInt("Enter User ID: ", 1, 99999);

                std::cout << "\n" << Color::GRAY << "Enter schedule timestamps in 'YYYY-MM-DD HH:MM' format." << Color::RESET << "\n";
                std::string start_time = readLine("Enter Start Time (e.g. 2026-08-28 10:00): ");
                std::string end_time   = readLine("Enter End Time   (e.g. 2026-08-28 14:00): ");
                std::string purpose    = readLine("Enter Booking Purpose: ");

                std::string err;
                if (BookingService::createBooking(res_id, user_id, start_time, end_time, purpose, err)) {
                    printSuccess("Booking confirmed! Resource status updated to 'booked'.");
                } else {
                    printError("Booking rejected: " + err);
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                printHeader("Complete Booking");
                auto list = BookingService::getActiveBookings();
                displayBookingsTable(list);
                if (list.empty()) {
                    pressEnterToContinue();
                    break;
                }
                int b_id = readInt("Enter Booking ID to complete: ", 1, 99999);
                std::string err;
                if (BookingService::completeBooking(b_id, err)) {
                    printSuccess("Booking #" + std::to_string(b_id) + " completed. Resource returned to available inventory.");
                } else {
                    printError("Operation failed: " + err);
                }
                pressEnterToContinue();
                break;
            }
            case 5: {
                printHeader("Cancel Booking");
                auto list = BookingService::getActiveBookings();
                displayBookingsTable(list);
                if (list.empty()) {
                    pressEnterToContinue();
                    break;
                }
                int b_id = readInt("Enter Booking ID to cancel: ", 1, 99999);
                std::string err;
                if (BookingService::cancelBooking(b_id, err)) {
                    printSuccess("Booking #" + std::to_string(b_id) + " cancelled. Equipment freed.");
                } else {
                    printError("Operation failed: " + err);
                }
                pressEnterToContinue();
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Maintenance Management Menu
// ----------------------------------------------------------------------------
void UI::handleMaintenanceMenu() {
    while (true) {
        printHeader("Maintenance Logs & Service Repairs");
        std::cout << "1. View Open / In-Progress Maintenance Tickets\n";
        std::cout << "2. View All Maintenance History & Costs\n";
        std::cout << "3. Report Issue / Open Service Ticket (Lock Equipment)\n";
        std::cout << "4. Update Ticket Status / Resolve Issue (Unlock Equipment)\n";
        std::cout << "0. Back to Main Menu\n\n";

        int choice = readInt("Select an option [0-4]: ", 0, 4);
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                auto list = MaintenanceService::getOpenLogs();
                std::cout << "\n";
                displayMaintenanceTable(list);
                pressEnterToContinue();
                break;
            }
            case 2: {
                auto list = MaintenanceService::getAllLogs();
                std::cout << "\n";
                displayMaintenanceTable(list);
                auto costSummary = MaintenanceService::getCostSummary();
                displayCostSummary(costSummary);
                pressEnterToContinue();
                break;
            }
            case 3: {
                printHeader("Report Equipment Fault / Start Maintenance");
                auto resList = ResourceService::getAllResources();
                displayResourcesTable(resList);
                int res_id = readInt("Enter Resource ID requiring service: ", 1, 99999);

                std::cout << "\nAuthorized Technicians:\n";
                auto techs = MaintenanceService::getTechnicians();
                displayUsersTable(techs);
                int tech_id = readInt("Enter Assigned Technician User ID: ", 1, 99999);

                std::string issue = readLine("Enter Issue Description: ");
                double est_cost = readDouble("Enter Estimated Repair Cost ($): ", 0.0);

                std::string err;
                if (MaintenanceService::logMaintenanceTicket(res_id, tech_id, issue, est_cost, err)) {
                    printSuccess("Maintenance ticket logged! Equipment status locked to 'under_maintenance'.");
                } else {
                    printError("Failed to log maintenance ticket: " + err);
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                printHeader("Update Service Ticket");
                auto list = MaintenanceService::getOpenLogs();
                displayMaintenanceTable(list);
                if (list.empty()) {
                    pressEnterToContinue();
                    break;
                }

                int log_id = readInt("Enter Ticket ID to update: ", 1, 99999);
                auto logOpt = MaintenanceService::getLogById(log_id);
                if (!logOpt.has_value()) {
                    printError("Ticket not found.");
                    pressEnterToContinue();
                    break;
                }

                std::cout << "\nStatus options: in_progress, resolved\n";
                std::string new_status = readLine("Enter new status (in_progress / resolved): ");
                if (new_status != "in_progress" && new_status != "resolved") {
                    printError("Status must be 'in_progress' or 'resolved'.");
                    pressEnterToContinue();
                    break;
                }

                std::string notes = readLine("Enter Resolution Notes / Action Taken: ");
                double actual_cost = readDouble("Enter Final Repair Cost ($) [" + std::to_string(logOpt->cost) + "]: ", 0.0);

                std::string err;
                if (MaintenanceService::updateMaintenanceStatus(log_id, new_status, notes, actual_cost, err)) {
                    if (new_status == "resolved") {
                        printSuccess("Ticket #" + std::to_string(log_id) + " RESOLVED! Equipment restored to 'available' status.");
                    } else {
                        printSuccess("Ticket #" + std::to_string(log_id) + " updated to in-progress.");
                    }
                } else {
                    printError("Failed to update ticket: " + err);
                }
                pressEnterToContinue();
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Master Data Management (Departments, Categories, Locations, Users)
// ----------------------------------------------------------------------------
void UI::handleAdminMenu() {
    while (true) {
        printHeader("Master Data & Administrative Setup");
        std::cout << "1. View Academic Departments\n";
        std::cout << "2. View & Add Equipment Categories\n";
        std::cout << "3. View & Add Laboratories / Locations\n";
        std::cout << "4. View & Register System Users\n";
        std::cout << "0. Back to Main Menu\n\n";

        int choice = readInt("Select an option [0-4]: ", 0, 4);
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                auto depts = ResourceService::getAllDepartments();
                std::cout << "\n";
                displayDepartmentsTable(depts);
                pressEnterToContinue();
                break;
            }
            case 2: {
                auto cats = ResourceService::getAllCategories();
                std::cout << "\n";
                displayCategoriesTable(cats);

                std::string opt = readLine("Do you want to add a new category? (yes/no): ");
                if (opt == "yes") {
                    std::string cat_name = readLine("Enter Category Name: ");
                    std::string cat_desc = readLine("Enter Description: ");
                    std::string err;
                    if (ResourceService::addCategory(cat_name, cat_desc, err)) {
                        printSuccess("Category '" + cat_name + "' added!");
                    } else {
                        printError("Failed: " + err);
                    }
                }
                pressEnterToContinue();
                break;
            }
            case 3: {
                auto locs = ResourceService::getAllLocations();
                std::cout << "\n";
                displayLocationsTable(locs);

                std::string opt = readLine("Do you want to add a new lab/location? (yes/no): ");
                if (opt == "yes") {
                    std::string room = readLine("Enter Room/Lab Name (e.g. Lab-402 Network Lab): ");
                    std::string bldg = readLine("Enter Building Name: ");
                    auto depts = ResourceService::getAllDepartments();
                    displayDepartmentsTable(depts);
                    int dept_id = readInt("Enter Department ID: ", 1, 999);
                    std::string err;
                    if (ResourceService::addLocation(room, bldg, dept_id, err)) {
                        printSuccess("Location added!");
                    } else {
                        printError("Failed: " + err);
                    }
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                auto users = BookingService::getAllUsers();
                std::cout << "\n";
                displayUsersTable(users);

                std::string opt = readLine("Do you want to register a new user? (yes/no): ");
                if (opt == "yes") {
                    std::string full_name = readLine("Enter Full Name: ");
                    std::string email = readLine("Enter Email: ");
                    std::cout << "Roles: student, faculty, admin, technician\n";
                    std::string role = readLine("Enter Role: ");
                    auto depts = ResourceService::getAllDepartments();
                    displayDepartmentsTable(depts);
                    int dept_id = readInt("Enter Department ID: ", 1, 999);
                    std::string err;
                    if (BookingService::addUser(full_name, email, role, dept_id, err)) {
                        printSuccess("User registered successfully!");
                    } else {
                        printError("Failed: " + err);
                    }
                }
                pressEnterToContinue();
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Analytics & Reporting Menu
// ----------------------------------------------------------------------------
void UI::handleAnalyticsMenu() {
    printHeader("DBMS Aggregation & Resource Analytics");
    std::cout << Color::BOLD << Color::CYAN << "1. Department-wise Equipment Distribution & Status Breakdown (SQL Aggregations)\n" << Color::RESET;
    auto deptSummary = ResourceService::getDepartmentSummary();
    displayDepartmentSummary(deptSummary);

    std::cout << "\n" << Color::BOLD << Color::CYAN << "2. Maintenance Financial & Operational Health\n" << Color::RESET;
    auto costSummary = MaintenanceService::getCostSummary();
    displayCostSummary(costSummary);

    pressEnterToContinue();
}

// ----------------------------------------------------------------------------
// Security & Parameterized Query Demonstration
// ----------------------------------------------------------------------------
void UI::handleSecurityDemo() {
    printHeader("SQL Injection Defense: Parameterized Query Verification");
    std::cout << "This demo shows why Parameterized Queries (sqlite3_bind_*) provide 100% immunity\n"
              << "against SQL Injection compared to vulnerable string concatenation.\n\n";

    std::string malicious_payload = "EQ-CSE-001' OR '1'='1";
    std::cout << Color::BOLD << "Payload: " << Color::RED << malicious_payload << Color::RESET << "\n\n";

    std::cout << Color::YELLOW << "Scenario A: Vulnerable Query String Concatenation" << Color::RESET << "\n";
    std::cout << "  Query: \"SELECT * FROM resources WHERE asset_tag = '\" + input + \"';\"\n";
    std::cout << "  Executed SQL: SELECT * FROM resources WHERE asset_tag = '" << malicious_payload << "';\n";
    std::cout << "  -> Result: Condition evaluates to TRUE for ALL rows. Attacker bypasses filters!\n\n";

    std::cout << Color::GREEN << "Scenario B: Parameterized Prepared Statement (Used in this project)" << Color::RESET << "\n";
    std::cout << "  Query: \"SELECT * FROM resources WHERE asset_tag = ?;\"\n";
    std::cout << "  sqlite3_bind_text(stmt, 1, input);\n";
    std::cout << "  -> Result: Database engine treats input purely as literal text data,\n"
              << "     NOT as executable SQL instructions. Search looks strictly for an asset\n"
              << "     whose literal tag is \"" << malicious_payload << "\".\n\n";

    std::cout << "Executing live parameterized lookup with payload...\n";
    auto resOpt = ResourceService::getResourceByTag(malicious_payload);
    if (!resOpt.has_value()) {
        printSuccess("DEFENSE VERIFIED: No record found with literal tag \"" + malicious_payload + "\".");
        printInfo("Zero data leakage. The database remained secure!");
    } else {
        printWarning("Record matched literal tag.");
    }

    pressEnterToContinue();
}

// ----------------------------------------------------------------------------
// Main Orchestration Loop
// ----------------------------------------------------------------------------
void UI::runMainMenu() {
    while (true) {
        printHeader("COLLEGE RESOURCE TRACKER (DBMS MINI-PROJECT)");
        std::cout << Color::BOLD << Color::YELLOW << "3NF Normalized Relational Database Management System | C++ Interface" << Color::RESET << "\n\n";
        std::cout << "  1. Equipment & Inventory Management (CRUD)\n";
        std::cout << "  2. Resource Booking & Reservation System (ACID Transactions)\n";
        std::cout << "  3. Maintenance Logs & Equipment Health (Service Tracking)\n";
        std::cout << "  4. Master Data (Departments, Categories, Labs, Users)\n";
        std::cout << "  5. Reports & DBMS Aggregation Analytics\n";
        std::cout << "  6. Security Demo: Parameterized Query Verification\n";
        std::cout << "  0. Exit Application\n\n";

        int choice = readInt("Select an option [0-6]: ", 0, 6);
        if (choice == 0) {
            std::cout << "\n" << Color::BOLD << Color::GREEN << "Thank you for using College Resource Tracker. Goodbye!" << Color::RESET << "\n\n";
            break;
        }

        switch (choice) {
            case 1: handleResourceMenu(); break;
            case 2: handleBookingMenu(); break;
            case 3: handleMaintenanceMenu(); break;
            case 4: handleAdminMenu(); break;
            case 5: handleAnalyticsMenu(); break;
            case 6: handleSecurityDemo(); break;
        }
    }
}

} // namespace tracker
