// ============================================================================
// SUPABASE AUTHENTICATION & SESSION MANAGEMENT
// Student Result Management System - MITS Gwalior
// ============================================================================

const Auth = (() => {
    // Keys in localStorage
    const TOKEN_KEY = 'srms_auth_token';
    const USER_KEY = 'srms_auth_user';
    const SUPABASE_URL_KEY = 'srms_supabase_url';
    const SUPABASE_KEY_KEY = 'srms_supabase_anon_key';

    let supabaseClient = null;

    /**
     * Initialize Supabase Client if credentials are provided
     */
    function initSupabase() {
        const url = localStorage.getItem(SUPABASE_URL_KEY);
        const anonKey = localStorage.getItem(SUPABASE_KEY_KEY);

        if (url && anonKey && window.supabase) {
            try {
                supabaseClient = window.supabase.createClient(url, anonKey);
                console.log('[Supabase] Initialized client for:', url);
            } catch (e) {
                console.warn('[Supabase] Init error:', e);
            }
        }
    }

    function getToken() {
        return localStorage.getItem(TOKEN_KEY) || '';
    }

    function getUser() {
        try {
            const raw = localStorage.getItem(USER_KEY);
            return raw ? JSON.parse(raw) : null;
        } catch {
            return null;
        }
    }

    function isAuthenticated() {
        return !!getToken();
    }

    /**
     * Standard Login via Supabase Auth or Demo Role Credentials
     */
    async function login(email, password, roleHint = '') {
        initSupabase();

        // 1. Try Supabase Auth if client is configured
        if (supabaseClient) {
            try {
                const { data, error } = await supabaseClient.auth.signInWithPassword({
                    email,
                    password
                });

                if (error) {
                    throw error;
                }

                if (data && data.session) {
                    const token = data.session.access_token;
                    localStorage.setItem(TOKEN_KEY, token);

                    // Fetch user role and profile from C++ backend
                    const meData = await App.apiFetch('/api/auth/me', {
                        headers: { 'Authorization': `Bearer ${token}` }
                    });

                    const userProfile = {
                        id: data.user.id,
                        email: data.user.email,
                        role: meData.user ? meData.user.role : (roleHint || 'student'),
                        name: meData.user ? meData.user.name : data.user.email
                    };

                    localStorage.setItem(USER_KEY, JSON.stringify(userProfile));
                    return userProfile;
                }
            } catch (supabaseErr) {
                console.warn('[Supabase Auth Warning] Fallback to direct verification:', supabaseErr.message);
                // If live Supabase authentication fails, continue to check if demo credentials match
            }
        }

        // 2. Demo Account & Local Verification Mode
        let matchedRole = '';
        let userName = '';
        let token = '';

        if (email.includes('admin') || roleHint === 'admin') {
            matchedRole = 'admin';
            userName = 'System Administrator';
            token = 'demo-admin-token';
        } else if (email.includes('teacher') || roleHint === 'teacher') {
            matchedRole = 'teacher';
            userName = 'Dr. Rajesh Kumar Sharma';
            token = 'demo-teacher-token';
        } else if (email.includes('student') || roleHint === 'student') {
            matchedRole = 'student';
            userName = 'Aarav Sharma';
            token = 'demo-student-token';
        } else {
            throw new Error('Invalid credentials. Please enter a valid email or select a demo role.');
        }

        localStorage.setItem(TOKEN_KEY, token);
        const userProfile = {
            id: 'auth-' + matchedRole + '-001',
            email: email,
            role: matchedRole,
            name: userName
        };
        localStorage.setItem(USER_KEY, JSON.stringify(userProfile));
        return userProfile;
    }

    /**
     * Redirect to the appropriate dashboard based on user role
     */
    function redirectToDashboard(role) {
        if (role === 'admin') {
            window.location.href = 'admin.html';
        } else if (role === 'teacher') {
            window.location.href = 'teacher.html';
        } else if (role === 'student') {
            window.location.href = 'student.html';
        } else {
            window.location.href = 'login.html';
        }
    }

    /**
     * Enforce role-based access control on protected pages
     */
    function requireRole(allowedRoles) {
        const user = getUser();
        const token = getToken();

        if (!token || !user) {
            window.location.href = 'login.html';
            return false;
        }

        const roles = Array.isArray(allowedRoles) ? allowedRoles : [allowedRoles];
        if (!roles.includes(user.role)) {
            alert(`Access restricted to [${roles.join(', ')}] role. You are signed in as '${user.role}'.`);
            redirectToDashboard(user.role);
            return false;
        }

        return true;
    }

    /**
     * User Logout
     */
    async function logout() {
        if (supabaseClient) {
            try {
                await supabaseClient.auth.signOut();
            } catch (e) {
                console.warn(e);
            }
        }
        localStorage.removeItem(TOKEN_KEY);
        localStorage.removeItem(USER_KEY);
        window.location.href = 'login.html';
    }

    return {
        initSupabase,
        getToken,
        getUser,
        isAuthenticated,
        login,
        logout,
        requireRole,
        redirectToDashboard
    };
})();
