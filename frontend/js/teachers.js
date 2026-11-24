// ============================================================================
// TEACHER MANAGEMENT MODULE (Frontend Controller)
// Student Result Management System - MITS Gwalior
// ============================================================================

const TeachersModule = (() => {
    let currentTeachers = [];

    async function loadTeachers(search = '') {
        const tableBody = document.getElementById('teachers-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="5" style="text-align:center;padding:2rem;">Loading teacher records...</td></tr>`;

        try {
            const query = search ? `?q=${encodeURIComponent(search)}` : '';
            const res = await App.apiFetch(`/api/teachers${query}`);
            currentTeachers = res.teachers || [];

            if (currentTeachers.length === 0) {
                tableBody.innerHTML = `<tr><td colspan="5" style="text-align:center;padding:2rem;color:var(--text-secondary);">No teacher records found.</td></tr>`;
                return;
            }

            renderTable(currentTeachers);
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="5" style="text-align:center;padding:2rem;color:var(--error);">Failed to load teachers: ${App.escapeHTML(err.message)}</td></tr>`;
        }
    }

    function renderTable(list) {
        const tableBody = document.getElementById('teachers-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = list.map(t => `
            <tr>
                <td><strong>${App.escapeHTML(t.teacher_id)}</strong></td>
                <td>${App.escapeHTML(t.name)}</td>
                <td><span class="badge" style="background:#e0e7ff;color:#3730a3;">${App.escapeHTML(t.subject)}</span></td>
                <td><small style="color:var(--text-secondary);">${App.escapeHTML(t.created_at ? t.created_at.substring(0, 10) : 'N/A')}</small></td>
                <td style="text-align:right;">
                    <button class="btn btn-outline btn-sm" onclick="TeachersModule.openEditModal('${t.teacher_id}')">Edit</button>
                    <button class="btn btn-danger btn-sm" onclick="TeachersModule.deleteTeacher('${t.teacher_id}')">Delete</button>
                </td>
            </tr>
        `).join('');
    }

    function openAddModal() {
        const form = document.getElementById('teacher-form');
        if (form) form.reset();
        document.getElementById('teacher-modal-title').textContent = 'Add New Faculty / Teacher';
        document.getElementById('teacher-id-field').removeAttribute('readonly');
        document.getElementById('is-editing-teacher').value = 'false';
        App.openModal('teacher-modal');
    }

    function openEditModal(teacherId) {
        const teacher = currentTeachers.find(t => t.teacher_id === teacherId);
        if (!teacher) return;

        document.getElementById('teacher-modal-title').textContent = 'Edit Faculty Details';
        document.getElementById('teacher-id-field').value = teacher.teacher_id;
        document.getElementById('teacher-id-field').setAttribute('readonly', 'true');
        document.getElementById('teacher-name-field').value = teacher.name;
        document.getElementById('teacher-subject-field').value = teacher.subject;
        document.getElementById('is-editing-teacher').value = 'true';

        App.openModal('teacher-modal');
    }

    async function handleFormSubmit(e) {
        e.preventDefault();
        const isEditing = document.getElementById('is-editing-teacher').value === 'true';
        const teacherId = document.getElementById('teacher-id-field').value.trim();
        const name = document.getElementById('teacher-name-field').value.trim();
        const subject = document.getElementById('teacher-subject-field').value.trim();

        if (!teacherId || !name || !subject) {
            App.showToast('Please fill in Teacher ID, Name, and Subject', 'error');
            return;
        }

        const payload = {
            teacher_id: teacherId,
            name: name,
            subject: subject
        };

        try {
            if (isEditing) {
                await App.apiFetch(`/api/teachers/${encodeURIComponent(teacherId)}`, {
                    method: 'PUT',
                    body: JSON.stringify(payload)
                });
                App.showToast('Teacher record updated successfully', 'success');
            } else {
                await App.apiFetch('/api/teachers', {
                    method: 'POST',
                    body: JSON.stringify(payload)
                });
                App.showToast('New faculty added successfully', 'success');
            }

            App.closeModal('teacher-modal');
            loadTeachers();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    async function deleteTeacher(teacherId) {
        if (!confirm(`Are you sure you want to remove teacher '${teacherId}'?`)) {
            return;
        }

        try {
            await App.apiFetch(`/api/teachers/${encodeURIComponent(teacherId)}`, {
                method: 'DELETE'
            });
            App.showToast('Teacher record deleted successfully', 'success');
            loadTeachers();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    function init() {
        const form = document.getElementById('teacher-form');
        if (form) form.addEventListener('submit', handleFormSubmit);

        const searchInput = document.getElementById('teacher-search-input');
        if (searchInput) {
            let debounceTimer;
            searchInput.addEventListener('input', (e) => {
                clearTimeout(debounceTimer);
                debounceTimer = setTimeout(() => {
                    loadTeachers(e.target.value.trim());
                }, 300);
            });
        }
    }

    return {
        init,
        loadTeachers,
        openAddModal,
        openEditModal,
        deleteTeacher
    };
})();
