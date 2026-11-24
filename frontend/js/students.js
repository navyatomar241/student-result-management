// ============================================================================
// STUDENT MANAGEMENT MODULE (Frontend Controller)
// Student Result Management System - MITS Gwalior
// ============================================================================

const StudentsModule = (() => {
    let currentStudents = [];

    async function loadStudents(search = '') {
        const tableBody = document.getElementById('students-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="6" style="text-align:center;padding:2rem;">Loading student records...</td></tr>`;

        try {
            const query = search ? `?q=${encodeURIComponent(search)}` : '';
            const res = await App.apiFetch(`/api/students${query}`);
            currentStudents = res.students || [];

            if (currentStudents.length === 0) {
                tableBody.innerHTML = `<tr><td colspan="6" style="text-align:center;padding:2rem;color:var(--text-secondary);">No student records found.</td></tr>`;
                return;
            }

            renderTable(currentStudents);
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="6" style="text-align:center;padding:2rem;color:var(--error);">Failed to load students: ${App.escapeHTML(err.message)}</td></tr>`;
        }
    }

    function renderTable(list) {
        const tableBody = document.getElementById('students-table-body');
        if (!tableBody) return;

        const user = Auth.getUser();
        const isAdmin = user && user.role === 'admin';

        tableBody.innerHTML = list.map(s => `
            <tr>
                <td><strong>${App.escapeHTML(s.student_id)}</strong></td>
                <td>${App.escapeHTML(s.name)}</td>
                <td><code>${App.escapeHTML(s.roll_no)}</code></td>
                <td>${App.escapeHTML(s.class)}</td>
                <td><small style="color:var(--text-secondary);">${App.escapeHTML(s.created_at ? s.created_at.substring(0, 10) : 'N/A')}</small></td>
                <td style="text-align:right;">
                    ${isAdmin ? `
                        <button class="btn btn-outline btn-sm" onclick="StudentsModule.openEditModal('${s.student_id}')">Edit</button>
                        <button class="btn btn-danger btn-sm" onclick="StudentsModule.deleteStudent('${s.student_id}')">Delete</button>
                    ` : `
                        <button class="btn btn-outline btn-sm" onclick="ResultsModule.viewStudentMarksheet('${s.student_id}')">View Result</button>
                    `}
                </td>
            </tr>
        `).join('');
    }

    function openAddModal() {
        const form = document.getElementById('student-form');
        if (form) form.reset();
        document.getElementById('student-modal-title').textContent = 'Add New Student';
        document.getElementById('student-id-field').removeAttribute('readonly');
        document.getElementById('is-editing-student').value = 'false';
        App.openModal('student-modal');
    }

    function openEditModal(studentId) {
        const student = currentStudents.find(s => s.student_id === studentId);
        if (!student) return;

        document.getElementById('student-modal-title').textContent = 'Edit Student Details';
        document.getElementById('student-id-field').value = student.student_id;
        document.getElementById('student-id-field').setAttribute('readonly', 'true');
        document.getElementById('student-name-field').value = student.name;
        document.getElementById('student-roll-field').value = student.roll_no;
        document.getElementById('student-class-field').value = student.class;
        document.getElementById('is-editing-student').value = 'true';

        App.openModal('student-modal');
    }

    async function handleFormSubmit(e) {
        e.preventDefault();
        const isEditing = document.getElementById('is-editing-student').value === 'true';
        const studentId = document.getElementById('student-id-field').value.trim();
        const name = document.getElementById('student-name-field').value.trim();
        const rollNo = document.getElementById('student-roll-field').value.trim();
        const className = document.getElementById('student-class-field').value.trim();

        if (!studentId || !name || !rollNo || !className) {
            App.showToast('Please fill in all required student details', 'error');
            return;
        }

        const payload = {
            student_id: studentId,
            name: name,
            roll_no: rollNo,
            class: className
        };

        try {
            if (isEditing) {
                await App.apiFetch(`/api/students/${encodeURIComponent(studentId)}`, {
                    method: 'PUT',
                    body: JSON.stringify(payload)
                });
                App.showToast('Student details updated successfully', 'success');
            } else {
                await App.apiFetch('/api/students', {
                    method: 'POST',
                    body: JSON.stringify(payload)
                });
                App.showToast('New student added successfully', 'success');
            }

            App.closeModal('student-modal');
            loadStudents();
            // Refresh stats if on dashboard
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    async function deleteStudent(studentId) {
        if (!confirm(`Are you sure you want to permanently delete student '${studentId}'? All associated examination marks and calculated results will also be deleted.`)) {
            return;
        }

        try {
            await App.apiFetch(`/api/students/${encodeURIComponent(studentId)}`, {
                method: 'DELETE'
            });
            App.showToast('Student record deleted successfully', 'success');
            loadStudents();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    function init() {
        const form = document.getElementById('student-form');
        if (form) form.addEventListener('submit', handleFormSubmit);

        const searchInput = document.getElementById('student-search-input');
        if (searchInput) {
            let debounceTimer;
            searchInput.addEventListener('input', (e) => {
                clearTimeout(debounceTimer);
                debounceTimer = setTimeout(() => {
                    loadStudents(e.target.value.trim());
                }, 300);
            });
        }
    }

    return {
        init,
        loadStudents,
        openAddModal,
        openEditModal,
        deleteStudent
    };
})();
