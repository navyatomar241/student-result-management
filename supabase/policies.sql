-- ============================================================================
-- STUDENT RESULT MANAGEMENT SYSTEM
-- Supabase Row Level Security (RLS) Policies
-- ============================================================================

-- Enable Row Level Security on all core tables
ALTER TABLE user_profiles ENABLE ROW LEVEL SECURITY;
ALTER TABLE students ENABLE ROW LEVEL SECURITY;
ALTER TABLE teachers ENABLE ROW LEVEL SECURITY;
ALTER TABLE subjects ENABLE ROW LEVEL SECURITY;
ALTER TABLE marks ENABLE ROW LEVEL SECURITY;
ALTER TABLE results ENABLE ROW LEVEL SECURITY;

-- ----------------------------------------------------------------------------
-- Helper Functions to Check User Roles from JWT Claims
-- ----------------------------------------------------------------------------

-- Function to get the current authenticated user's role
CREATE OR REPLACE FUNCTION get_user_role()
RETURNS VARCHAR AS $$
    SELECT role FROM user_profiles WHERE id = auth.uid();
$$ LANGUAGE sql SECURITY DEFINER STABLE;

-- ----------------------------------------------------------------------------
-- 1. USER PROFILES POLICIES
-- ----------------------------------------------------------------------------
-- Users can read their own profile
CREATE POLICY "Users can read own profile"
ON user_profiles FOR SELECT
USING (auth.uid() = id);

-- Admins have full access to view and manage user profiles
CREATE POLICY "Admins have full access to profiles"
ON user_profiles FOR ALL
USING (get_user_role() = 'admin');

-- ----------------------------------------------------------------------------
-- 2. STUDENTS POLICIES
-- ----------------------------------------------------------------------------
-- Admins and Teachers can view all student records
CREATE POLICY "Admins and teachers can view all students"
ON students FOR SELECT
USING (get_user_role() IN ('admin', 'teacher'));

-- Students can only view their own student record
CREATE POLICY "Students can view only own record"
ON students FOR SELECT
USING (auth_user_id = auth.uid());

-- Only Admins can insert, update, or delete student records
CREATE POLICY "Only admins can insert students"
ON students FOR INSERT
WITH CHECK (get_user_role() = 'admin');

CREATE POLICY "Only admins can update students"
ON students FOR UPDATE
USING (get_user_role() = 'admin');

CREATE POLICY "Only admins can delete students"
ON students FOR DELETE
USING (get_user_role() = 'admin');

-- ----------------------------------------------------------------------------
-- 3. TEACHERS POLICIES
-- ----------------------------------------------------------------------------
-- Authenticated users (admin, teacher, student) can view teacher names/subjects
CREATE POLICY "Authenticated users can view teachers"
ON teachers FOR SELECT
USING (auth.role() = 'authenticated');

-- Only Admins can insert, update, or delete teachers
CREATE POLICY "Only admins can manage teachers"
ON teachers FOR ALL
USING (get_user_role() = 'admin');

-- ----------------------------------------------------------------------------
-- 4. SUBJECTS POLICIES
-- ----------------------------------------------------------------------------
-- All authenticated users can view subjects
CREATE POLICY "Authenticated users can view subjects"
ON subjects FOR SELECT
USING (auth.role() = 'authenticated');

-- Only Admins can create, update, or delete subjects
CREATE POLICY "Only admins can manage subjects"
ON subjects FOR ALL
USING (get_user_role() = 'admin');

-- ----------------------------------------------------------------------------
-- 5. MARKS POLICIES
-- ----------------------------------------------------------------------------
-- Admins can view all marks
CREATE POLICY "Admins can view all marks"
ON marks FOR SELECT
USING (get_user_role() = 'admin');

-- Teachers can view all marks
CREATE POLICY "Teachers can view all marks"
ON marks FOR SELECT
USING (get_user_role() = 'teacher');

-- Students can only view their own marks
CREATE POLICY "Students can view only own marks"
ON marks FOR SELECT
USING (
    student_id IN (
        SELECT student_id FROM students WHERE auth_user_id = auth.uid()
    )
);

-- Teachers and Admins can insert and update marks
CREATE POLICY "Teachers and admins can insert marks"
ON marks FOR INSERT
WITH CHECK (get_user_role() IN ('admin', 'teacher'));

CREATE POLICY "Teachers and admins can update marks"
ON marks FOR UPDATE
USING (get_user_role() IN ('admin', 'teacher'));

-- Only Admins can delete marks entries
CREATE POLICY "Only admins can delete marks"
ON marks FOR DELETE
USING (get_user_role() = 'admin');

-- ----------------------------------------------------------------------------
-- 6. RESULTS POLICIES
-- ----------------------------------------------------------------------------
-- Admins and Teachers can view all calculated results
CREATE POLICY "Admins and teachers can view all results"
ON results FOR SELECT
USING (get_user_role() IN ('admin', 'teacher'));

-- Students can only view their own result
CREATE POLICY "Students can view only own result"
ON results FOR SELECT
USING (
    student_id IN (
        SELECT student_id FROM students WHERE auth_user_id = auth.uid()
    )
);

-- Backend service / Admins / Teachers can write results
CREATE POLICY "Admins and teachers can upsert results"
ON results FOR ALL
USING (get_user_role() IN ('admin', 'teacher'));
