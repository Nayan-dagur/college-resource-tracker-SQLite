-- ============================================================================
-- College Resource Tracker: Seed Data
-- Sample dataset for testing and demonstration
-- ============================================================================

-- Departments
INSERT OR IGNORE INTO departments (department_id, dept_name, building, contact_email) VALUES
(1, 'Computer Science & Engineering', 'Alan Turing Block', 'cse@college.edu'),
(2, 'Electronics & Communication', 'Shannon Hall', 'ece@college.edu'),
(3, 'Mechanical Engineering', 'Watt Pavilion', 'mech@college.edu'),
(4, 'Central Research & Computing', 'Main Administrative Complex', 'computing@college.edu');

-- Locations / Labs
INSERT OR IGNORE INTO locations (location_id, room_number, building, department_id) VALUES
(1, 'Lab-301 (AI & Robotics Lab)', 'Alan Turing Block', 1),
(2, 'Lab-105 (VLSI & Embedded Lab)', 'Shannon Hall', 2),
(3, 'Lab-204 (CAD/CAM Studio)', 'Watt Pavilion', 3),
(4, 'Server Room A', 'Main Administrative Complex', 4),
(5, 'Seminar Hall B', 'Alan Turing Block', 1);

-- Categories
INSERT OR IGNORE INTO categories (category_id, category_name, description) VALUES
(1, 'High-End Workstations', 'GPU-accelerated desktop towers for Deep Learning & 3D rendering'),
(2, 'Laboratory Instruments', 'Oscilloscopes, signal generators, logic analyzers, multimeters'),
(3, 'Audio/Visual Equipment', '4K Laser Projectors, motorized projection screens, wireless mic systems'),
(4, 'Rapid Prototyping Tools', 'Industrial 3D Printers, laser cutters, CNC milling machines'),
(5, 'Mobile Testing Devices', 'Evaluation laptops, tablets, sensor development boards');

-- Users
INSERT OR IGNORE INTO users (user_id, full_name, email, role, department_id) VALUES
(1, 'Dr. Aris Thorne', 'aris.thorne@college.edu', 'faculty', 1),
(2, 'Prof. Elena Vance', 'elena.vance@college.edu', 'faculty', 2),
(3, 'Nayan Dagur', 'nayan.dagur@college.edu', 'student', 1),
(4, 'Rohan Sharma', 'rohan.sharma@college.edu', 'student', 2),
(5, 'Marcus Brody', 'marcus.brody@college.edu', 'technician', 4),
(6, 'Admin Office', 'admin.resources@college.edu', 'admin', 4);

-- Resources / Equipment
INSERT OR IGNORE INTO resources (resource_id, asset_tag, resource_name, category_id, location_id, status, purchase_date) VALUES
(1, 'EQ-CSE-001', 'NVIDIA RTX 4090 AI Workstation #1', 1, 1, 'available', '2024-01-15'),
(2, 'EQ-CSE-002', 'NVIDIA RTX 4090 AI Workstation #2', 1, 1, 'booked', '2024-01-15'),
(3, 'EQ-ECE-101', 'Keysight 4-Channel Digital Oscilloscope', 2, 2, 'available', '2023-08-10'),
(4, 'EQ-ECE-102', 'Rohde & Schwarz Spectrum Analyzer', 2, 2, 'under_maintenance', '2023-09-01'),
(5, 'EQ-MEC-201', 'Ultimaker S5 Dual Extruder 3D Printer', 4, 3, 'available', '2023-11-20'),
(6, 'EQ-AV-301', 'Epson EB-PU2220B 20K Lumen Laser Projector', 3, 5, 'available', '2024-02-01'),
(7, 'EQ-MOB-401', 'Dell Precision 7780 Mobile Workstation', 5, 1, 'available', '2024-03-10');

-- Bookings
INSERT OR IGNORE INTO bookings (booking_id, resource_id, user_id, start_time, end_time, purpose, status, created_at) VALUES
(1, 2, 3, '2026-08-26 10:00', '2026-08-26 14:00', 'LLM Fine-tuning & Benchmarking for Final Year Project', 'active', '2026-08-25 09:30:00'),
(2, 6, 1, '2026-08-27 14:00', '2026-08-27 17:00', 'Guest Lecture on Distributed Systems in Seminar Hall', 'active', '2026-08-25 11:00:00');

-- Maintenance Logs
INSERT OR IGNORE INTO maintenance_logs (log_id, resource_id, technician_id, issue_description, resolution_notes, cost, start_date, end_date, status) VALUES
(1, 4, 5, 'RF Input channel B calibration error above 3GHz', 'Replaced input attenuator module, awaiting final RF re-calibration test', 150.00, '2026-08-24 14:00', NULL, 'in_progress'),
(2, 5, 5, 'Bowden tube clogged with carbon-fiber PLA filament', 'Nozzle cleared, heated block cleaned, print head re-aligned and tested', 25.00, '2026-08-10 10:00', '2026-08-10 16:30', 'resolved');
