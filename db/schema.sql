-- ============================================================================
-- College Resource Tracker: 3NF Relational Database Schema
-- DBMS Mini-Project
-- ============================================================================

PRAGMA foreign_keys = ON;

-- ----------------------------------------------------------------------------
-- 1. Departments Table
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS departments (
    department_id INTEGER PRIMARY KEY AUTOINCREMENT,
    dept_name     TEXT NOT NULL UNIQUE,
    building      TEXT NOT NULL,
    contact_email TEXT NOT NULL
);

-- ----------------------------------------------------------------------------
-- 2. Locations / Laboratories Table
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS locations (
    location_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    room_number   TEXT NOT NULL,
    building      TEXT NOT NULL,
    department_id INTEGER NOT NULL,
    FOREIGN KEY (department_id) REFERENCES departments (department_id) ON DELETE RESTRICT,
    UNIQUE (room_number, building)
);

-- ----------------------------------------------------------------------------
-- 3. Resource Categories Table
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS categories (
    category_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    category_name TEXT NOT NULL UNIQUE,
    description   TEXT
);

-- ----------------------------------------------------------------------------
-- 4. Users Table (Students, Faculty, Admins, Technicians)
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS users (
    user_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name     TEXT NOT NULL,
    email         TEXT NOT NULL UNIQUE,
    role          TEXT NOT NULL CHECK (role IN ('student', 'faculty', 'admin', 'technician')),
    department_id INTEGER NOT NULL,
    FOREIGN KEY (department_id) REFERENCES departments (department_id) ON DELETE RESTRICT
);

-- ----------------------------------------------------------------------------
-- 5. Resources / Equipment Table
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS resources (
    resource_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    asset_tag     TEXT NOT NULL UNIQUE,
    resource_name TEXT NOT NULL,
    category_id   INTEGER NOT NULL,
    location_id   INTEGER NOT NULL,
    status        TEXT NOT NULL DEFAULT 'available' CHECK (status IN ('available', 'booked', 'under_maintenance', 'decommissioned')),
    purchase_date TEXT NOT NULL,
    FOREIGN KEY (category_id) REFERENCES categories (category_id) ON DELETE RESTRICT,
    FOREIGN KEY (location_id) REFERENCES locations (location_id) ON DELETE RESTRICT
);

-- ----------------------------------------------------------------------------
-- 6. Equipment Bookings Table
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS bookings (
    booking_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    resource_id   INTEGER NOT NULL,
    user_id       INTEGER NOT NULL,
    start_time    TEXT NOT NULL, -- Format: YYYY-MM-DD HH:MM
    end_time      TEXT NOT NULL, -- Format: YYYY-MM-DD HH:MM
    purpose       TEXT NOT NULL,
    status        TEXT NOT NULL DEFAULT 'active' CHECK (status IN ('active', 'completed', 'cancelled')),
    created_at    TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
    FOREIGN KEY (resource_id) REFERENCES resources (resource_id) ON DELETE RESTRICT,
    FOREIGN KEY (user_id) REFERENCES users (user_id) ON DELETE RESTRICT
);

-- ----------------------------------------------------------------------------
-- 7. Maintenance Logs Table
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS maintenance_logs (
    log_id            INTEGER PRIMARY KEY AUTOINCREMENT,
    resource_id       INTEGER NOT NULL,
    technician_id     INTEGER NOT NULL,
    issue_description TEXT NOT NULL,
    resolution_notes  TEXT,
    cost              REAL DEFAULT 0.0 CHECK (cost >= 0.0),
    start_date        TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
    end_date          TEXT,
    status            TEXT NOT NULL DEFAULT 'open' CHECK (status IN ('open', 'in_progress', 'resolved')),
    FOREIGN KEY (resource_id) REFERENCES resources (resource_id) ON DELETE RESTRICT,
    FOREIGN KEY (technician_id) REFERENCES users (user_id) ON DELETE RESTRICT
);

-- ----------------------------------------------------------------------------
-- Database Indexes for Query Optimization & Foreign Key Lookups
-- ----------------------------------------------------------------------------
CREATE INDEX IF NOT EXISTS idx_locations_dept ON locations (department_id);
CREATE INDEX IF NOT EXISTS idx_users_dept ON users (department_id);
CREATE INDEX IF NOT EXISTS idx_resources_category ON resources (category_id);
CREATE INDEX IF NOT EXISTS idx_resources_location ON resources (location_id);
CREATE INDEX IF NOT EXISTS idx_resources_status ON resources (status);
CREATE INDEX IF NOT EXISTS idx_bookings_resource ON bookings (resource_id);
CREATE INDEX IF NOT EXISTS idx_bookings_user ON bookings (user_id);
CREATE INDEX IF NOT EXISTS idx_bookings_times ON bookings (start_time, end_time);
CREATE INDEX IF NOT EXISTS idx_maintenance_resource ON maintenance_logs (resource_id);
CREATE INDEX IF NOT EXISTS idx_maintenance_status ON maintenance_logs (status);
