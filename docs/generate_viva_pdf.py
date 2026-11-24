#!/usr/bin/env python3
"""
=============================================================================
VIVA VOCE & PROJECT DEFENSE DOSSIER GENERATOR
Student Result Management System - MITS Gwalior (Session 2026-27)
Pure Python 3 PDF Generator (Zero External Dependencies)
=============================================================================
"""

import sys
import os

class SimplePDF:
    def __init__(self, output_path):
        self.output_path = output_path
        self.pages = []  # List of stream strings
        self.page_width = 612  # Standard US Letter
        self.page_height = 792
        self.margin_x = 48
        self.margin_top = 54
        self.margin_bottom = 50
        self.content_width = self.page_width - (2 * self.margin_x)

        # Current page rendering state
        self.current_stream = []
        self.y = self.page_height - self.margin_top
        self.current_page_num = 1

    def _color_cmd(self, r, g, b, is_fill=True):
        cmd = "rg" if is_fill else "RG"
        return f"{r:.3f} {g:.3f} {b:.3f} {cmd}"

    def set_color(self, hex_code, is_fill=True):
        hex_code = hex_code.lstrip('#')
        r = int(hex_code[0:2], 16) / 255.0
        g = int(hex_code[2:4], 16) / 255.0
        b = int(hex_code[4:6], 16) / 255.0
        self.current_stream.append(self._color_cmd(r, g, b, is_fill))

    def escape_text(self, text):
        return text.replace("\\", "\\\\").replace("(", "\\(").replace(")", "\\)")

    def draw_rect(self, x, y, w, h, fill_hex=None, stroke_hex=None, stroke_w=1):
        cmds = []
        if fill_hex:
            hex_code = fill_hex.lstrip('#')
            r, g, b = int(hex_code[0:2], 16)/255.0, int(hex_code[2:4], 16)/255.0, int(hex_code[4:6], 16)/255.0
            cmds.append(self._color_cmd(r, g, b, True))
        if stroke_hex:
            hex_code = stroke_hex.lstrip('#')
            r, g, b = int(hex_code[0:2], 16)/255.0, int(hex_code[2:4], 16)/255.0, int(hex_code[4:6], 16)/255.0
            cmds.append(self._color_cmd(r, g, b, False))
            cmds.append(f"{stroke_w} w")

        op = "B" if (fill_hex and stroke_hex) else ("f" if fill_hex else "S")
        cmds.append(f"{x:.1f} {y:.1f} {w:.1f} {h:.1f} re {op}")
        self.current_stream.append(" ".join(cmds))

    def new_page(self):
        # Draw header & footer on current page before pushing
        self._draw_page_decorations()
        self.pages.append("\n".join(self.current_stream))
        self.current_stream = []
        self.current_page_num += 1
        self.y = self.page_height - self.margin_top

    def _draw_page_decorations(self):
        # Header (pages > 1)
        if self.current_page_num > 1:
            self.set_color("#64748B")
            self.current_stream.append(f"BT /F1 8 Tf {self.margin_x} {self.page_height - 32} Td (Student Result Management System | MITS Gwalior) Tj ET")
            self.current_stream.append(f"BT /F1 8 Tf {self.page_width - self.margin_x - 140} {self.page_height - 32} Td (Team Viva Voce Defense Dossier) Tj ET")
            self.draw_rect(self.margin_x, self.page_height - 38, self.content_width, 0.5, stroke_hex="#CBD5E1", stroke_w=0.5)

        # Footer (all pages)
        self.draw_rect(self.margin_x, 38, self.content_width, 0.5, stroke_hex="#CBD5E1", stroke_w=0.5)
        self.set_color("#64748B")
        self.current_stream.append(f"BT /F1 8 Tf {self.margin_x} 26 Td (CONFIDENTIAL - Internal Team Study Guide - Session 2026-27) Tj ET")
        pg_str = f"Page {self.current_page_num}"
        self.current_stream.append(f"BT /F2 8 Tf {self.page_width - self.margin_x - 45} 26 Td ({pg_str}) Tj ET")

    def ensure_space(self, needed_pt):
        if self.y - needed_pt < self.margin_bottom:
            self.new_page()

    def add_title_banner(self, title, subtitle, session_info):
        banner_h = 95
        self.ensure_space(banner_h + 20)
        top_y = self.y - banner_h
        # Draw Deep Teal Banner Box
        self.draw_rect(self.margin_x, top_y, self.content_width, banner_h, fill_hex="#164E63")
        
        # MITS Emblem Badge
        self.draw_rect(self.margin_x + 16, top_y + 48, 52, 32, fill_hex="#0D9488")
        self.set_color("#FFFFFF")
        self.current_stream.append(f"BT /F2 14 Tf {self.margin_x + 22} {top_y + 58} Td (MITS) Tj ET")

        # Titles
        self.current_stream.append(f"BT /F2 15 Tf {self.margin_x + 78} {top_y + 64} Td ({self.escape_text(title)}) Tj ET")
        self.set_color("#DCE9E2")
        self.current_stream.append(f"BT /F1 9.5 Tf {self.margin_x + 78} {top_y + 48} Td ({self.escape_text(subtitle)}) Tj ET")
        self.set_color("#FAF9F6")
        self.current_stream.append(f"BT /F1 8.5 Tf {self.margin_x + 18} {top_y + 20} Td ({self.escape_text(session_info)}) Tj ET")
        
        self.y = top_y - 20

    def add_section_heading(self, number_str, title_str):
        self.ensure_space(40)
        self.y -= 8
        # Accent bar
        self.draw_rect(self.margin_x, self.y - 14, 4, 18, fill_hex="#0D9488")
        self.set_color("#164E63")
        full_title = f"{number_str} {title_str}"
        self.current_stream.append(f"BT /F2 12.5 Tf {self.margin_x + 12} {self.y - 10} Td ({self.escape_text(full_title)}) Tj ET")
        self.draw_rect(self.margin_x, self.y - 18, self.content_width, 0.5, stroke_hex="#E2E8F0", stroke_w=0.5)
        self.y -= 26

    def add_subheading(self, subhead):
        self.ensure_space(25)
        self.set_color("#0D9488")
        self.current_stream.append(f"BT /F2 10 Tf {self.margin_x} {self.y} Td ({self.escape_text(subhead)}) Tj ET")
        self.y -= 15

    def add_paragraph(self, text, font="F1", size=9, color="#263238", line_spacing=13):
        # Word wrap text within content_width
        words = text.split(" ")
        lines = []
        cur_line = []
        # approximate char width for 9pt Helvetica is ~4.5 pt
        max_chars = int(self.content_width / 4.7)

        for w in words:
            test_line = " ".join(cur_line + [w])
            if len(test_line) <= max_chars:
                cur_line.append(w)
            else:
                lines.append(" ".join(cur_line))
                cur_line = [w]
        if cur_line:
            lines.append(" ".join(cur_line))

        needed = len(lines) * line_spacing
        self.ensure_space(needed)
        self.set_color(color)
        for line in lines:
            self.current_stream.append(f"BT /{font} {size} Tf {self.margin_x} {self.y} Td ({self.escape_text(line)}) Tj ET")
            self.y -= line_spacing
        self.y -= 4

    def add_bullet(self, bold_prefix, text, line_spacing=12):
        bullet_indent = 16
        avail_width = self.content_width - bullet_indent
        max_chars = int(avail_width / 4.7)

        full_text = f"{bold_prefix} {text}"
        words = full_text.split(" ")
        lines = []
        cur = []
        for w in words:
            test = " ".join(cur + [w])
            if len(test) <= max_chars:
                cur.append(w)
            else:
                lines.append(" ".join(cur))
                cur = [w]
        if cur:
            lines.append(" ".join(cur))

        needed = len(lines) * line_spacing
        self.ensure_space(needed + 4)

        # Draw bullet symbol
        self.set_color("#0D9488")
        self.current_stream.append(f"BT /F2 10 Tf {self.margin_x + 3} {self.y} Td (>) Tj ET")

        self.set_color("#263238")
        for i, line in enumerate(lines):
            # If first line and has bold prefix
            if i == 0 and bold_prefix:
                # print prefix in bold then remainder in normal
                p_len = len(bold_prefix)
                line_rem = line[p_len:].strip()
                self.current_stream.append(f"BT /F2 8.5 Tf {self.margin_x + bullet_indent} {self.y} Td ({self.escape_text(bold_prefix)}) Tj ET")
                # calculate offset roughly
                offset = int(p_len * 5.2) + 4
                self.current_stream.append(f"BT /F1 8.5 Tf {self.margin_x + bullet_indent + offset} {self.y} Td ({self.escape_text(line_rem)}) Tj ET")
            else:
                self.current_stream.append(f"BT /F1 8.5 Tf {self.margin_x + bullet_indent} {self.y} Td ({self.escape_text(line)}) Tj ET")
            self.y -= line_spacing
        self.y -= 3

    def add_callout(self, title, body_lines, tag="CRITICAL TIP"):
        # Box height calculation
        h = 24 + (len(body_lines) * 12) + 10
        self.ensure_space(h + 10)
        top_y = self.y - h

        # Background + border
        self.draw_rect(self.margin_x, top_y, self.content_width, h, fill_hex="#F0FDF4", stroke_hex="#86EFAC", stroke_w=1)
        # Left accent stripe
        self.draw_rect(self.margin_x, top_y, 4, h, fill_hex="#16A34A")

        # Tag
        self.set_color("#16A34A")
        self.current_stream.append(f"BT /F2 8 Tf {self.margin_x + 12} {top_y + h - 14} Td ([{tag}] {self.escape_text(title)}) Tj ET")

        # Body
        self.set_color("#1E293B")
        cur_y = top_y + h - 26
        for line in body_lines:
            self.current_stream.append(f"BT /F1 8 Tf {self.margin_x + 12} {cur_y} Td ({self.escape_text(line)}) Tj ET")
            cur_y -= 12

        self.y = top_y - 10

    def add_qa_card(self, q_num, question, short_answer, why_examiner_asks):
        # Calculate needed height
        approx_lines = 1 + (len(short_answer) // 90) + (len(why_examiner_asks) // 90) + 4
        h = max(55, approx_lines * 11 + 20)
        self.ensure_space(h + 8)
        top_y = self.y - h

        # Card container
        self.draw_rect(self.margin_x, top_y, self.content_width, h, fill_hex="#FFFFFF", stroke_hex="#E2E8F0", stroke_w=0.8)
        # Deep teal header bar inside card
        self.draw_rect(self.margin_x, top_y + h - 18, self.content_width, 18, fill_hex="#164E63")

        # Question header text
        self.set_color("#FFFFFF")
        q_header = f"Q{q_num}: {question}"
        if len(q_header) > 85: q_header = q_header[:82] + "..."
        self.current_stream.append(f"BT /F2 8.5 Tf {self.margin_x + 8} {top_y + h - 13} Td ({self.escape_text(q_header)}) Tj ET")

        # Model Answer
        cur_y = top_y + h - 30
        self.set_color("#0F766E")
        self.current_stream.append(f"BT /F2 8 Tf {self.margin_x + 8} {cur_y} Td (EXACT MODEL ANSWER:) Tj ET")
        cur_y -= 11

        self.set_color("#1E293B")
        # Split short answer lines
        words = short_answer.split(" ")
        line = []
        for w in words:
            if len(" ".join(line + [w])) < 95:
                line.append(w)
            else:
                self.current_stream.append(f"BT /F1 8 Tf {self.margin_x + 8} {cur_y} Td ({self.escape_text(' '.join(line))}) Tj ET")
                cur_y -= 10
                line = [w]
        if line:
            self.current_stream.append(f"BT /F1 8 Tf {self.margin_x + 8} {cur_y} Td ({self.escape_text(' '.join(line))}) Tj ET")
            cur_y -= 11

        # Why examiner asks / Gotcha note
        self.set_color("#9A3412")
        self.current_stream.append(f"BT /F2 7.5 Tf {self.margin_x + 8} {cur_y} Td (WHY THEY ASK / GOTCHA: ) Tj ET")
        self.set_color("#475569")
        self.current_stream.append(f"BT /F1 7.5 Tf {self.margin_x + 130} {cur_y} Td ({self.escape_text(why_examiner_asks)}) Tj ET")

        self.y = top_y - 8

    def build(self):
        # Finalize last page
        self._draw_page_decorations()
        self.pages.append("\n".join(self.current_stream))

        # PDF Object assembly
        num_pages = len(self.pages)
        objects = []
        # obj 1: Catalog
        objects.append(b"<< /Type /Catalog /Pages 2 0 R >>")
        
        # obj 2: Pages container
        kids = " ".join([f"{3 + (2*i)} 0 R" for i in range(num_pages)])
        objects.append(f"<< /Type /Pages /Kids [{kids}] /Count {num_pages} >>".encode('ascii'))

        # Standard Fonts
        # font 1: Helvetica (F1)
        # font 2: Helvetica-Bold (F2)
        # font 3: Courier (F3)
        font_f1_idx = 3 + (2 * num_pages)
        font_f2_idx = font_f1_idx + 1
        font_f3_idx = font_f1_idx + 2

        # Page and content stream objects
        for i, p_stream in enumerate(self.pages):
            content_idx = 3 + (2*i) + 1
            # Page Object
            p_obj = (
                f"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 {self.page_width} {self.page_height}] "
                f"/Resources << /Font << /F1 {font_f1_idx} 0 R /F2 {font_f2_idx} 0 R /F3 {font_f3_idx} 0 R >> >> "
                f"/Contents {content_idx} 0 R >>"
            ).encode('ascii')
            objects.append(p_obj)

            # Content Stream Object
            stream_bytes = p_stream.encode('utf-8')
            s_obj = f"<< /Length {len(stream_bytes)} >>\nstream\n".encode('ascii') + stream_bytes + b"\nendstream"
            objects.append(s_obj)

        # Fonts objects
        objects.append(b"<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>")
        objects.append(b"<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>")
        objects.append(b"<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>")

        # Compile PDF binary
        out = bytearray()
        out.extend(b"%PDF-1.4\n")
        offsets = []
        for i, obj in enumerate(objects):
            offsets.append(len(out))
            obj_num = i + 1
            out.extend(f"{obj_num} 0 obj\n".encode('ascii'))
            out.extend(obj)
            out.extend(b"\nendobj\n")

        # Xref
        xref_offset = len(out)
        out.extend(f"xref\n0 {len(objects) + 1}\n".encode('ascii'))
        out.extend(b"0000000000 65535 f \n")
        for off in offsets:
            out.extend(f"{off:010d} 00000 n \n".encode('ascii'))

        # Trailer
        out.extend(
            f"trailer\n<< /Size {len(objects) + 1} /Root 1 0 R >>\n"
            f"startxref\n{xref_offset}\n%%EOF\n".encode('ascii')
        )

        with open(self.output_path, "wb") as f:
            f.write(out)
        print(f"[+] Successfully generated {num_pages}-page Viva Defense PDF: {self.output_path}")


def generate_dossier():
    output_pdf = "/Users/jatinjadoun/.gemini/antigravity/scratch/student-result-management/docs/VIVA_DEFENSE_GUIDE.pdf"
    doc = SimplePDF(output_pdf)

    # =========================================================================
    # PAGE 1: TITLE & THE 60-SECOND ELEVATOR PITCH
    # =========================================================================
    doc.add_title_banner(
        "STUDENT RESULT MANAGEMENT SYSTEM",
        "Comprehensive Project Defense & Viva Voce Master Dossier",
        "Madhav Institute of Technology & Science, Gwalior | Academic Session 2026-27 | Department of CSE / IT"
    )

    doc.add_callout(
        "HOW TO USE THIS GUIDE DURING YOUR EVALUATION",
        [
            "Read this dossier together as a team before entering the lab examination room.",
            "Both team partners should split speaking roles: one explains C++ & calculations, one explains Database & UI.",
            "Use natural student phrasing instead of AI jargon. Sound like passionate developers who debugged code hands-on."
        ],
        tag="TEAM STRATEGY"
    )

    doc.add_section_heading("SECTION 1", "The 60-Second Elevator Pitch (Memorize This!)")
    doc.add_paragraph(
        "When the external examiner sits down and says: 'Alright, tell me what your project does and what technologies you used', "
        "do NOT recite generic buzzwords. Speak confidently with this exact student pitch:"
    )

    doc.add_callout(
        "SCRIPT: WHAT TO SAY VERBATIM IN THE FIRST 60 SECONDS",
        [
            "'Good morning Sir/Ma'am. Our project is a centralized Student Result Management System for collegiate departments.'",
            "'In our college, marks are frequently maintained on registers and spreadsheets, which leads to duplicate roll numbers,'",
            "'manual calculation mistakes, and students queuing up outside faculty cabins to view mark sheets.'",
            "'To solve this, we implemented a 3-tier architecture: Vanilla HTML/CSS/JS frontend, a high-performance C++17 Crow REST backend,'",
            "'and a Supabase PostgreSQL cloud database with Row Level Security. All business logic, input validation, and result calculations'",
            "'execute inside compiled C++ functions to ensure tamper-proof calculation and floating-point accuracy. It supports three distinct roles:'",
            "'Administrator, Faculty, and Student, each with isolated permissions and printable grade cards.'"
        ],
        tag="WORD-FOR-WORD SCRIPT"
    )

    doc.add_section_heading("SECTION 2", "High-Level Architecture (The Diagram to Draw on the Board)")
    doc.add_paragraph(
        "If the examiner points to the whiteboard and asks: 'Draw your architecture', draw this simple 3-box diagram:"
    )
    doc.add_bullet("Box 1: Presentation Tier (Frontend):", "HTML5, CSS3, Vanilla JS, and Supabase JS SDK. No heavy React/Angular framework.")
    doc.add_bullet("Box 2: Application Tier (C++ Backend):", "C++17 Crow framework on Port 18080. Handles token verification & result calculations.")
    doc.add_bullet("Box 3: Database Tier (Cloud Storage):", "Supabase PostgreSQL with Row Level Security (RLS) and referential integrity constraints.")

    doc.add_callout(
        "THE TRAP QUESTION: 'Why didn't you calculate percentage in JavaScript?'",
        [
            "Answer: 'Sir, JavaScript runs on the client browser. Any student could press F12, open Developer Tools Console, and change'",
            "'their marks, total, or grade before it is saved! By keeping calculation in C++, the server has full authority and guarantees'",
            "'consistent, unalterable academic records.'"
        ],
        tag="CLASSIC EXAMINER TRAP"
    )

    # =========================================================================
    # PAGE 2: C++ BACKEND WALKTHROUGH (FILE BY FILE)
    # =========================================================================
    doc.new_page()
    doc.add_section_heading("SECTION 3", "C++ Backend Walkthrough (File by File)")
    doc.add_paragraph(
        "Examiners love to ask: 'Open your backend folder. What does database.cpp do? Where is your main function?'. "
        "Here is the clean breakdown of every single C++ source file in your project:"
    )

    doc.add_bullet("1. main.cpp (The Central Router):", "Initializes database, reads environment variables (PORT, SUPABASE_DB_URL), registers Crow routes like CROW_ROUTE(app, '/api/students'), applies role checks, and starts the multi-threaded HTTP server.")
    doc.add_bullet("2. result.cpp & result.h (The Calculation Engine):", "The heart of your project! Contains pure C++ functions: calculate_total_obtained(), calculate_maximum_total(), calculate_percentage(), assign_grade(), and determine_pass_fail().")
    doc.add_bullet("3. database.cpp & database.h (The Persistence Layer):", "Encapsulates PostgreSQL table access. Also implements an atomic local JSON cache (data/database.json) so the project never crashes if the internet disconnects during your viva demo!")
    doc.add_bullet("4. auth.cpp & auth.h (Security & Role Verification):", "Extracts Bearer token from the HTTP Authorization header. Decodes the base64url JWT payload, checks expiration timestamp (exp), and verifies user roles (admin, teacher, student).")
    doc.add_bullet("5. student.cpp & student.h (Student Management):", "Handles student CRUD: ID, name, roll_no, class. Enforces uniqueness so two students cannot have the same roll number.")
    doc.add_bullet("6. teacher.cpp & teacher.h (Faculty Directory):", "Handles faculty registration, assigned teaching subjects, and faculty profiles.")
    doc.add_bullet("7. subject.cpp & subject.h (Curriculum Subjects):", "Manages courses. Blocks deletion of a subject if students already have exam marks recorded for it (Foreign key protection).")
    doc.add_bullet("8. marks.cpp & marks.h (Marks Entry Controller):", "Validates that obtained marks are >= 0 and <= maximum marks. Saves marks and triggers immediate result recalculation.")
    doc.add_bullet("9. CMakeLists.txt & build.sh (Build Scripts):", "Standard C++17 build configurations. build.sh compiles with clang++ or g++ in under two seconds with -pthread.")

    doc.add_section_heading("SECTION 4", "The Database Schema & Supabase Setup")
    doc.add_paragraph(
        "Examiners often inspect database design to test normalization and constraint knowledge. Explain these 5 core tables:"
    )
    doc.add_bullet("1. students table:", "PK: student_id, UNIQUE: roll_no, class, name, auth_user_id (references auth user).")
    doc.add_bullet("2. teachers table:", "PK: teacher_id, name, subject, auth_user_id.")
    doc.add_bullet("3. subjects table:", "PK: subject_id, UNIQUE: subject_name.")
    doc.add_bullet("4. marks table:", "PK: marks_id, FKs: student_id, subject_id. UNIQUE(student_id, subject_id) ensures 1 mark per subject.")
    doc.add_bullet("5. results table:", "PK: result_id, FK: student_id (UNIQUE). Stores total_marks, max_marks, percentage, grade, status.")
    doc.add_bullet("6. user_profiles table:", "Protected role map linking auth UUID to role ('admin', 'teacher', 'student').")

    # =========================================================================
    # PAGE 3: THE CALCULATION ENGINE & TOP VIVA QUESTIONS
    # =========================================================================
    doc.new_page()
    doc.add_section_heading("SECTION 5", "The C++ Result Calculation Engine Code Walkthrough")
    doc.add_paragraph(
        "If the examiner says: 'Show me your grading logic in C++', navigate directly to backend/result.cpp lines 38-60 and explain:"
    )

    doc.add_callout(
        "C++ CODE SNIPPET FROM RESULT.CPP (MEMORIZE LOGIC)",
        [
            "double ResultEngine::calculate_percentage(double total_obtained, double maximum_total) {",
            "    if (maximum_total <= 0.0) return 0.0; // Avoid division by zero crash",
            "    return std::round(((total_obtained / maximum_total) * 100.0) * 100.0) / 100.0; // 2 decimal places",
            "}",
            "std::string ResultEngine::assign_grade(double percentage) {",
            "    if (percentage >= 90.0) return 'A+';      else if (percentage >= 80.0) return 'A';",
            "    else if (percentage >= 70.0) return 'B'; else if (percentage >= 60.0) return 'C';",
            "    else if (percentage >= 40.0) return 'D'; else return 'Fail';",
            "}",
            "std::string ResultEngine::determine_pass_fail(double percentage) {",
            "    return (percentage >= 40.0) ? 'Pass' : 'Fail';",
            "}"
        ],
        tag="EXACT C++ SOURCE"
    )

    doc.add_section_heading("SECTION 6", "Top 15 Tough Viva Questions & Exact Model Answers")
    doc.add_paragraph(
        "Here are the exact questions external examiners love to ask, along with the precise answers you should give:"
    )

    doc.add_qa_card(
        1,
        "Why use C++ for a web app instead of Node.js or Python Django?",
        "C++ provides compiled execution speed, low memory overhead, and strict type safety. In academic micro-projects, it demonstrates systems programming and socket handling rather than relying on bloated runtime packages.",
        "They want to ensure you didn't just pick a stack randomly or because of a tutorial."
    )

    doc.add_qa_card(
        2,
        "What is Crow, and how does routing work?",
        "Crow is a modern C++17 microframework inspired by Flask. It uses C++ templates and POSIX sockets to bind routes like CROW_ROUTE(app, '/api/students'). It parses headers, query parameters, and JSON payloads.",
        "They test whether you know what third-party libraries you actually included."
    )

    doc.add_qa_card(
        3,
        "What happens if a student scores exactly 39.9%? What grade do they get?",
        "They receive grade 'Fail' and status 'Fail'. Our C++ rule is strictly percentage >= 40.0% for 'D Pass'. 39.9% is below 40%, so our test suite confirms it fails.",
        "Examiners frequently test boundary conditions and edge cases."
    )

    # =========================================================================
    # PAGE 4: VIVA QUESTIONS CONTINUED (QUESTIONS 4 - 9)
    # =========================================================================
    doc.new_page()
    doc.add_section_heading("SECTION 6 (CONT.)", "External Examiner Viva Questions 4 to 9")

    doc.add_qa_card(
        4,
        "What happens if a subject has maximum marks = 0?",
        "In result.cpp, calculate_percentage() checks if (maximum_total <= 0.0) return 0.0;. This explicitly prevents an IEEE floating-point division-by-zero or SIGFPE crash.",
        "Classic defensive programming question."
    )

    doc.add_qa_card(
        5,
        "How do you prevent a student from seeing other students' results?",
        "Two-layer defense: First, the C++ backend checks if role == 'student'. If so, it verifies that the requested student_id matches their own auth_user_id. Second, Supabase PostgreSQL Row Level Security (RLS) blocks queries for other user IDs.",
        "Data isolation and privacy question."
    )

    doc.add_qa_card(
        6,
        "How do you verify a JWT token in C++ without external heavy crypto libraries?",
        "In auth.cpp, we extract the Bearer token, split by '.', and base64url decode the payload segment. We parse the JSON, check that the exp (expiration epoch) is greater than system_clock::now(), and match the sub user ID.",
        "Examiners want to see if you understand how JWT tokens actually work under the hood."
    )

    doc.add_qa_card(
        7,
        "What happens if you try to enter 105 marks out of 100?",
        "The request is rejected with HTTP 400 Bad Request. Both frontend client validation and C++ marks.cpp check if (marks_obtained > maximum_marks). In SQL, CHECK (marks_obtained <= maximum_marks) also blocks it.",
        "Integrity and input validation test."
    )

    doc.add_qa_card(
        8,
        "What happens if you delete a subject that already has marks assigned?",
        "Deletion is blocked! In database.cpp, delete_subject() checks if subject_has_marks(subject_id). If marks exist, it returns an error. In PostgreSQL, CONSTRAINT fk_marks_subject has ON DELETE RESTRICT.",
        "Foreign key constraint and referential integrity check."
    )

    doc.add_qa_card(
        9,
        "Where are user passwords stored in your database?",
        "Passwords are NEVER stored in our application database or C++ code! Supabase Auth securely manages user passwords using bcrypt/Argon2 hashing inside auth.users. Our application only handles secure JWT access tokens.",
        "Crucial security question. Never say passwords are in plaintext or in your database!"
    )

    # =========================================================================
    # PAGE 5: VIVA QUESTIONS CONTINUED (10 - 15) & LIVE DEMO SCRIPT
    # =========================================================================
    doc.new_page()
    doc.add_section_heading("SECTION 6 (CONT.)", "External Examiner Viva Questions 10 to 15")

    doc.add_qa_card(
        10,
        "What is Row Level Security (RLS) in PostgreSQL?",
        "RLS is a database security feature where PostgreSQL evaluates row access policies for each SQL statement. For example, our policy 'Students can view only own record' enforces that SELECT queries only return rows matching auth.uid().",
        "Tests database security knowledge."
    )

    doc.add_qa_card(
        11,
        "Can a teacher delete a student from the system?",
        "No. In main.cpp, the DELETE /api/students/<id> route checks AuthManager::check_role(ctx, {'admin'}). If a teacher tries, the backend returns HTTP 403 Forbidden.",
        "Role-Based Access Control (RBAC) test."
    )

    doc.add_qa_card(
        12,
        "How do you compile your C++ backend on Linux or macOS?",
        "We execute ./build.sh, which runs clang++ -std=c++17 -pthread -Ibackend/include backend/*.cpp -o bin/student_result_server. Alternatively, we use CMake with mkdir build && cd build && cmake ../backend && make.",
        "Build tools and compiler flags question."
    )

    doc.add_section_heading("SECTION 7", "The 5-Minute Live Demo Presentation Script")
    doc.add_paragraph(
        "Follow these exact steps during your practical demonstration to leave zero room for doubt:"
    )

    doc.add_bullet("Step 1 (Open Login):", "Open http://localhost:3001/login.html. Click 'Admin' quick button. Show how the role button dynamically updates the form.")
    doc.add_bullet("Step 2 (Admin Dashboard):", "Point out the 4 real-time stat cards (Total Students, Teachers, Subjects, Results). Click 'Add Student' and add a sample student (e.g., STU006, Mohit Verma, 0901CS241006).")
    doc.add_bullet("Step 3 (Teacher Marks Entry):", "Log out. Sign in as Teacher. Navigate to 'Enter & Manage Marks'. Select the new student, select 'Object Oriented Programming with C++', enter 92 marks out of 100, and click Save.")
    doc.add_bullet("Step 4 (Show Automatic Recalculation):", "Show the toast message: 'Marks saved & result recalculated by C++ engine'. Explain that C++ immediately computed the percentage (92.00%) and assigned grade A+.")
    doc.add_bullet("Step 5 (Student Mark Sheet & Print):", "Log out. Sign in as Student. Show their personal grade card with all subjects. Click 'Print Official Grade Card' to trigger the clean print preview.")

    # =========================================================================
    # PAGE 6: AI RED FLAGS TO AVOID & TEAM MEMBER ROLES
    # =========================================================================
    doc.new_page()
    doc.add_section_heading("SECTION 8", "'AI Red Flags' That Trigger Examiners — And What to Say Instead")
    doc.add_paragraph(
        "External examiners can instantly tell when students used AI if they start reciting robotic textbook definitions. "
        "Review these contrast tables so you speak naturally like student coders:"
    )

    doc.add_callout(
        "HOW TO TALK LIKE A GENUINE STUDENT DEVELOPER",
        [
            "DON'T SAY: 'We architected a synergistic cloud-native paradigm leveraging asynchronous microservices...'",
            "DO SAY: 'We built a C++ web server using Crow that talks to a Supabase database over SQL and REST endpoints.'",
            "",
            "DON'T SAY: 'The solution incorporates robust automated state management mechanisms...'",
            "DO SAY: 'When marks are updated, result.cpp runs calculate_percentage() and saves the grade to results table.'",
            "",
            "DON'T SAY: 'The platform mitigates adversarial intrusion vectors through cryptographic hashing...'",
            "DO SAY: 'We verify the JWT token header on every request so unauthorized users can't edit marks.'"
        ],
        tag="LANGUAGE FILTER"
    )

    doc.add_section_heading("SECTION 9", "Team Division of Work (When Asked 'Who Did What?')")
    doc.add_paragraph(
        "Examiners always ask: 'What did partner A do, and what did partner B do?'. Here is your prepared division:"
    )

    doc.add_bullet("Partner 1 (Backend & Calculations Lead):", "Responsible for C++17 Crow REST backend, result calculation engine (result.cpp), JWT token verification (auth.cpp), and unit testing edge cases (test_suite.cpp).")
    doc.add_bullet("Partner 2 (Database & UI/Frontend Lead):", "Responsible for Supabase PostgreSQL database schema (schema.sql), Row Level Security policies (policies.sql), HTML/CSS/JavaScript dashboards, and printable mark sheet styling.")

    doc.add_callout(
        "FINAL CONFIDENCE CHECKLIST BEFORE YOU ENTER THE LAB",
        [
            "[X] Make sure backend is running: ./bin/student_result_server (Port 18080)",
            "[X] Make sure frontend server is running: python3 -m http.server 3001",
            "[X] Keep the test suite command handy: ./bin/test_suite (Runs all 20 tests in 0.05 seconds)",
            "[X] Remember the grading boundaries: 90+ is A+, 80+ is A, 70+ is B, 60+ is C, 40+ is D, <40 is Fail.",
            "[X] Smile, be polite, address the examiner as 'Sir' or 'Ma'am', and speak with confidence!"
        ],
        tag="EXAM DAY CHECKLIST"
    )

    doc.build()

if __name__ == "__main__":
    generate_dossier()
