# -*- coding: utf-8 -*-
import sys
import os
import json
import hashlib
import codecs
import urllib
import urllib.request
import jieba
import jieba.analyse
import jieba.posseg

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
    result = json.loads(response.read().decode('utf-8'))
    if result['errorCode'] == 0:
        if 'basic' in result:
            if 'explains' in result['basic']:
                return result['basic']['explains']
        else:
            return result['translation']
    return []
    
def query_online(word):
    result = query_youdao(word)
    if len(result) == 0:
        return query_baidu(word)
    return result
    
    
if __name__ == '__main__':
    '''l = phrase_parse(u"工信处女干事每月经过下属科室都要亲口交代24口交换机等技术性器件的安装工作")
    for w in l:
        print(w.word, ' ', w.flag)'''
    f = codecs.open('test.txt', 'w', 'utf-8')
    f.write(str(query_online(u'美好')))
    f.close()

