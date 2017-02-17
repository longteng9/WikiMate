# -*- coding: utf-8 -*-
import os
import codecs
import json
import hashlib
import time
import urllib
import urllib.request
import jieba
import jieba.analyse
import jieba.posseg

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
    
if __name__ == '__main__':
    file_in = None
    file_out = None
    try:
        file_in = codecs.open('mandarin_parse_in.txt', 'r', 'utf-8')
        file_out = codecs.open('mandarin_parse_out.txt', 'w', 'utf-8')
    except Exception as e:
        print('failed to open file: \n', e)
    # jieba.load_userdict('trans_mem.txt')
    
    if file_in and file_out:
        for line in file_in:
            lst = jieba.cut(line) # jieba.posseg.cut(line)
            for word in lst:
                file_out.write(word + '/#/')
            file_out.write('\n')
        file_in.close()
        file_out.close()
        print('finished')
