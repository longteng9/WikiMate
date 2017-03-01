import os
import codecs
import sqlite3
import shlex

class CsvToDB:
    def __init__(self, csv_filename):
        self.csv_filename = csv_filename
        self.db = sqlite3.connect("zh_pages.db")
        self.create_table()
        self.insert_sql()
        self.db.close()
        
    def create_table(self):
        self.db.execute("CREATE TABLE IF NOT EXISTS page_zh("
            "page_id INTEGER PRIMARY KEY NOT NULL,"
            "page_title TEXT NOT NULL,"
            "redirection TEXT,"
            "en_entries TEXT,"
            "tag TEXT)")
        self.db.execute("CREATE UNIQUE INDEX IF NOT EXISTS page_title_index ON page_zh(page_title)")
        
    def insert_sql(self):
        file_csv = codecs.open(self.csv_filename, 'r', 'utf-8')
        for line in file_csv:
            lexer = shlex.shlex(line)
            lexer.quotes = '"'
            lexer.whitespace=','
            lexer.whitesapce_split=True
            
            tmp = list(lexer)
            if len(tmp) != 5:
                print("wrong line:", line)
                print(tmp)
            self.db.execute("INSERT INTO page_zh(page_id, page_title, redirection, en_entries, tag) VALUES(" + tmp[0] + "," + tmp[1] + "," + tmp[2] +"," + tmp[3]+ "," + tmp[4] + ");")
        file_csv.close()
        
if __name__ == "__main__":
    csvToDB = CsvToDB("import.csv")
