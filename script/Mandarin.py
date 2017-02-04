# -*- coding: utf-8 -*-
import sys
import os
import json
import hashlib
import codecs
import time
import urllib
import urllib.request
import jieba
import jieba.analyse
import jieba.posseg
import xml.etree.cElementTree as et


def load_dict(path):
    jieba.load_userdict(path.encode('utf-8'))

def phrase_parse(text):
    seg_list = jieba.posseg.cut(text.encode('utf-8'))
    return seg_list
    
def query_baidu(word):
    appid = '20170118000036209'
    secret_key = 'koAkqialgJjn8SP6bLn7'
    url = 'http://api.fanyi.baidu.com/api/trans/vip/translate'
    q=word
    fromLang = 'zh'
    toLang = 'en'
    salt = '65536'
    
    sign = appid + q + str(salt) + secret_key
    m1 = hashlib.md5()
    m1.update(sign.encode('utf-8'))
    sign = m1.hexdigest()
    
    url = url + '?appid=' + appid + '&q=' + urllib.parse.quote(q) + '&from=' + fromLang \
        + '&to=' + toLang + '&salt=' + salt + '&sign=' + sign
    
    result = urllib.request.urlopen(url).read().decode('utf-8')
    result = json.loads(result)

    return [result['trans_result'][0]['dst']]
    
def query_youdao(word):
    url = 'http://fanyi.youdao.com/openapi.do'
    data = {
        'keyfrom': 'wikimate',
        'key': '1196728651',
        'type': 'data',
        'doctype': 'json',
        'version': 1.1,
        'q': word
    }
    data = urllib.parse.urlencode(data)
    url += ('?' + data)
    req = urllib.request.Request(url)
    response = urllib.request.urlopen(req)
    result = None
    try:
        result = json.loads(response.read().decode('utf-8'))
    except:
        return []
    if result['errorCode'] == 0:
        if 'basic' in result:
            if 'explains' in result['basic']:
                return result['basic']['explains']
        elif 'translation' in result:
            if result['translation'][0] == word:
                return []
            else:
                return result['translation']
    return []
    
def query_online(word):
    result = query_youdao(word)
    if len(result) == 0:
        return query_baidu(word)
    return result
    
def query_wiki(word):
    url = 'https://en.wiktionary.org/w/api.php'
    data = {
        'format': 'xml',
        'action': 'query',
        'rvprop': 'content',
        'prop': 'revisions',
        'redirects': '1',
        'titles': word
    }
    data = urllib.parse.urlencode(data)
    url += ('?' + data)
    print(url)
    req = urllib.request.Request(url)
    response = urllib.request.urlopen(req)
    
    xml_string = response.read().decode('utf-8')
    print(xml_string)
    root = et.fromstring(xml_string) 
    pages = root.find('query').find('pages')
    text_map = {}
    for page in pages.findall('page'):
        try:
            idx = int(page.get('_idx'))
            if idx == -1:
                print('idx == -1')
                text_map[word] = ''
                continue
            revisions = page.find('revisions')
            if revisions == None:
                print('revisions == None')
                text_map[word] = ''
                continue
            rev = revisions.find('rev')
            if rev == None:
                print('rev == None')
                text_map[word] = ''
                continue
            text_map[word] = rev.text
        except:
            text_map[word] = ''
    if text_map[word]:
        return parse_wikitext(text_map[word])
    return []
    
def parse_wikitext(text):
    return text
        
    
if __name__ == '__main__':
    f = codecs.open('zh_index.txt', 'r', 'utf-8')
    empty = 0
    collision = 0
    errs = 0
    valid = 0
    empty_list = []
    collision_list = []
    except_list = []
    trans_map = {}
    start = time.time()
    try:
        for line in f:
            line = line.strip()
            data = line.split(':')
            id = data[0]
            word = data[1]
            trans = []
            try:
                trans = query_online(word)
            except Exception as e:
                errs += 1
                except_list.append(word)
                print('exception on ', word)
                print(e)
            if len(trans) == 0:
                empty += 1
                empty_list.append(word)
            else:
                if word in trans_map:
                    collision += 1
                    collision_list.append(word)
                    valid -= 1
                trans_map[word] = trans
                valid += 1
            if valid % 100 == 0:
                print('valid:', valid)
                print('empty:', empty)
                print('except:', errs)
                print('collision:', collision)
                print('\n')
            if valid % 1000 == 0:
                ff = codecs.open('zh_cachedtrans.txt', 'w', 'utf-8')
                json.dump(trans_map, ff, ensure_ascii=False, indent=4)
                ff.close()
                print('dump at ', valid)
        f.close()
        print('time cost:', time.time() - start)
        print('empty count:', empty)
        print('collision count:', collision)
        print('exception account:', errs)
        print('valid count:', valid)
        
        f = codecs.open('zh_cachedtrans.txt', 'w', 'utf-8')
        json.dump(trans_map, f, ensure_ascii=False, indent=4)
        f.close()
        
        if empty > 0:
            f = codecs.open('zh_empty.txt', 'w', 'utf-8')
            f.write('\n'.join(empty_list))
            f.close()
        if collision > 0:
            f = codecs.open('zh_collision.txt', 'w', 'utf-8')
            f.write('\n'.join(collision_list))
            f.close()
        if err > 0:
            f = codecs.open('zh_except.txt', 'w', 'utf-8')
            f.write('\n'.join(except_list))
            f.close()
    except Exception as e:
        print('except:', e)
        print('time cost:', time.time() - start)
        print('empty count:', empty)
        print('collision count:', collision)
        print('exception account:', errs)
        print('valid count:', valid)
        f = codecs.open('zh_cachedtrans.txt', 'w', 'utf-8')
        json.dump(trans_map, f, ensure_ascii=False, indent=4)
        f.close()
