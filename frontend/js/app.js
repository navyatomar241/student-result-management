// ============================================================================
// STUDENT RESULT MANAGEMENT SYSTEM - CORE APPLICATION LOGIC
// Institution: Madhav Institute of Technology & Science, Gwalior
// Shared API Helper, UI Utilities, Modal & Toast Notifications
// ============================================================================

const App = (() => {
    // API Base URL (C++ Crow Backend default port 18080)
    const API_BASE = localStorage.getItem('API_BASE_URL') || 'http://127.0.0.1:18080';

    /**
     * Authenticated API Fetch Wrapper
     */
    async function apiFetch(endpoint, options = {}) {
        const token = Auth.getToken();
        const headers = {
            'Content-Type': 'application/json',
            ...(token ? { 'Authorization': `Bearer ${token}` } : {}),
            ...(options.headers || {})
        };

        const config = {
            ...options,
            headers
        };

        try {
            const url = endpoint.startsWith('http') ? endpoint : `${API_BASE}${endpoint}`;
            const response = await fetch(url, config);
            
            // Check for authentication error
            if (response.status === 401) {
                showToast('Session expired or unauthorized. Please login.', 'error');
                setTimeout(() => {
                    Auth.logout();
                }, 1500);
                throw new Error('Unauthorized');
            }

            const data = await response.json().catch(() => ({}));

            if (!response.ok) {
                const msg = data.error || `Request failed with status ${response.status}`;
                throw new Error(msg);
            }

            return data;
        } catch (err) {
            console.error(`[API Error] ${endpoint}:`, err);
            throw err;
        }
    }

    /**
     * Toast Notification System
     */
    function showToast(message, type = 'info') {
        let container = document.getElementById('toast-container');
        if (!container) {
            container = document.createElement('div');
            container.id = 'toast-container';
            document.body.appendChild(container);
        }

        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.innerHTML = `
            <span>${escapeHTML(message)}</span>
            <button style="background:none;border:none;color:white;cursor:pointer;margin-left:12px;" onclick="this.parentElement.remove()">✕</button>
        `;

        container.appendChild(toast);
        setTimeout(() => {
            if (toast.parentElement) toast.remove();
        }, 4000);
    }

    /**
     * Modal Helper Functions
     */
    function openModal(modalId) {
        const el = document.getElementById(modalId);
        if (el) el.classList.add('active');
    }

    function closeModal(modalId) {
        const el = document.getElementById(modalId);
        if (el) el.classList.remove('active');
    }

    /**
     * Formatters for Badges
     */
    function formatGradeBadge(grade) {
        const g = (grade || '').toUpperCase();
        let cls = 'badge-grade-d';
        if (g === 'A+') cls = 'badge-grade-aplus';
        else if (g === 'A') cls = 'badge-grade-a';
        else if (g === 'B') cls = 'badge-grade-b';
        else if (g === 'C') cls = 'badge-grade-c';
        else if (g === 'FAIL') cls = 'badge-fail';
        return `<span class="badge ${cls}">${escapeHTML(grade || 'N/A')}</span>`;
    }

    function formatStatusBadge(status) {
        const isPass = (status || '').toLowerCase() === 'pass';
        const cls = isPass ? 'badge-pass' : 'badge-fail';
        return `<span class="badge ${cls}">${isPass ? 'PASS' : 'FAIL'}</span>`;
    }

    function escapeHTML(str) {
        if (!str) return '';
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#39;');
    }

    /**
     * Initialize Mobile Sidebar & User UI
     */
    function initShell() {
        const mobileBtn = document.getElementById('btn-mobile-menu');
        const sidebar = document.querySelector('.sidebar');
        if (mobileBtn && sidebar) {
            mobileBtn.addEventListener('click', () => {
                sidebar.classList.toggle('mobile-open');
            });
        }

        // Display logged in user in sidebar
        const user = Auth.getUser();
        if (user) {
            const nameEl = document.getElementById('sidebar-user-name');
            const roleEl = document.getElementById('sidebar-user-role');
            const avatarEl = document.getElementById('sidebar-user-avatar');
            if (nameEl) nameEl.textContent = user.name || user.email || 'User';
            if (roleEl) roleEl.textContent = user.role || 'Role';
            if (avatarEl) {
                const initial = (user.name || user.email || 'U').charAt(0).toUpperCase();
                avatarEl.textContent = initial;
            }
        }
    }

    return {
        API_BASE,
        apiFetch,
        showToast,
        openModal,
        closeModal,
        formatGradeBadge,
        formatStatusBadge,
        escapeHTML,
        initShell
    };
})();
