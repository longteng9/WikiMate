import os
import re
import codecs
import xml.etree.ElementTree as ET

class XMLProvider:
    def __init__(self, filename):
        self.filename = filename
        self.tree = ET.parse(filename)
        self.zh_page_count = 0
        
        self.parse()
        
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
                if self.is_zh(title):
                    self.zh_page_count += 1
                    file_index = codecs.open('zh-index.txt', 'a', 'utf-8')
                    file_page = codecs.open('zh-pages/'+ id + '.wiki', 'w')
                    file_index.write(id + ':' + title + '\n')
                    file_index.close()
                    file_page.write(text)
                    file_page.close()
            except Exception as e:
                self.zh_page_count -= 1
                print('failed to write file')
                continue
        print('zh page count:', self.zh_page_count)
        
    def is_zh(text):
        for ch in text.strip('\r\n'):
            if u'\u4e00' > ch or ch > u'\u9fff':
                return False
        return True


if __name__ == '__main__':
    provider = XMLProvider("enwiktionary-20170101-pages-articles-multistream.xml")
