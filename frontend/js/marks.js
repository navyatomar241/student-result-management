// ============================================================================
// MARKS MANAGEMENT MODULE (Frontend Controller)
// Student Result Management System - MITS Gwalior
// ============================================================================

const MarksModule = (() => {
    let currentMarks = [];

    async function loadMarks(studentIdFilter = '') {
        const tableBody = document.getElementById('marks-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;padding:2rem;">Loading examination marks...</td></tr>`;

        try {
            const query = studentIdFilter ? `?student_id=${encodeURIComponent(studentIdFilter)}` : '';
            const res = await App.apiFetch(`/api/marks${query}`);
            currentMarks = res.marks || [];

            if (currentMarks.length === 0) {
                tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;padding:2rem;color:var(--text-secondary);">No marks recorded for this criteria.</td></tr>`;
                return;
            }

            renderTable(currentMarks);
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center;padding:2rem;color:var(--error);">Failed to load marks: ${App.escapeHTML(err.message)}</td></tr>`;
        }
    }

    function renderTable(list) {
        const tableBody = document.getElementById('marks-table-body');
        if (!tableBody) return;

        const user = Auth.getUser();
        const canEdit = user && (user.role === 'admin' || user.role === 'teacher');
        const isAdmin = user && user.role === 'admin';

        tableBody.innerHTML = list.map(m => {
            const percentage = m.maximum_marks > 0 ? ((m.marks_obtained / m.maximum_marks) * 100).toFixed(1) : '0.0';
            return `
                <tr>
                    <td><strong>${App.escapeHTML(m.student_id)}</strong></td>
                    <td>${App.escapeHTML(m.student_name || 'N/A')}</td>
                    <td><span class="badge" style="background:#e0f2fe;color:#0369a1;">${App.escapeHTML(m.subject_name || m.subject_id)}</span></td>
                    <td><strong>${m.marks_obtained}</strong></td>
                    <td>${m.maximum_marks}</td>
                    <td><code>${percentage}%</code></td>
                    <td style="text-align:right;">
                        ${canEdit ? `
                            <button class="btn btn-outline btn-sm" onclick="MarksModule.openEditModal('${m.marks_id}')">Edit</button>
                        ` : ''}
                        ${isAdmin ? `
                            <button class="btn btn-danger btn-sm" onclick="MarksModule.deleteMark('${m.marks_id}')">Delete</button>
                        ` : ''}
                    </td>
                </tr>
            `;
        }).join('');
    }

    function populateStudentDropdowns(students) {
        const marksStudentSelect = document.getElementById('marks-student-select');
        const filterSelect = document.getElementById('marks-filter-student');

        const options = `<option value="">-- Choose Student --</option>` +
            students.map(s => `<option value="${App.escapeHTML(s.student_id)}">${App.escapeHTML(s.student_id)} - ${App.escapeHTML(s.name)} (${App.escapeHTML(s.roll_no)})</option>`).join('');

        if (marksStudentSelect) marksStudentSelect.innerHTML = options;
        if (filterSelect) {
            filterSelect.innerHTML = `<option value="">All Students</option>` +
                students.map(s => `<option value="${App.escapeHTML(s.student_id)}">${App.escapeHTML(s.roll_no)} - ${App.escapeHTML(s.name)}</option>`).join('');
        }
    }

    function openAddModal() {
        const form = document.getElementById('marks-form');
        if (form) form.reset();
        document.getElementById('marks-modal-title').textContent = 'Enter Student Examination Marks';
        document.getElementById('marks-id-field').value = '';
        document.getElementById('is-editing-marks').value = 'false';
        document.getElementById('marks-max-field').value = '100';
        document.getElementById('marks-student-select').removeAttribute('disabled');
        document.getElementById('marks-subject-select').removeAttribute('disabled');
        App.openModal('marks-modal');
    }

    function openEditModal(marksId) {
        const mark = currentMarks.find(m => m.marks_id === marksId);
        if (!mark) return;

        document.getElementById('marks-modal-title').textContent = 'Update Examination Marks';
        document.getElementById('marks-id-field').value = mark.marks_id;
        document.getElementById('is-editing-marks').value = 'true';
        document.getElementById('marks-student-select').value = mark.student_id;
        document.getElementById('marks-student-select').setAttribute('disabled', 'true');
        document.getElementById('marks-subject-select').value = mark.subject_id;
        document.getElementById('marks-subject-select').setAttribute('disabled', 'true');
        document.getElementById('marks-obtained-field').value = mark.marks_obtained;
        document.getElementById('marks-max-field').value = mark.maximum_marks;

        App.openModal('marks-modal');
    }

    async function handleFormSubmit(e) {
        e.preventDefault();
        const isEditing = document.getElementById('is-editing-marks').value === 'true';
        const marksId = document.getElementById('marks-id-field').value.trim();
        const studentId = document.getElementById('marks-student-select').value;
        const subjectId = document.getElementById('marks-subject-select').value;
        const marksObtained = parseFloat(document.getElementById('marks-obtained-field').value);
        const maxMarks = parseFloat(document.getElementById('marks-max-field').value);

        if (!studentId || !subjectId || isNaN(marksObtained) || isNaN(maxMarks)) {
            App.showToast('Please fill all marks fields with valid values', 'error');
            return;
        }

        if (marksObtained < 0) {
            App.showToast('Marks obtained cannot be negative', 'error');
            return;
        }

        if (maxMarks <= 0) {
            App.showToast('Maximum marks must be greater than zero', 'error');
            return;
        }

        if (marksObtained > maxMarks) {
            App.showToast(`Obtained marks (${marksObtained}) cannot exceed Maximum marks (${maxMarks})`, 'error');
            return;
        }

        const payload = {
            student_id: studentId,
            subject_id: subjectId,
            marks_obtained: marksObtained,
            maximum_marks: maxMarks
        };

        try {
            if (isEditing && marksId) {
                await App.apiFetch(`/api/marks/${encodeURIComponent(marksId)}`, {
                    method: 'PUT',
                    body: JSON.stringify(payload)
                });
                App.showToast('Examination marks updated & result recalculated successfully', 'success');
            } else {
                await App.apiFetch('/api/marks', {
                    method: 'POST',
                    body: JSON.stringify(payload)
                });
                App.showToast('Marks saved & result recalculated by C++ engine', 'success');
            }

            App.closeModal('marks-modal');
            loadMarks();
            if (typeof ResultsModule !== 'undefined') ResultsModule.loadResults();
            if (typeof TeacherDashboard !== 'undefined') TeacherDashboard.loadStats();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    async function deleteMark(marksId) {
        if (!confirm(`Are you sure you want to delete this marks entry? Student's result will be automatically recalculated.`)) {
            return;
        }

        try {
            await App.apiFetch(`/api/marks/${encodeURIComponent(marksId)}`, {
                method: 'DELETE'
            });
            App.showToast('Marks entry deleted and result updated', 'success');
            loadMarks();
            if (typeof ResultsModule !== 'undefined') ResultsModule.loadResults();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    function init() {
        const form = document.getElementById('marks-form');
        if (form) form.addEventListener('submit', handleFormSubmit);

        const filterSelect = document.getElementById('marks-filter-student');
        if (filterSelect) {
            filterSelect.addEventListener('change', (e) => {
                loadMarks(e.target.value);
            });
        }
    }

    return {
        init,
        loadMarks,
        populateStudentDropdowns,
        openAddModal,
        openEditModal,
        deleteMark
    };
})();
