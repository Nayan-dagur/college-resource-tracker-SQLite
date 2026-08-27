#pragma once

#include <string>
#include <vector>
#include <optional>

namespace tracker {

struct Department {
    int department_id{0};
    std::string dept_name;
    std::string building;
    std::string contact_email;
};

struct Location {
    int location_id{0};
    std::string room_number;
    std::string building;
    int department_id{0};
    std::string dept_name;
};

struct Category {
    int category_id{0};
    std::string category_name;
    std::string description;
};

struct User {
    int user_id{0};
    std::string full_name;
    std::string email;
    std::string role; // 'student', 'faculty', 'admin', 'technician'
    int department_id{0};
    std::string dept_name;
};

struct Resource {
    int resource_id{0};
    std::string asset_tag;
    std::string resource_name;
    int category_id{0};
    std::string category_name;
    int location_id{0};
    std::string room_number;
    std::string building;
    std::string status; // 'available', 'booked', 'under_maintenance', 'decommissioned'
    std::string purchase_date;
};

struct Booking {
    int booking_id{0};
    int resource_id{0};
    std::string resource_name;
    std::string asset_tag;
    int user_id{0};
    std::string user_name;
    std::string user_email;
    std::string start_time;
    std::string end_time;
    std::string purpose;
    std::string status; // 'active', 'completed', 'cancelled'
    std::string created_at;
};

struct MaintenanceLog {
    int log_id{0};
    int resource_id{0};
    std::string resource_name;
    std::string asset_tag;
    int technician_id{0};
    std::string technician_name;
    std::string issue_description;
    std::string resolution_notes;
    double cost{0.0};
    std::string start_date;
    std::string end_date;
    std::string status; // 'open', 'in_progress', 'resolved'
};

struct DepartmentSummary {
    std::string dept_name;
    int total_resources{0};
    int available_resources{0};
    int booked_resources{0};
    int maintenance_resources{0};
};

struct CostSummary {
    double total_cost{0.0};
    int total_tickets{0};
    int open_tickets{0};
    int resolved_tickets{0};
};

} // namespace tracker
