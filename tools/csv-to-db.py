import os
import codecs
import sqlite3
import shlex

class CsvToDB:
    def __init__(self, csv_filename):
        self.csv_filename = csv_filename
        self.db = sqlite3.connect("pages_zh.db")
        self.count = 0
        self.create_table()
        self.insert_sql()
        self.db.close()
        print("insert ", self.count, " rows")
        
    def create_table(self):
        self.db.execute("CREATE TABLE IF NOT EXISTS pages_zh("
            "page_id INTEGER PRIMARY KEY NOT NULL,"
            "page_title TEXT NOT NULL,"
            "redirection TEXT DEFAULT NULL,"
            "en_entries TEXT DEFAULT NULL,"
            "tag TEXT DEFAULT NULL)")
        self.db.execute("CREATE UNIQUE INDEX IF NOT EXISTS page_title_index ON pages_zh(page_title)")
        
    def insert_sql(self):
        file_csv = codecs.open(self.csv_filename, 'r', 'utf-8')
        for line in file_csv:
            '''lexer = shlex.shlex(line.strip(),posix=True)
            lexer.quotes = '"'
            lexer.whitespace=','
            lexer.whitesapce_split=True'''
            tmp = self.split5(line.strip(), ",")
            if len(tmp) != 5:
                print("failed on:", line, "\n", tmp)
                return
            self.db.execute("INSERT INTO pages_zh VALUES(?,?,?,?,?)", tuple(tmp))
            self.count += 1
        self.db.commit()
        file_csv.close()
        
    def split5(self, str, delm):
        result = []
        tmp = str.split(delm)
        if(len(tmp)) >= 5:
            result.extend(tmp[0:3])
            result.append(",".join(tmp[3:-1]))
            result.append(tmp[-1])
        else:
            print("failed on parsing:", str)
            
        for i in range(0, 5):
            if result[i].startswith('"'):
                result[i] = result[i][1:]
            if result[i].endswith('"'):
                result[i] = result[i][0:-1]
        return result
        
if __name__ == "__main__":
    csvToDB = CsvToDB("import.csv")
