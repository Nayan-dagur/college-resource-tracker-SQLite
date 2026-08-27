#pragma once

#include "models.hpp"
#include <string>
#include <vector>

namespace tracker {

class UI {
public:
    // Color and Style Helpers
    static void printHeader(const std::string& title);
    static void printSuccess(const std::string& msg);
    static void printError(const std::string& msg);
    static void printWarning(const std::string& msg);
    static void printInfo(const std::string& msg);

    // Table Display Formatters
    static void displayResourcesTable(const std::vector<Resource>& resources);
    static void displayBookingsTable(const std::vector<Booking>& bookings);
    static void displayMaintenanceTable(const std::vector<MaintenanceLog>& logs);
    static void displayDepartmentsTable(const std::vector<Department>& depts);
    static void displayCategoriesTable(const std::vector<Category>& categories);
    static void displayLocationsTable(const std::vector<Location>& locations);
    static void displayUsersTable(const std::vector<User>& users);
    static void displayDepartmentSummary(const std::vector<DepartmentSummary>& summaries);
    static void displayCostSummary(const CostSummary& summary);

    // Input Helpers
    static std::string readLine(const std::string& prompt);
    static int readInt(const std::string& prompt, int min_val = 0, int max_val = 999999);
    static double readDouble(const std::string& prompt, double min_val = 0.0);
    static void pressEnterToContinue();

    // Menu Screens
    static void runMainMenu();
    static void handleResourceMenu();
    static void handleBookingMenu();
    static void handleMaintenanceMenu();
    static void handleAdminMenu();
    static void handleAnalyticsMenu();
    static void handleSecurityDemo();
};

} // namespace tracker
