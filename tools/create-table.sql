CREATE TABLE IF NOT EXISTS pages_zh(
    page_id INTEGER PRIMARY KEY NOT NULL,
    page_title TEXT NOT NULL,
    redirection TEXT DEFAULT NULL,
    en_entries TEXT DEFAULT NULL,
    tag TEXT DEFAULT NULL);
CREATE UNIQUE INDEX IF NOT EXISTS page_title_index ON page_zh(page_title);

