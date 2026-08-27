#pragma once

#include "models.hpp"
#include <vector>
#include <optional>
#include <string>

namespace tracker {

class BookingService {
public:
    // Core Booking Lifecycle
    static bool createBooking(int resource_id, int user_id,
                             const std::string& start_time, const std::string& end_time,
                             const std::string& purpose, std::string& err_msg);

    static bool completeBooking(int booking_id, std::string& err_msg);
    static bool cancelBooking(int booking_id, std::string& err_msg);

    // Queries
    static std::vector<Booking> getAllBookings();
    static std::vector<Booking> getActiveBookings();
    static std::vector<Booking> getBookingsByUser(int user_id);
    static std::vector<Booking> getBookingsByResource(int resource_id);
    static std::optional<Booking> getBookingById(int booking_id);

    // Conflict Checking Logic
    static bool hasScheduleConflict(int resource_id, const std::string& start_time, const std::string& end_time, int exclude_booking_id = 0);

    // User Management Helpers
    static std::vector<User> getAllUsers();
    static std::optional<User> getUserById(int user_id);
    static bool addUser(const std::string& full_name, const std::string& email,
                        const std::string& role, int department_id, std::string& err_msg);
};

} // namespace tracker
