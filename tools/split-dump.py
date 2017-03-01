import os
import re
import codecs
import xml.etree.ElementTree as ET

class XMLProvider:
    def __init__(self, filename):
        if not self.filter(filename):
            return
        self.filename = filename + ".zh"
        self.zh_page_count = 0
        self.tree = ET.parse(self.filename)
        self.parse()
        
    def filter(self, filename):
        zh_filename = filename + ".zh"

        count = 0
        recording = False
        buff = ""
        try:
            file1 = codecs.open(filename, 'r', 'utf-8')
            file2 = codecs.open(zh_filename, 'a', 'utf-8')
            file2.write("<pages>\n")
            for line in file1:
                if "<page>" in line:
                    recording = True
                    buff = ""
                    buff += line.strip() + "\n"
                elif recording:
                    buff += line.strip() + "\n"
                    if "</page>" in line:
                        recording = False
                        count += 1
                        file2.write(buff)
                    elif "<title>" in line and "</title>" in line:
                        title = line[line.index("<title>")+7: line.index("</title>")]
                        if not self.is_zh(title):
                            buff = ""
                            recording = False
            file2.write("</pages>")
            print("zh page count:", count)
            file1.close()
            file2.close()
        except Exception as e:
            print("failed to filter zh pages:\n", e)
            return False
        
        print("succeed to filter zh pages")
        return True
        
    def parse(self):
        failures = 0
        root = self.tree.getroot()
        id = ''
        title = ''
        text = ''
        for page in root.iter('page'):
            try:
                id = page.find('id').text
                title = page.find('title').text
                text = page.find('revision').find('text').text
            except Exception as e:
                failures += 1
                print('failed to parse count: ', failures)
                continue
            try:
                self.zh_page_count += 1
                file_index = codecs.open('zh-index.txt', 'a', 'utf-8')
                file_page = codecs.open('zh-pages/'+ id + '.wiki', 'w', 'utf-8')
                file_index.write(id + ':' + title + '\n')
                file_index.close()
                file_page.write(text)
                file_page.close()
            except Exception as e:
                self.zh_page_count -= 1
                print('failed to write file:\n', e)
                continue
        print('zh page count:', self.zh_page_count)
        
    def is_zh(self, text):
        for ch in text.strip('\r\n'):
            if u'\u4e00' > ch or ch > u'\u9fff':
                return False
        return True


if __name__ == '__main__':
    provider = XMLProvider("enwiktionary-20170101-pages-articles-multistream.xml")
