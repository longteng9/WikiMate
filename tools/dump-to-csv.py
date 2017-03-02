import os
import re
import codecs
import xml.etree.ElementTree as ET

class ImportData:
    def __init__(self, index_filename, page_dir):
        self.index_filename = index_filename
        self.page_dir = page_dir
        self.total_count = 0
        self.zh_count = 0
        self.start_convert()
        print("total count:", self.total_count)
        print("chinese count:", self.zh_count)
        
    def start_convert(self):
        try:
            file_index = codecs.open(self.index_filename, 'r', 'utf-8')
            file_csv = codecs.open("import.csv", 'a', 'utf-8')
            page_id = ''
            page_title = ''
            redirection = ''
            en_entries = ''
            tag = ''
            count = 0
            for line in file_index:
                tmp = line.split(':')
                if len(tmp) != 2:
                    continue
                page_id = tmp[0]
                page_title = tmp[1].strip()
                (redirection, en_entries, tag) = self.extract_info(os.path.join(self.page_dir, page_id+".wiki"))
                if page_title:
                    page_title = "\"" + page_title + "\""
                if redirection:
                    redirection = "\"" + redirection + "\""
                if en_entries:
                    if en_entries.endswith(','):
                        en_entries = en_entries[0: -1]
                    if en_entries.startswith(','):
                        en_entries = en_entries[1:]
                    en_entries = en_entries.strip()
                    en_entries = "\"" + en_entries + "\""
                if tag:
                    tag = "\"" + tag + "\""
                file_csv.write(page_id + "," + page_title + "," + redirection + "," + en_entries + "," + tag + "\n")
                count += 1
            file_index.close()
            file_csv.close()
            print("import ", count, " words into csv")
        except Exception as e:
            print("failed to import data to csv: ", e)
            file_index.close()
            return
            
    def extract_info(self, page_path):
        self.total_count += 1
        redirection = ""
        en_entries = ""
        tag = ""
        (public, zh_section) = self.split_section(page_path)
        if "{{also|" in public:
            pos = public.index("{{also|") + 7
            end = public.index("}}", pos)
            redirection += public[pos: end]
            
        if zh_section:
            tag = "zh"
            if "{{zh-see|" in zh_section:
                pos = zh_section.index("{{zh-see|") + 9
                end = zh_section.index("}}", pos)
                if redirection:
                    redirection += "|" + zh_section[pos: end]
                else:
                    redirection += zh_section[pos: end]
            
            if redirection:
                tmp = list(set(redirection.split("|")))
                redirection = "|".join(tmp)
            
            for line in zh_section.splitlines():
                if not line.startswith("# "):
                    continue
                line = line.replace("# ", "")
                line = self.erase_desc(line)
                line = line.replace("[[", "").replace("]]", "").replace("'''", "")
                tmp = line.split(";")
                for j in range(0, len(tmp)):
                    tmp[j] = tmp[j].strip()
                en_entries += "|".join(tmp) + "|"
            if en_entries.endswith("|"):
                en_entries = en_entries[0: -1]
                while "||" in en_entries:
                    en_entries = en_entries.replace("||", "|")
            self.zh_count += 1
        else:
            tag = "ja"
        return redirection, en_entries, tag
        
    def split_section(self, page_path):
        file = codecs.open(page_path, 'r', 'utf-8')
        public = ""
        buff = ""
        recording = False
        isPublic = True
        for line in file:
            if isPublic:
                public += line.strip() + "\n"
            if line.strip().startswith("=="):
                isPublic = False
                
            if recording and line.startswith("==") and not line.startswith("==="):
                recording = False
            if line.strip().startswith("==Chinese=="):
                recording = True
            if recording:
                buff += line.strip() + "\n"
        file.close()
        return public, buff
        
    def erase_desc(self, line):
        start = -1
        L = len(line)
        for i in range(0, L-1):
            if line[i] == '{' and line[i + 1] == '{':
                start = i
                continue
            if line[i] == '}' and line[i + 1] == '}':
                if start == -1:
                    return line
                end = i + 1
                line = line[0: start] + line[end+1:]
                break
        if start == -1:
            return line
        else:
            return self.erase_desc(line)

if __name__ == "__main__":
    importData = ImportData("index_zh.txt", "pages_zh")
    