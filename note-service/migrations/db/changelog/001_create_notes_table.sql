--changeset danil:1
CREATE TABLE notes 
(
    id UUID PRIMARY KEY,
    user_id VARCHAR(50) NOT NULL,
    title VARCHAR(255) NOT NULL,
    content TEXT NOT NULL
);
--rollback empty
