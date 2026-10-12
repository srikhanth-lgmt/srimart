-- Enforce the same case-insensitive email identity used by the auth service.
CREATE UNIQUE INDEX IF NOT EXISTS idx_users_email_normalized ON users(lower(email));
