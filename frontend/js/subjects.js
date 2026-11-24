// ============================================================================
// SUBJECT MANAGEMENT MODULE (Frontend Controller)
// Student Result Management System - MITS Gwalior
// ============================================================================

const SubjectsModule = (() => {
    let currentSubjects = [];

    async function loadSubjects() {
        const tableBody = document.getElementById('subjects-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="4" style="text-align:center;padding:2rem;">Loading academic subjects...</td></tr>`;

        try {
            const res = await App.apiFetch('/api/subjects');
            currentSubjects = res.subjects || [];

            if (currentSubjects.length === 0) {
                tableBody.innerHTML = `<tr><td colspan="4" style="text-align:center;padding:2rem;color:var(--text-secondary);">No subjects registered yet.</td></tr>`;
                return;
            }

            renderTable(currentSubjects);
            populateSubjectDropdowns(currentSubjects);
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="4" style="text-align:center;padding:2rem;color:var(--error);">Failed to load subjects: ${App.escapeHTML(err.message)}</td></tr>`;
        }
    }

    function renderTable(list) {
        const tableBody = document.getElementById('subjects-table-body');
        if (!tableBody) return;

        const user = Auth.getUser();
        const isAdmin = user && user.role === 'admin';

        tableBody.innerHTML = list.map(s => `
            <tr>
                <td><strong>${App.escapeHTML(s.subject_id)}</strong></td>
                <td>${App.escapeHTML(s.subject_name)}</td>
                <td><small style="color:var(--text-secondary);">${App.escapeHTML(s.created_at ? s.created_at.substring(0, 10) : 'N/A')}</small></td>
                <td style="text-align:right;">
                    ${isAdmin ? `
                        <button class="btn btn-outline btn-sm" onclick="SubjectsModule.openEditModal('${s.subject_id}')">Edit</button>
                        <button class="btn btn-danger btn-sm" onclick="SubjectsModule.deleteSubject('${s.subject_id}')">Delete</button>
                    ` : `
                        <span class="badge" style="background:#f1f5f9;color:#475569;">Active</span>
                    `}
                </td>
            </tr>
        `).join('');
    }

    function populateSubjectDropdowns(list) {
        const marksSubjectSelect = document.getElementById('marks-subject-select');
        if (marksSubjectSelect) {
            marksSubjectSelect.innerHTML = `<option value="">-- Choose Subject --</option>` +
                list.map(s => `<option value="${App.escapeHTML(s.subject_id)}">${App.escapeHTML(s.subject_id)} - ${App.escapeHTML(s.subject_name)}</option>`).join('');
        }
    }

    function openAddModal() {
        const form = document.getElementById('subject-form');
        if (form) form.reset();
        document.getElementById('subject-modal-title').textContent = 'Add New Subject';
        document.getElementById('subject-id-field').removeAttribute('readonly');
        document.getElementById('is-editing-subject').value = 'false';
        App.openModal('subject-modal');
    }

    function openEditModal(subjectId) {
        const sub = currentSubjects.find(s => s.subject_id === subjectId);
        if (!sub) return;

        document.getElementById('subject-modal-title').textContent = 'Edit Subject';
        document.getElementById('subject-id-field').value = sub.subject_id;
        document.getElementById('subject-id-field').setAttribute('readonly', 'true');
        document.getElementById('subject-name-field').value = sub.subject_name;
        document.getElementById('is-editing-subject').value = 'true';

        App.openModal('subject-modal');
    }

    async function handleFormSubmit(e) {
        e.preventDefault();
        const isEditing = document.getElementById('is-editing-subject').value === 'true';
        const subjectId = document.getElementById('subject-id-field').value.trim();
        const subjectName = document.getElementById('subject-name-field').value.trim();

        if (!subjectId || !subjectName) {
            App.showToast('Subject ID and Name are required', 'error');
            return;
        }

        const payload = {
            subject_id: subjectId,
            subject_name: subjectName
        };

        try {
            if (isEditing) {
                await App.apiFetch(`/api/subjects/${encodeURIComponent(subjectId)}`, {
                    method: 'PUT',
                    body: JSON.stringify(payload)
                });
                App.showToast('Subject updated successfully', 'success');
            } else {
                await App.apiFetch('/api/subjects', {
                    method: 'POST',
                    body: JSON.stringify(payload)
                });
                App.showToast('New subject added successfully', 'success');
            }

            App.closeModal('subject-modal');
            loadSubjects();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    async function deleteSubject(subjectId) {
        if (!confirm(`Are you sure you want to delete subject '${subjectId}'?`)) {
            return;
        }

        try {
            await App.apiFetch(`/api/subjects/${encodeURIComponent(subjectId)}`, {
                method: 'DELETE'
            });
            App.showToast('Subject deleted successfully', 'success');
            loadSubjects();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    function init() {
        const form = document.getElementById('subject-form');
        if (form) form.addEventListener('submit', handleFormSubmit);
    }

    return {
        init,
        loadSubjects,
        openAddModal,
        openEditModal,
        deleteSubject
    };
})();
