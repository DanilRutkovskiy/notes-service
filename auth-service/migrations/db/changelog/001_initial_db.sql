--changeset danil:1
CREATE EXTENSION IF NOT EXISTS citext;
--rollback DROP EXTENSION citext CASCADE;

--changeset danil:2
CREATE TABLE users 
(
    id UUID PRIMARY KEY,
    email CITEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL,
    is_active BOOL NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
--rollback DROP TABLE users;

--changeset danil:3
CREATE TABLE refresh_tokens 
(
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL,
    token_hash TEXT NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    revoked_at TIMESTAMP,
    user_agent TEXT,
    ip INET,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);
--rollback DROP TABLE refresh_tokens;
