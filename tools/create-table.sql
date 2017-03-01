CREATE TABLE IF NOT EXISTS page_zh(
    page_id INTEGER PRIMARY KEY NOT NULL,
    page_title TEXT NOT NULL,
    redirection TEXT DEFAULT(""),
    en_entries TEXT DEFAULT(""),
    tag TEXT DEFAULT("")
);
CREATE UNIQUE INDEX IF NOT EXISTS page_title_index ON page_zh(page_title);

