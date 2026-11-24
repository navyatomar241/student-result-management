# Student Result Management System

**Academic Institution:** Madhav Institute of Technology & Science, Gwalior  
**Academic Session:** 2026–27  
**Project Category:** College Micro-Project (B.Tech CSE / IT)  
**Primary Backend Language:** C++ (C++17)  
**Database & Authentication:** Supabase (PostgreSQL + Supabase Auth)  

---

## 1. Project Overview

The **Student Result Management System** is a centralized, digital academic evaluation platform designed to eliminate manual calculation errors, duplicate record entries, and inconsistent grading registers in college departments.

The application allows:
- **Administrators** to register and manage students, faculty members, and academic subjects.
- **Teachers** to record examination marks, update scores, and automatically compute results.
- **Students** to access their personal academic dashboard, review subject-wise performance, and print official grade cards.

All business logic, input validation, and academic calculations execute in compiled **C++17**, while user authentication and cloud database hosting are managed via **Supabase PostgreSQL**.

---

## 2. Why C++ is Used

While modern web applications frequently use dynamic scripting languages (such as JavaScript or Python), this project intentionally employs **C++17** for the backend:
1. **Computational Accuracy:** C++ provides native, deterministic floating-point arithmetic without unpredictable type conversions.
2. **Speed & Efficiency:** Compiled C++ machine code handles calculation loops with minimal memory footprint and zero virtual machine overhead.
3. **Academic Demonstration:** Proves practical mastery of C++ object-oriented design, POSIX sockets, multi-threading, JSON serialization, and REST architecture for college project evaluation.

---

## 3. Why Supabase is Used

