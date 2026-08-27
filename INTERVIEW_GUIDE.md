# College Resource Tracker: Comprehensive Interview & Technical Guide 🎓💡

This document is your complete preparation manual for answering questions about this project in technical interviews, DBMS viva examinations, and software engineering discussions.

---

## 📑 Table of Contents
1. [Core Resume Bullet Points Explained](#1-core-resume-bullet-points-explained)
2. [Database Schema & 3NF Normalization Proof](#2-database-schema--3nf-normalization-proof)
3. [Functional Dependencies (FD) Analysis](#3-functional-dependencies-fd-analysis)
4. [Relational Anomalies Eliminated](#4-relational-anomalies-eliminated)
5. [Deep-Dive: Secure Parameterized Queries vs. SQL Injection](#5-deep-dive-secure-parameterized-queries-vs-sql-injection)
6. [ACID Compliance & Transaction Management](#6-acid-compliance--transaction-management)
7. [C++ Architecture & Design Patterns](#7-c-architecture--design-patterns)
8. [Top 15 Technical Interview Questions & Model Answers](#8-top-15-technical-interview-questions--model-answers)

---

## 1. Core Resume Bullet Points Explained

### Bullet 1: *"Designed a normalized (3NF) relational schema to track college equipment, bookings, and maintenance logs."*
- **What to say in 30 seconds**:
  > *"I designed a 7-table relational schema modeled in Third Normal Form (3NF) to eliminate data redundancy and anomalies in managing university laboratory equipment. The schema enforces strict referential integrity across departments, labs, equipment inventory, time-slotted bookings, and repair logs, while using secondary B-tree indexes for fast lookups."*

### Bullet 2: *"Implemented CRUD operations through a C++ console interface using secure, parameterized queries."*
- **What to say in 30 seconds**:
  > *"I built a modular C++17 console interface interfacing with SQLite3 using native prepared statements (`sqlite3_prepare_v2` and `sqlite3_bind_*`). This completely mitigates SQL injection attacks by pre-compiling the SQL AST prior to binding user input as literal constants. I also implemented RAII wrappers for statement lifecycle management and managed multi-step state changes within ACID transactions."*

---

## 2. Database Schema & 3NF Normalization Proof

### What is Normalization?
Normalization is a systematic database design technique that decomposes relations to minimize data redundancy and prevent Insertion, Update, and Deletion anomalies.

---

### Step-by-Step Normal Forms Breakdown:

#### 1. First Normal Form (1NF)
- **Rules**:
  1. Each column contains only **atomic (indivisible) values**.
  2. No repeating groups or arrays stored in a single cell.
  3. Every table has a primary key uniquely identifying each row.
- **In our project**:
  - All attributes (e.g., `room_number`, `purchase_date`, `status`) store singular atomic values. No comma-separated strings for equipment items or multiple users per cell.

#### 2. Second Normal Form (2NF)
- **Rules**:
  1. The table must be in **1NF**.
  2. **No Partial Functional Dependencies**: No non-prime attribute should be functionally dependent on a proper subset of any candidate key.
  *(Note: If a table has a single-attribute primary key, 2NF is automatically satisfied!)*
- **In our project**:
  - Every table uses a single synthetic primary key (e.g., `resource_id`, `booking_id`, `department_id`), meaning no partial dependencies can exist.
  - In `bookings`, composite dependencies (like `user_name` depending on `user_id`) were intentionally factored out into the `users` table instead of being redundantly stored in `bookings`.

#### 3. Third Normal Form (3NF)
- **Rules**:
  1. The table must be in **2NF**.
  2. **No Transitive Dependencies**: No non-prime attribute is transitively dependent on the primary key ($X \to Y$ and $Y \to Z \implies X \to Z$).
  - Formal Condition: For every non-trivial functional dependency $X \to Y$:
    - Either $X$ is a **Superkey**, OR
    - $Y$ is a **Prime Attribute** (part of a candidate key).
- **In our project**:
  - Suppose we stored `dept_name` and `building` inside `resources`. We would have:
    $$\text{resource\_id} \to \text{location\_id} \to \text{dept\_name}$$
    This is a transitive dependency!
  - **Resolution**: We separated `locations` and `departments` into their own relations. `resources` only stores foreign keys (`category_id`, `location_id`).

---

## 3. Functional Dependencies (FD) Analysis

| Table | Candidate Keys | Functional Dependencies (FD) | Normal Form Status |
| :--- | :--- | :--- | :--- |
| **`departments`** | `department_id`, `dept_name` | `department_id` $\to$ `{dept_name, building, contact_email}`<br>`dept_name` $\to$ `{department_id, building, contact_email}` | **3NF / BCNF** (LHS is superkey) |
| **`locations`** | `location_id`, `{room_number, building}` | `location_id` $\to$ `{room_number, building, department_id}`<br>`{room_number, building}` $\to$ `{location_id, department_id}` | **3NF / BCNF** (LHS is superkey) |
| **`categories`** | `category_id`, `category_name` | `category_id` $\to$ `{category_name, description}`<br>`category_name` $\to$ `{category_id, description}` | **3NF / BCNF** (LHS is superkey) |
| **`users`** | `user_id`, `email` | `user_id` $\to$ `{full_name, email, role, department_id}`<br>`email` $\to$ `{user_id, full_name, role, department_id}` | **3NF / BCNF** (LHS is superkey) |
| **`resources`** | `resource_id`, `asset_tag` | `resource_id` $\to$ `{asset_tag, resource_name, category_id, location_id, status, purchase_date}`<br>`asset_tag` $\to$ `{resource_id, ...}` | **3NF / BCNF** (LHS is superkey) |
| **`bookings`** | `booking_id` | `booking_id` $\to$ `{resource_id, user_id, start_time, end_time, purpose, status, created_at}` | **3NF / BCNF** (LHS is superkey) |
| **`maintenance_logs`** | `log_id` | `log_id` $\to$ `{resource_id, technician_id, issue_description, resolution_notes, cost, start_date, end_date, status}` | **3NF / BCNF** (LHS is superkey) |

---

## 4. Relational Anomalies Eliminated

### 1. Insertion Anomaly
- **Unnormalized Problem**: If department info were embedded in the `resources` table, we couldn't record a new academic department unless that department already owned at least one piece of equipment.
- **3NF Solution**: Departments exist independently in `departments`.

### 2. Deletion Anomaly
- **Unnormalized Problem**: If the only oscilloscope in the ECE department was decommissioned and its row deleted, all information about the ECE department and its laboratory room would be permanently lost.
- **3NF Solution**: Deleting a row from `resources` leaves `departments` and `locations` untouched.

### 3. Update / Modification Anomaly
- **Unnormalized Problem**: If a laboratory changed rooms or an email address was updated, we would have to update hundreds of resource rows. If one row failed, data would become inconsistent.
- **3NF Solution**: Updating `contact_email` in `departments` requires updating exactly one row.

---

## 5. Deep-Dive: Secure Parameterized Queries vs. SQL Injection

### What is SQL Injection (SQLi)?
SQL Injection occurs when untrusted user input is directly concatenated into a dynamic SQL string, altering the query's Abstract Syntax Tree (AST) during parsing.

#### Vulnerable Example (String Concatenation):
```cpp
// VULNERABLE CODE (DO NOT DO THIS)
std::string sql = "SELECT * FROM resources WHERE asset_tag = '" + userInput + "';";
sqlite3_exec(db, sql.c_str(), callback, 0, &errMsg);
```
- If an attacker supplies `userInput = "EQ-001' OR '1'='1"`:
- The executed SQL becomes:
  ```sql
  SELECT * FROM resources WHERE asset_tag = 'EQ-001' OR '1'='1';
  ```
- Because `'1'='1'` evaluates to `TRUE`, the query dumps all records across all departments, bypassing security access controls.

---

### How Parameterized Queries Prevent SQL Injection:
Parameterized queries separate the **SQL code structure** from the **literal data payload**.

```
[SQL String with Placeholders '?']
                │
                ▼
      ┌──────────────────┐
      │ sqlite3_prepare_v2│ ──> [Lexer -> Parser -> AST -> Bytecode VM Program]
      └──────────────────┘
                │ (Query structure is locked & compiled)
                ▼
      ┌──────────────────┐
      │ sqlite3_bind_text │ ──> [Binds payload into VM register as literal string]
      └──────────────────┘
                │ (Payload CANNOT alter query logic)
                ▼
      ┌──────────────────┐
      │   sqlite3_step   │ ──> [Executes bytecode safely]
      └──────────────────┘
```

#### Secure Implementation in Our Project:
```cpp
const std::string sql = "SELECT * FROM resources WHERE asset_tag = ?;";
auto stmt = db.prepare(sql); // 1. Compile AST
DatabaseManager::bindText(stmt, 1, userInput); // 2. Bind literal data
int rc = sqlite3_step(stmt); // 3. Execute safely
```

Even if `userInput` contains `"' OR '1'='1; DROP TABLE resources; --"`, SQLite treats the entire payload as a literal byte sequence searching for an equipment whose exact tag equals that string.

---

## 6. ACID Compliance & Transaction Management

In the **College Resource Tracker**, multi-step operations are wrapped in explicit database transactions (`BEGIN TRANSACTION`, `COMMIT`, `ROLLBACK`):

### Example: Booking Reservation Workflow
1. Check schedule conflict:
   $$\text{Overlap} \iff (\text{new\_start} < \text{existing\_end}) \land (\text{new\_end} > \text{existing\_start})$$
2. `BEGIN TRANSACTION`
3. `INSERT INTO bookings (...) VALUES (...)`
4. `UPDATE resources SET status = 'booked' WHERE resource_id = ?`
5. `COMMIT`

### ACID Mapping:
- **Atomicity**: If updating the resource status fails, the booking insert is immediately rolled back using `ROLLBACK`.
- **Consistency**: Database constraints (`CHECK (status IN (...))`, `FOREIGN KEY`) are validated at every transaction boundary.
- **Isolation**: SQLite handles concurrent readers and writers using database-level locking and WAL (Write-Ahead Logging).
- **Durability**: Once `COMMIT` returns, changes are persisted to non-volatile disk storage.

---

## 7. C++ Architecture & Design Patterns

### 1. RAII Statement Wrapper (`StatementHandle`)
In SQLite C API, failing to call `sqlite3_finalize(stmt)` results in memory leaks and database file lock contention.
We built a custom C++ RAII class:
```cpp
class StatementHandle {
public:
    explicit StatementHandle(sqlite3_stmt* stmt = nullptr) : stmt_(stmt) {}
    ~StatementHandle() {
        if (stmt_) sqlite3_finalize(stmt_);
    }
    // Move constructor and move assignment implemented for safe transfer
    ...
};
```
Whenever `StatementHandle` goes out of scope (even if an exception is thrown), `sqlite3_finalize` is guaranteed to execute.

### 2. Singleton Database Manager
`DatabaseManager::getInstance()` guarantees a single unified connection handle across services, avoiding connection churn and mutex locking conflicts.

### 3. Layered Service Architecture
- **Presentation Layer**: `UI` (ANSI terminal, input sanitization, ASCII tables)
- **Service Layer**: `ResourceService`, `BookingService`, `MaintenanceService` (Business rules & transactions)
- **Data Access Layer**: `DatabaseManager` (Native C API SQLite engine & prepared statements)

---

## 8. Top 15 Technical Interview Questions & Model Answers

### Q1: Why did you choose 3NF instead of BCNF?
> **Answer**: All of our relations satisfy both 3NF and Boyce-Codd Normal Form (BCNF) because in every functional dependency $X \to Y$, the determinant $X$ is a superkey. There are no overlapping composite candidate keys that would cause a violation of BCNF.

### Q2: How does SQLite handle foreign key constraints by default?
> **Answer**: By default, SQLite has foreign key constraints disabled for backward compatibility. In our C++ database manager, we explicitly execute `PRAGMA foreign_keys = ON;` immediately upon opening the database connection to ensure all referential integrity rules are strictly enforced.

### Q3: What happens if a user tries to delete a department that has active labs and equipment?
> **Answer**: Because we specified `ON DELETE RESTRICT` on our foreign keys, SQLite rejects the deletion and returns an error constraint violation (`SQLITE_CONSTRAINT_FOREIGNKEY`), preventing orphan records.

### Q4: How do you detect overlapping bookings in SQL?
> **Answer**: Two time intervals $[A, B]$ and $[C, D]$ overlap if and only if $A < D$ and $B > C$. In our query:
> ```sql
> SELECT COUNT(*) FROM bookings 
> WHERE resource_id = ? AND status = 'active' 
>   AND (? < end_time AND ? > start_time);
> ```
> If `COUNT(*)` $> 0$, the reservation is rejected.

### Q5: What is the time complexity of looking up a resource by asset tag?
> **Answer**: Looking up by `asset_tag` takes $\mathcal{O}(\log N)$ time because `asset_tag` is marked with a `UNIQUE` constraint, which creates a B-Tree index in SQLite. Without an index, it would require a full table scan of $\mathcal{O}(N)$.

### Q6: What is the difference between `sqlite3_step()` and `sqlite3_exec()`?
> **Answer**: `sqlite3_exec()` is a wrapper that parses, prepares, and executes SQL in a single step using string concatenation, making it vulnerable to SQL injection if variables are concatenated. `sqlite3_step()` is used in conjunction with `sqlite3_prepare_v2()` and `sqlite3_bind_*()`, executing pre-compiled bytecode with bound parameters safely.

### Q7: Why do you pass `SQLITE_TRANSIENT` to `sqlite3_bind_text()`?
> **Answer**: `SQLITE_TRANSIENT` tells SQLite that the string buffer in C++ might be modified or destroyed after the bind call, so SQLite must make its own internal copy of the string data before returning.

### Q8: How does your application ensure ACID compliance during equipment booking?
> **Answer**: When booking an item, we need to create a record in `bookings` and set the status in `resources` to `'booked'`. We wrap both SQL statements in `BEGIN TRANSACTION` and `COMMIT`. If either step fails, we issue `ROLLBACK`, guaranteeing Atomicity and Consistency.

### Q9: What is a Transitive Dependency? Give an example from a college system.
> **Answer**: A transitive dependency exists when non-key attribute $A$ determines non-key attribute $B$, which determines non-key attribute $C$ ($A \to B \to C$). For example, `student_id -> department_id -> department_hod`. If stored in a single table, changing the HOD would require updating every student record in that department. 3NF removes this by splitting it into separate `students` and `departments` tables.

### Q10: How would you scale this database if the university grew to 1,000,000 resources?
> **Answer**: 
> 1. Add composite indexes on `(department_id, status)` and `(resource_id, start_time, end_time)`.
> 2. Implement database connection pooling and migrate from SQLite to client-server RDBMS like PostgreSQL with WAL replication.
> 3. Partition the `bookings` table by year/semester using range partitioning.

### Q11: What is the role of RAII in your C++ database layer?
> **Answer**: RAII (Resource Acquisition Is Initialization) ties resource lifetime to object lifetime. We encapsulate `sqlite3_stmt*` inside a `StatementHandle` class whose destructor calls `sqlite3_finalize()`. This prevents statement and memory leaks even during early returns or unexpected errors.

### Q12: What is the difference between 2NF and 3NF?
> **Answer**: 
> - **2NF** removes partial dependencies (where a non-prime attribute depends on only part of a composite candidate key).
> - **3NF** removes transitive dependencies (where a non-prime attribute depends on another non-prime attribute).

### Q13: Can a resource under maintenance be booked in your system?
> **Answer**: No. Before booking, `BookingService::createBooking()` inspects the resource status and rejects requests if status is `'under_maintenance'` or `'decommissioned'`. Furthermore, logging a maintenance ticket atomically flips the resource status to `'under_maintenance'`.

### Q14: How does your code handle database errors gracefully?
> **Answer**: All database calls inspect return codes (`SQLITE_OK`, `SQLITE_DONE`, `SQLITE_ROW`). On failure, the exact SQLite error message from `sqlite3_errmsg()` is propagated back to the service layer and presented cleanly to the user via UI error banners without crashing the application.

### Q15: What design pattern did you use for the database connection?
> **Answer**: The **Singleton Pattern** in `DatabaseManager` ensures a single global point of access to the database instance while preventing multiple unsynchronized connection handles from conflicting on the SQLite file lock.
