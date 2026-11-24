-- ============================================================================
-- STUDENT RESULT MANAGEMENT SYSTEM
-- Institution: Madhav Institute of Technology & Science, Gwalior
-- Academic Session: 2026-27
-- Database Schema for Supabase PostgreSQL
-- ============================================================================

-- Clean up existing tables if resetting schema (in reverse dependency order)
DROP TABLE IF EXISTS results CASCADE;
DROP TABLE IF EXISTS marks CASCADE;
DROP TABLE IF EXISTS subjects CASCADE;
DROP TABLE IF EXISTS teachers CASCADE;
DROP TABLE IF EXISTS students CASCADE;
DROP TABLE IF EXISTS user_profiles CASCADE;

-- ----------------------------------------------------------------------------
-- 0. USER PROFILES & ROLES
-- Protected application user profile table linked to Supabase Auth
-- Roles: 'admin', 'teacher', 'student'
-- ----------------------------------------------------------------------------
CREATE TABLE user_profiles (
    id UUID PRIMARY KEY, -- Linked to auth.users.id
    email VARCHAR(255) NOT NULL UNIQUE,
    role VARCHAR(20) NOT NULL CHECK (role IN ('admin', 'teacher', 'student')),
    name VARCHAR(150) NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_user_profiles_role ON user_profiles(role);

-- ----------------------------------------------------------------------------
-- 1. STUDENTS TABLE
-- ----------------------------------------------------------------------------
CREATE TABLE students (
    student_id VARCHAR(50) PRIMARY KEY,
    name VARCHAR(150) NOT NULL,
    class VARCHAR(100) NOT NULL,
    roll_no VARCHAR(50) NOT NULL UNIQUE,
    auth_user_id UUID UNIQUE, -- Linked to Supabase Auth User ID (optional during initial creation)
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_students_auth FOREIGN KEY (auth_user_id) REFERENCES user_profiles(id) ON DELETE SET NULL
);

CREATE INDEX idx_students_roll_no ON students(roll_no);
CREATE INDEX idx_students_class ON students(class);

-- ----------------------------------------------------------------------------
-- 2. TEACHERS TABLE
-- ----------------------------------------------------------------------------
CREATE TABLE teachers (
    teacher_id VARCHAR(50) PRIMARY KEY,
    name VARCHAR(150) NOT NULL,
    subject VARCHAR(100) NOT NULL,
    auth_user_id UUID UNIQUE, -- Linked to Supabase Auth User ID
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_teachers_auth FOREIGN KEY (auth_user_id) REFERENCES user_profiles(id) ON DELETE SET NULL
);

CREATE INDEX idx_teachers_subject ON teachers(subject);

-- ----------------------------------------------------------------------------
-- 3. SUBJECTS TABLE
-- ----------------------------------------------------------------------------
CREATE TABLE subjects (
    subject_id VARCHAR(50) PRIMARY KEY,
    subject_name VARCHAR(150) NOT NULL UNIQUE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- ----------------------------------------------------------------------------
-- 4. MARKS TABLE
-- ----------------------------------------------------------------------------
CREATE TABLE marks (
    marks_id VARCHAR(50) PRIMARY KEY,
    student_id VARCHAR(50) NOT NULL,
    subject_id VARCHAR(50) NOT NULL,
    marks_obtained NUMERIC(5, 2) NOT NULL CHECK (marks_obtained >= 0),
    maximum_marks NUMERIC(5, 2) NOT NULL CHECK (maximum_marks > 0),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    
    -- Referential Integrity
    CONSTRAINT fk_marks_student FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
    CONSTRAINT fk_marks_subject FOREIGN KEY (subject_id) REFERENCES subjects(subject_id) ON DELETE RESTRICT,
    
    -- Business Logic Check: Obtained marks cannot exceed maximum marks
    CONSTRAINT chk_marks_range CHECK (marks_obtained <= maximum_marks),
    
    -- Unique constraint: A student can only have one marks entry per subject
    CONSTRAINT uq_student_subject UNIQUE (student_id, subject_id)
);

CREATE INDEX idx_marks_student_id ON marks(student_id);
CREATE INDEX idx_marks_subject_id ON marks(subject_id);

-- ----------------------------------------------------------------------------
-- 5. RESULTS TABLE
-- Calculated and stored by the C++ Result Calculation Engine
-- ----------------------------------------------------------------------------
CREATE TABLE results (
    result_id VARCHAR(50) PRIMARY KEY,
    student_id VARCHAR(50) NOT NULL UNIQUE,
    total_marks NUMERIC(7, 2) NOT NULL,
    maximum_total_marks NUMERIC(7, 2) NOT NULL,
    percentage NUMERIC(5, 2) NOT NULL,
    grade VARCHAR(10) NOT NULL CHECK (grade IN ('A+', 'A', 'B', 'C', 'D', 'Fail')),
    result_status VARCHAR(10) NOT NULL CHECK (result_status IN ('Pass', 'Fail')),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    
    -- Referential Integrity
    CONSTRAINT fk_results_student FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE
);

CREATE INDEX idx_results_student_id ON results(student_id);
CREATE INDEX idx_results_status ON results(result_status);

-- ============================================================================
-- SEED DATA: Madhav Institute of Technology & Science, Gwalior (2026-27)
-- ============================================================================

-- Subjects
INSERT INTO subjects (subject_id, subject_name) VALUES
('SUB101', 'Object Oriented Programming with C++'),
('SUB102', 'Database Management Systems'),
('SUB103', 'Data Structures & Algorithms'),
('SUB104', 'Computer Networks'),
('SUB105', 'Discrete Mathematics');

-- Students
INSERT INTO students (student_id, name, class, roll_no) VALUES
('STU001', 'Aarav Sharma', 'B.Tech CSE III Sem', '0901CS241001'),
('STU002', 'Priya Patel', 'B.Tech CSE III Sem', '0901CS241002'),
('STU003', 'Rohan Gupta', 'B.Tech CSE III Sem', '0901CS241003'),
('STU004', 'Ananya Singh', 'B.Tech CSE III Sem', '0901CS241004'),
('STU005', 'Vikramaditya Verma', 'B.Tech CSE III Sem', '0901CS241005');

-- Teachers
INSERT INTO teachers (teacher_id, name, subject) VALUES
('TCH001', 'Dr. Rajesh Kumar Sharma', 'Object Oriented Programming with C++'),
('TCH002', 'Prof. Sunita Verma', 'Database Management Systems'),
('TCH003', 'Dr. Amit Dixit', 'Data Structures & Algorithms');

-- Marks for STU001 (Aarav Sharma) - Distinction / A+
INSERT INTO marks (marks_id, student_id, subject_id, marks_obtained, maximum_marks) VALUES
('MRK001', 'STU001', 'SUB101', 94.00, 100.00),
('MRK002', 'STU001', 'SUB102', 91.00, 100.00),
('MRK003', 'STU001', 'SUB103', 95.00, 100.00),
('MRK004', 'STU001', 'SUB104', 88.00, 100.00),
('MRK005', 'STU001', 'SUB105', 92.00, 100.00);

-- Marks for STU002 (Priya Patel) - Grade A
INSERT INTO marks (marks_id, student_id, subject_id, marks_obtained, maximum_marks) VALUES
('MRK006', 'STU002', 'SUB101', 82.00, 100.00),
('MRK007', 'STU002', 'SUB102', 85.00, 100.00),
('MRK008', 'STU002', 'SUB103', 88.00, 100.00),
('MRK009', 'STU002', 'SUB104', 79.00, 100.00),
('MRK010', 'STU002', 'SUB105', 81.00, 100.00);

-- Marks for STU003 (Rohan Gupta) - Grade B
INSERT INTO marks (marks_id, student_id, subject_id, marks_obtained, maximum_marks) VALUES
('MRK011', 'STU003', 'SUB101', 72.00, 100.00),
('MRK012', 'STU003', 'SUB102', 74.00, 100.00),
('MRK013', 'STU003', 'SUB103', 70.00, 100.00),
('MRK014', 'STU003', 'SUB104', 75.00, 100.00),
('MRK015', 'STU003', 'SUB105', 71.00, 100.00);

-- Marks for STU004 (Ananya Singh) - Grade C
INSERT INTO marks (marks_id, student_id, subject_id, marks_obtained, maximum_marks) VALUES
('MRK016', 'STU004', 'SUB101', 65.00, 100.00),
('MRK017', 'STU004', 'SUB102', 62.00, 100.00),
('MRK018', 'STU004', 'SUB103', 68.00, 100.00),
('MRK019', 'STU004', 'SUB104', 61.00, 100.00),
('MRK020', 'STU004', 'SUB105', 64.00, 100.00);

-- Calculated Results (calculated via C++ Engine rules)
-- STU001: Total = 460 / 500, Percentage = 92.00%, Grade = A+, Status = Pass
INSERT INTO results (result_id, student_id, total_marks, maximum_total_marks, percentage, grade, result_status) VALUES
('RES001', 'STU001', 460.00, 500.00, 92.00, 'A+', 'Pass'),
('RES002', 'STU002', 415.00, 500.00, 83.00, 'A', 'Pass'),
('RES003', 'STU003', 362.00, 500.00, 72.40, 'B', 'Pass'),
('RES004', 'STU004', 320.00, 500.00, 64.00, 'C', 'Pass');
