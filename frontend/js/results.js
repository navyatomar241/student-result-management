// ============================================================================
// RESULT CALCULATION & MARKSHEET DISPLAY MODULE (Frontend Controller)
// Student Result Management System - MITS Gwalior
// ============================================================================

const ResultsModule = (() => {
    let currentResults = [];

    async function loadResults() {
        const tableBody = document.getElementById('results-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="8" style="text-align:center;padding:2rem;">Loading academic results...</td></tr>`;

        try {
            const res = await App.apiFetch('/api/results');
            currentResults = res.results || [];

            if (currentResults.length === 0) {
                tableBody.innerHTML = `<tr><td colspan="8" style="text-align:center;padding:2rem;color:var(--text-secondary);">No calculated results available yet.</td></tr>`;
                return;
            }

            renderTable(currentResults);
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="8" style="text-align:center;padding:2rem;color:var(--error);">Failed to load results: ${App.escapeHTML(err.message)}</td></tr>`;
        }
    }

    function renderTable(list) {
        const tableBody = document.getElementById('results-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = list.map(r => `
            <tr>
                <td><strong>${App.escapeHTML(r.student_id)}</strong></td>
                <td>${App.escapeHTML(r.student_name || 'N/A')}</td>
                <td><code>${App.escapeHTML(r.roll_no || 'N/A')}</code></td>
                <td><strong>${r.total_marks}</strong> / ${r.maximum_total_marks}</td>
                <td><strong>${r.percentage.toFixed(2)}%</strong></td>
                <td>${App.formatGradeBadge(r.grade)}</td>
                <td>${App.formatStatusBadge(r.result_status)}</td>
                <td style="text-align:right;">
                    <button class="btn btn-outline btn-sm" onclick="ResultsModule.viewStudentMarksheet('${r.student_id}')">
                        Mark Sheet
                    </button>
                </td>
            </tr>
        `).join('');
    }

    async function triggerRecalculateAll() {
        if (!confirm('Recalculate academic results for all students using the C++ Calculation Engine?')) {
            return;
        }

        try {
            App.showToast('Executing C++ result calculation engine...', 'info');
            const res = await App.apiFetch('/api/results/calculate', {
                method: 'POST',
                body: JSON.stringify({})
            });

            App.showToast(res.message || 'Results recalculated successfully', 'success');
            loadResults();
            if (typeof AdminDashboard !== 'undefined') AdminDashboard.loadStats();
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    /**
     * View Detailed Student Mark Sheet with Browser Printing
     */
    async function viewStudentMarksheet(studentId) {
        try {
            const data = await App.apiFetch(`/api/results/${encodeURIComponent(studentId)}`);
            const s = data.student || {};
            const r = data.result || {};
            const marks = data.marks || [];

            const container = document.getElementById('marksheet-modal-content');
            if (!container) return;

            let subjectRows = marks.map(m => `
                <tr>
                    <td>${App.escapeHTML(m.subject_name || m.subject_id)}</td>
                    <td style="text-align:center;">${m.maximum_marks}</td>
                    <td style="text-align:center;"><strong>${m.marks_obtained}</strong></td>
                    <td style="text-align:center;">${m.maximum_marks > 0 ? ((m.marks_obtained / m.maximum_marks) * 100).toFixed(1) : 0}%</td>
                </tr>
            `).join('');

            if (marks.length === 0) {
                subjectRows = `<tr><td colspan="4" style="text-align:center;padding:1.5rem;color:var(--text-secondary);">No subject examination marks recorded yet.</td></tr>`;
            }

            container.innerHTML = `
                <div class="marksheet-card">
                    <div class="marksheet-header">
                        <h2>Madhav Institute of Technology & Science, Gwalior</h2>
                        <p style="font-weight:500;color:var(--primary);margin-top:4px;">(A Govt. Aided UGC Autonomous Institute Affiliated to RGPV, Bhopal)</p>
                        <p style="font-size:0.85rem;margin-top:2px;">Academic Session 2026–27 | Examination Grade Card</p>
                    </div>

                    <div class="marksheet-details">
                        <div>
                            <p style="font-size:0.8rem;text-transform:uppercase;color:var(--text-secondary);">Student Name</p>
                            <h4 style="color:var(--text-main);">${App.escapeHTML(s.name || 'N/A')}</h4>
                        </div>
                        <div>
                            <p style="font-size:0.8rem;text-transform:uppercase;color:var(--text-secondary);">Enrollment / Roll No</p>
                            <h4 style="color:var(--text-main);">${App.escapeHTML(s.roll_no || 'N/A')}</h4>
                        </div>
                        <div>
                            <p style="font-size:0.8rem;text-transform:uppercase;color:var(--text-secondary);">Student ID</p>
                            <p style="font-weight:600;color:var(--text-main);">${App.escapeHTML(s.student_id || 'N/A')}</p>
                        </div>
                        <div>
                            <p style="font-size:0.8rem;text-transform:uppercase;color:var(--text-secondary);">Class / Semester</p>
                            <p style="font-weight:600;color:var(--text-main);">${App.escapeHTML(s.class || 'N/A')}</p>
                        </div>
                    </div>

                    <table class="data-table" style="margin-bottom:1.5rem;border:1px solid var(--border-light);">
                        <thead>
                            <tr>
                                <th>Subject Title</th>
                                <th style="text-align:center;">Maximum Marks</th>
                                <th style="text-align:center;">Marks Obtained</th>
                                <th style="text-align:center;">Percentage</th>
                            </tr>
                        </thead>
                        <tbody>
                            ${subjectRows}
                        </tbody>
                    </table>

                    ${data.has_result ? `
                        <div class="marksheet-summary-box">
                            <div>
                                <p style="font-size:0.75rem;text-transform:uppercase;color:var(--text-secondary);font-weight:600;">Total Aggregate</p>
                                <h3 style="color:var(--primary);">${r.total_marks} / ${r.maximum_total_marks}</h3>
                            </div>
                            <div>
                                <p style="font-size:0.75rem;text-transform:uppercase;color:var(--text-secondary);font-weight:600;">Percentage</p>
                                <h3 style="color:var(--secondary);">${r.percentage.toFixed(2)}%</h3>
                            </div>
                            <div>
                                <p style="font-size:0.75rem;text-transform:uppercase;color:var(--text-secondary);font-weight:600;">Grade</p>
                                <div style="margin-top:4px;">${App.formatGradeBadge(r.grade)}</div>
                            </div>
                            <div>
                                <p style="font-size:0.75rem;text-transform:uppercase;color:var(--text-secondary);font-weight:600;">Academic Status</p>
                                <div style="margin-top:4px;">${App.formatStatusBadge(r.result_status)}</div>
                            </div>
                        </div>
                    ` : `
                        <div style="padding:1rem;background:#fefce8;color:#854d0e;border-radius:var(--radius-sm);text-align:center;">
                            Result has not yet been computed. Please record marks for all registered subjects.
                        </div>
                    `}

                    <div style="margin-top:2rem;display:flex;justify-content:space-between;align-items:flex-end;font-size:0.75rem;color:var(--text-secondary);border-top:1px dashed var(--border-light);padding-top:1rem;">
                        <div>
                            <p>Calculated by C++ Result Engine</p>
                            <p>MITS Gwalior Academic Management System</p>
                        </div>
                        <div style="text-align:right;">
                            <p>Controller of Examinations</p>
                            <p>MITS Gwalior (M.P.)</p>
                        </div>
                    </div>
                </div>
            `;

            App.openModal('marksheet-modal');
        } catch (err) {
            App.showToast(err.message, 'error');
        }
    }

    return {
        loadResults,
        triggerRecalculateAll,
        viewStudentMarksheet
    };
})();
