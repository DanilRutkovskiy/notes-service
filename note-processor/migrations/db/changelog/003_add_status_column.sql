--changeset danil:3
ALTER TABLE notes ADD COLUMN status VARCHAR(50) DEFAULT 'active';
--rollback empty