[Supabase](https://supabase.com) is utilized as an open-source Firebase alternative powered by PostgreSQL:
1. **Hosted PostgreSQL:** Eliminates the complexity of hosting and configuring local database servers during college demonstrations.
2. **Built-in Authentication:** Manages encrypted passwords, user registration, and issues signed JSON Web Tokens (JWTs).
3. **Row Level Security (RLS):** Enforces database-level access policies so that students can never query other students' academic records.

---

## 4. How the Frontend Communicates with C++

The architecture follows a clean three-tier model:

```
[ Frontend: HTML5 / CSS3 / Vanilla JS ]
                    │
                    │ 1. Submits email/password to Supabase Auth
                    ▼
           [ Supabase Auth ]
                    │
                    │ 2. Issues Bearer JWT
                    ▼
[ Frontend attaches: Authorization: Bearer <TOKEN> ]
                    │
                    │ 3. HTTP REST Requests (GET, POST, PUT, DELETE)
                    ▼
       [ C++ Crow Backend (Port 18080) ]
                    │
                    │ 4. Parametrized SQL / Local Persistence
                    ▼
       [ Supabase PostgreSQL Database ]
```

1. The user logs in via `login.html`.
2. Supabase Auth validates credentials and provides an access token.
3. The frontend `apiFetch` utility passes the token in every request:
   ```http
   Authorization: Bearer <JWT_ACCESS_TOKEN>
   ```
4. The C++ `AuthManager` verifies the token, checks role permissions, processes the request, and returns structured JSON responses.

---

## 5. How Supabase Auth Works

1. **User Accounts:** Users are registered in Supabase Auth's `auth.users` table.
2. **Profile Mapping:** Each user has an entry in the application's `user_profiles` table linking their unique `UUID` to their designated role (`admin`, `teacher`, or `student`).
3. **Token Claims:** When a user logs in, Supabase issues an industry-standard JWT containing:
   - `sub`: User UUID
   - `email`: User email address
   - `exp`: Expiration timestamp
4. **C++ Verification:** The C++ backend extracts the token, verifies that `exp > current_time`, matches the user ID, and verifies permissions before performing any database action.

---

## 6. How C++ Connects to PostgreSQL

- In production or live cloud mode, C++ connects to Supabase PostgreSQL using the standard PostgreSQL connection URI configured in `SUPABASE_DB_URL`.
- The connection utilizes parametrized SQL statements to prevent SQL injection vulnerabilities.
- If cloud credentials are not yet supplied (such as during local development or offline college evaluation), the backend seamlessly activates an atomic local storage file (`data/database.json`) seeded with MITS Gwalior data, ensuring the application remains 100% functional without connectivity issues.

---

## 7. How C++ Calculates Student Results

The **Result Calculation Engine** (`backend/result.cpp`) executes the official academic rules:

1. **Total Obtained Marks:**
   $$\text{Total} = \sum \text{marks\_obtained}$$
2. **Maximum Aggregate Marks:**
   $$\text{Maximum Total} = \sum \text{maximum\_marks}$$
3. **Percentage:**
   $$\text{Percentage} = \left(\frac{\text{Total}}{\text{Maximum Total}}\right) \times 100$$
4. **Official Grading Rules:**
   - **90% – 100%:** Grade `A+` (Pass)
   - **80% – 89%:** Grade `A` (Pass)
   - **70% – 79%:** Grade `B` (Pass)
   - **60% – 69%:** Grade `C` (Pass)
   - **40% – 59%:** Grade `D` (Pass)
   - **Below 40%:** Grade `Fail` (Fail)
5. **Division-by-Zero Protection:** If maximum marks equal zero, percentage defaults to `0.00%` safely without crashing.
6. **Automatic Trigger:** Whenever marks are added, updated, or removed, C++ recalculates the result immediately and updates the database.

---

## 8. Database Tables Organization

The database schema (`supabase/schema.sql`) consists of five primary academic tables:

| Table Name | Primary Key | Foreign Keys | Key Constraints | Purpose |
|---|---|---|---|---|
| `students` | `student_id` | `auth_user_id` | `roll_no` (UNIQUE) | Student profile and class records |
| `teachers` | `teacher_id` | `auth_user_id` | -- | Faculty members and assigned subjects |
| `subjects` | `subject_id` | -- | `subject_name` (UNIQUE) | Registered academic courses |
| `marks` | `marks_id` | `student_id`, `subject_id` | `UNIQUE(student_id, subject_id)` | Subject-wise examination marks |
| `results` | `result_id` | `student_id` (UNIQUE) | Grade CHECK constraint | Calculated summary and Pass/Fail status |

Plus the protected role profile table:
- `user_profiles` (`id UUID PRIMARY KEY`, `email`, `role CHECK ('admin', 'teacher', 'student')`, `name`).

---

## 9. Supabase Cloud Configuration Guide

1. Create a free account at [supabase.com](https://supabase.com) and create a new project.
2. Open your project's **SQL Editor**.
3. Copy and execute the contents of `supabase/schema.sql` to create all tables and initial MITS seed data.
4. Copy and execute the contents of `supabase/policies.sql` to activate Row Level Security (RLS).
5. Navigate to **Project Settings -> API** to obtain your `Project URL` and `anon public key`.
6. Navigate to **Project Settings -> Database** to copy your direct connection string (URI).

---

## 10. Environment Configuration

Create a `.env` file in the root directory using `.env.example`:

```bash
cp .env.example .env
```

Populate the variables with your Supabase credentials:

```ini
SUPABASE_URL=https://your-project-id.supabase.co
SUPABASE_ANON_KEY=your-supabase-anon-key
SUPABASE_DB_URL=postgresql://postgres:your_password@db.your-project-id.supabase.co:5432/postgres
PORT=18080
```

---

## 11. How to Compile & Run the C++ Backend

### Method A: Single-Command Build Script (Recommended)
From the project root directory, run:

```bash
./build.sh
```

This compiles all modules into an executable binary located at `bin/student_result_server`.

### Method B: Building with CMake
```bash
mkdir -p build && cd build
cmake ../backend
make
```

### Method C: Running the Test Suite
To verify all 20 requirements and edge cases (zero marks, full marks, boundary grades 40%, 60%, 70%, 80%, 90%, and CRUD operations):

```bash
clang++ -std=c++17 -Ibackend/include -Ibackend backend/test_suite.cpp backend/database.cpp backend/auth.cpp backend/student.cpp backend/teacher.cpp backend/subject.cpp backend/marks.cpp backend/result.cpp -pthread -o bin/test_suite
./bin/test_suite
```

### Starting the Server
```bash
./bin/student_result_server
```
The server will start listening on `http://127.0.0.1:18080`.

---

## 12. How to Run the Frontend

The frontend is lightweight HTML5/CSS3/JavaScript. You can serve it with Python's built-in HTTP server:

```bash
cd frontend
python3 -m http.server 3000
```

Open your browser and navigate to:
```
http://127.0.0.1:3000/
```

---

## 13. How to Test Each User Role

The login screen (`login.html`) provides role tabs and pre-filled demo accounts:

### 1. Administrator (`admin.html`)
- **Email:** `admin@mits.ac.in`
- **Permissions:** Full control over students, teachers, subjects, marks, and calculated results.
- **Workflow:** Add a new student, create a new subject, and view overall statistics.

### 2. Faculty / Teacher (`teacher.html`)
- **Email:** `teacher@mits.ac.in`
- **Permissions:** View student list, select a student, and enter/update marks.
- **Workflow:** Enter marks for `STU001` or `STU005`. Notice that the C++ backend immediately validates the marks and recalculates the student's result.

### 3. Student (`student.html`)
- **Email:** `student@mits.ac.in`
- **Permissions:** View personal academic information and mark sheet.
- **Workflow:** Click **Print Official Grade Card** to test the browser print layout.

---

## 14. Major C++ Source Files Explained

| Source File | Header File | Simple Explanation |
|---|---|---|
| `main.cpp` | -- | The main entry point. Sets up the Crow web server, loads configuration, and registers REST API routes. |
| `database.cpp` | `database.h` | Database management layer. Connects to PostgreSQL, manages tables, enforces unique constraints, and handles atomic persistence. |
| `auth.cpp` | `auth.h` | Security layer. Decodes and verifies Supabase JWT access tokens and checks user roles before executing protected actions. |
| `result.cpp` | `result.h` | The core academic calculation engine. Calculates totals, percentage, assigns grades (A+, A, B, C, D, Fail), and determines Pass/Fail status. |
| `student.cpp` | `student.h` | Student management controller. Implements student registration, search, editing, and deletion. |
| `teacher.cpp` | `teacher.h` | Faculty management controller. Handles faculty directory records. |
| `subject.cpp` | `subject.h` | Subject controller. Manages course subjects and blocks deletion if examination marks exist. |
| `marks.cpp` | `marks.h` | Marks entry controller. Validates marks range and triggers automatic result recalculation. |

---

## 15. Institution & Evaluation Credits

- **Institution:** Madhav Institute of Technology & Science, Gwalior (M.P.)
- **Affiliation:** A Govt. Aided UGC Autonomous Institute Affiliated to RGPV Bhopal
- **Academic Session:** 2026–27
- **Project Type:** College Micro-Project
