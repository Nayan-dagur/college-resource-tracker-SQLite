#pragma once

#include "models.hpp"
#include <vector>
#include <optional>
#include <string>

namespace tracker {

class ResourceService {
public:
    // Core Resource CRUD
    static bool addResource(const std::string& asset_tag, const std::string& name,
                            int category_id, int location_id, const std::string& purchase_date,
                            std::string& err_msg);

    static std::vector<Resource> getAllResources();
    static std::optional<Resource> getResourceById(int resource_id);
    static std::optional<Resource> getResourceByTag(const std::string& asset_tag);
    static std::vector<Resource> filterResources(std::optional<int> category_id,
                                                std::optional<int> location_id,
                                                const std::string& status_filter);

    static bool updateResource(int resource_id, const std::string& name,
                              int category_id, int location_id, const std::string& status,
                              std::string& err_msg);

    static bool updateResourceStatus(int resource_id, const std::string& new_status, std::string& err_msg);

    static bool deleteResource(int resource_id, std::string& err_msg);

    // Reference Table Operations
    static std::vector<Category> getAllCategories();
    static bool addCategory(const std::string& name, const std::string& description, std::string& err_msg);

    static std::vector<Location> getAllLocations();
    static bool addLocation(const std::string& room_number, const std::string& building, int department_id, std::string& err_msg);

    static std::vector<Department> getAllDepartments();

    // Analytics & Summaries
    static std::vector<DepartmentSummary> getDepartmentSummary();
};

} // namespace tracker
