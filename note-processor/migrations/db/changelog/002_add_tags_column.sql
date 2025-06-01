--changeset danil:2
ALTER TABLE notes ADD COLUMN tags VARCHAR(255);
--rollback empty
