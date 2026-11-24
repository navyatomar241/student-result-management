# System Architecture & Technical Specifications

**Student Result Management System**  
**Institution:** Madhav Institute of Technology & Science, Gwalior  
**Academic Session:** 2026–27  
**Department:** Computer Science & Engineering / Information Technology  

---

## 1. High-Level Architecture

The Student Result Management System is architected as a decoupled, multi-tier web application that enforces a strict separation of concerns:

```
+-------------------------------------------------------------+
|                 PRESENTATION TIER (Frontend)                |
|  - HTML5, CSS3, Vanilla JavaScript                          |
|  - Supabase JavaScript Client (Session Token Management)     |
|  - Responsive Dashboards for Admin, Teacher, Student        |
+------------------------------+------------------------------+
                               |
                               | (1) Authorization: Bearer <JWT>
                               | (2) JSON REST API Requests
                               v
+-------------------------------------------------------------+
|                 APPLICATION TIER (C++ Backend)              |
|  - C++17 Crow Microframework REST API                       |
|  - JWT Signature & Claim Verification (AuthManager)         |
|  - Role-Based Access Control (RBAC)                         |
|  - Academic Business Logic & Input Validation               |
|  - Result Calculation Engine (Percentage, Grade, Pass/Fail) |
+------------------------------+------------------------------+
                               |
                               | (3) SQL Queries & Connection
                               v
+-------------------------------------------------------------+
|                    DATA TIER (Database)                     |
|  - Supabase PostgreSQL Relational Database                  |
|  - Row Level Security (RLS) Policies                        |
|  - Referential Integrity (Foreign Keys & Cascades)          |
|  - Fallback Local JSON Persistence for Offline Dev          |
+-------------------------------------------------------------+
```

---

## 2. Security & Authentication Flow

1. **User Authentication:**
   - The user submits their email and password through `login.html`.
   - Supabase Auth verifies credentials and issues a signed JSON Web Token (JWT).
   - The JWT is securely cached in browser session storage.

2. **API Request Dispatch:**
   - Every protected API request from the browser includes the header:
     ```http
     Authorization: Bearer <SUPABASE_JWT_ACCESS_TOKEN>
     ```

3. **C++ Backend Verification (`auth.cpp`):**
   - Extracts and decodes the JWT payload.
   - Verifies token expiry (`exp`).
   - Retrieves the authenticated user identity (`sub`) and role (`admin`, `teacher`, `student`).
   - Rejects unauthenticated or forbidden requests with HTTP 401/403 before executing any business logic.

4. **Data Isolation:**
   - Administrators possess full CRUD privileges over students, teachers, subjects, marks, and results.
   - Teachers are permitted to view students, view subjects, enter marks, and calculate results.
   - Students can only view their own profile, subject marks, and calculated grade card.

---

## 3. Academic Calculation Rules & Grading Engine

All calculations execute exclusively inside dedicated C++ functions (`result.cpp`):

### Formulas:
- **Total Obtained Marks:**
  $$\text{Total} = \sum_{i=1}^{n} \text{marks\_obtained}_i$$
- **Maximum Aggregate Marks:**
  $$\text{Max Total} = \sum_{i=1}^{n} \text{maximum\_marks}_i$$
- **Percentage:**
  $$\text{Percentage} = \left(\frac{\text{Total}}{\text{Max Total}}\right) \times 100$$
  *(Computed using IEEE 754 floating-point arithmetic and rounded to 2 decimal places).*

### Grading Scale:
| Percentage Range | Grade Assigned | Academic Status |
|---|:---:|:---:|
| 90.00% – 100.00% | **A+** | Pass |
| 80.00% – 89.99% | **A** | Pass |
| 70.00% – 79.99% | **B** | Pass |
| 60.00% – 69.99% | **C** | Pass |
| 40.00% – 59.99% | **D** | Pass |
| Below 40.00% | **Fail** | Fail |

---

## 4. REST API Endpoint Reference

| HTTP Method | Endpoint | Allowed Roles | Description |
|---|---|---|---|
| `GET` | `/` | Public | System status and institution details |
| `GET` | `/api/stats` | Public / Auth | Dashboard summary card metrics |
| `GET` | `/api/auth/me` | Authenticated | Current user identity, role, and profile |
| `GET` | `/api/students` | Admin, Teacher | List all students (supports `?q=search`) |
| `POST` | `/api/students` | Admin | Register new student record |
| `GET` | `/api/students/{id}` | Admin, Teacher, Student (Self) | Retrieve specific student details |
| `PUT` | `/api/students/{id}` | Admin | Update student details |
| `DELETE` | `/api/students/{id}` | Admin | Delete student and cascade delete records |
| `GET` | `/api/teachers` | Admin, Teacher | List faculty members |
| `POST` | `/api/teachers` | Admin | Register new teacher |
| `PUT` | `/api/teachers/{id}` | Admin | Update teacher information |
| `DELETE` | `/api/teachers/{id}` | Admin | Remove teacher |
| `GET` | `/api/subjects` | Authenticated | List all active curriculum subjects |
| `POST` | `/api/subjects` | Admin | Register new subject |
| `PUT` | `/api/subjects/{id}` | Admin | Update subject title |
| `DELETE` | `/api/subjects/{id}` | Admin | Delete subject (blocked if marks exist) |
| `GET` | `/api/marks` | Admin, Teacher, Student (Self) | Retrieve marks records |
| `POST` | `/api/marks` | Teacher, Admin | Record marks & auto-recalculate result |
| `PUT` | `/api/marks/{id}` | Teacher, Admin | Update marks & auto-recalculate result |
| `DELETE` | `/api/marks/{id}` | Admin | Delete marks entry |
| `POST` | `/api/results/calculate` | Teacher, Admin | Trigger C++ result calculation engine |
| `GET` | `/api/results` | Teacher, Admin | List all calculated student results |
| `GET` | `/api/results/{student_id}` | Admin, Teacher, Student (Self) | Full mark sheet with subject breakdown |
