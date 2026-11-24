# Viva-Voce Examination Guide & FAQ

**Project Name:** Student Result Management System  
**Institution:** Madhav Institute of Technology & Science, Gwalior  
**Academic Session:** 2026–27  
**Evaluation Level:** B.Tech Micro-Project  

---

### Q1: What is the main objective of this project?
**Answer:** The objective is to eliminate manual calculation errors, duplicate student entries, and inconsistent records in academic institutions by developing a centralized, secure web application with an automated result calculation engine written in C++ and a hosted Supabase PostgreSQL database.

---

### Q2: Why is C++ used for the backend instead of Node.js or Python?
**Answer:**
1. **Computational Speed & Efficiency:** C++ provides native compiled performance with deterministic memory management and zero runtime interpretation overhead.
2. **Academic Value:** Demonstrates low-level systems programming, strict object-oriented design, socket communication, and algorithmic grading rules.
3. **Data Integrity:** Strict compile-time type checking in C++ prevents implicit type coercion bugs that commonly cause arithmetic errors in dynamic scripting languages.

---

### Q3: Why are academic calculations performed in C++ rather than frontend JavaScript or SQL?
**Answer:**
- Performing calculations in JavaScript is vulnerable to client-side tampering through browser Developer Tools, which could allow students to falsify their grades or percentages.
- Performing calculations in C++ centralizes the business logic on the server, ensuring that all grade criteria, division-by-zero checks, and rounding rules are enforced consistently before being persisted to the database.

---

### Q4: What is Crow, and how does it handle HTTP requests?
**Answer:** Crow is a lightweight, modern C++ web microframework (inspired by Python's Flask) that maps incoming HTTP requests to C++ route handlers using macros like `CROW_ROUTE(app, "/api/...")`. It parses incoming headers, request methods (GET, POST, PUT, DELETE), JSON bodies, and URL parameters, returning structured JSON HTTP responses.

---

### Q5: How does authentication and role-based authorization work in this system?
**Answer:**
1. Users authenticate via Supabase Auth using email and password.
2. Supabase issues a signed JSON Web Token (JWT).
3. The frontend passes this token in the `Authorization: Bearer <TOKEN>` header.
4. The C++ `AuthManager` decodes the JWT, verifies its expiration timestamp (`exp`), and validates the user role (`admin`, `teacher`, `student`) against the database profile before granting access to protected endpoints.

---

### Q6: What is Row Level Security (RLS) in PostgreSQL?
**Answer:** Row Level Security is a database-level security mechanism in PostgreSQL that restricts which rows can be returned or modified based on the identity and role of the querying user. In our system, RLS ensures that students can never view or modify other students' marks or results even if they somehow bypass the application layer.

---

### Q7: How does the system handle referential integrity when a student or subject is deleted?
**Answer:**
- Deleting a student cascades to remove all associated marks and results (`ON DELETE CASCADE`), ensuring no orphaned records exist.
- Deleting a subject is restricted (`ON DELETE RESTRICT`) if examination marks are recorded for that subject, preventing accidental loss of historical academic data.

---

### Q8: What edge cases are handled in the result calculation engine?
**Answer:**
- **Division by Zero:** Prevented if a student has registered subjects with zero maximum marks.
- **Marks Validation:** Obtained marks cannot be negative and cannot exceed maximum marks.
- **Grade Boundary Precision:** Tests verify exact thresholds (e.g., 40.00% is 'D Pass', 39.80% is 'Fail', 90.00% is 'A+').
- **Missing Marks:** Calculation requires marks to be present; otherwise, it returns a descriptive error rather than generating incomplete results.
