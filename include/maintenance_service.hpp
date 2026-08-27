#pragma once

#include "models.hpp"
#include <vector>
#include <optional>
#include <string>

namespace tracker {

class MaintenanceService {
public:
    // Core Maintenance Operations
    static bool logMaintenanceTicket(int resource_id, int technician_id,
                                    const std::string& issue_description, double estimated_cost,
                                    std::string& err_msg);

    static bool updateMaintenanceStatus(int log_id, const std::string& status,
                                       const std::string& resolution_notes, double actual_cost,
                                       std::string& err_msg);

    // Queries
    static std::vector<MaintenanceLog> getAllLogs();
    static std::vector<MaintenanceLog> getOpenLogs();
    static std::vector<MaintenanceLog> getLogsByResource(int resource_id);
    static std::optional<MaintenanceLog> getLogById(int log_id);

    // Analytics
    static CostSummary getCostSummary();
    static std::vector<User> getTechnicians();
};

} // namespace tracker
