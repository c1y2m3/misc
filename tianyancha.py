#! /usr/bin/env python3
# -*- coding:UTF-8 -*-

import json
import requests
import urllib
import re

import sys
reload(sys)
sys.setdefaultencoding('utf8')


class expolit():
    def __init__(self):
        self.companys = []
        self.chinanme = 'xxxx'
        self.companys.append(self.chinanme)
        self.chinazurl = 'https://data.chinaz.com/company/t-0/p-0/c-0/i-0/d-0/s-{}'
        self.domain = 'https://data.chinaz.com{}'
        self.Foreignurl = 'https://open.api.tianyancha.com/services/open/ic/inverst/2.0?name={}'.format(
            urllib.quote(self.chinanme))
        self.branchurl = 'https://open.api.tianyancha.com/services/open/ic/branch/2.0?name={}'.format(
            urllib.quote(self.chinanme))
        self.WebSiteurl = 'https://open.api.tianyancha.com/services/open/ipr/icp/2.0?name={name}'
        self.key = '7beae071-f80d-466e-b757-7f79d37027fe'
        self.headers = {
            'Accept': 'application/json, text/plain, */*',
            'Cache-Control': 'max-age=0',
            'Authorization': self.key,
            'Accept-Encoding': 'gzip, deflate,',
            'Accept-Language': 'zh-CN,zh;q=0.8'
        }

    def http_request(self, url):
        try:
            # proxies = {
            #     "http": "http://127.0.0.1:8081",
            #     "https": "http://127.0.0.1:8081",
            # }
            requests.packages.urllib3.disable_warnings()
            response = requests.get(url,
                                    verify=False,
                                    timeout=3.5,
                                    headers=self.headers)
            # response.encoding = response.apparent_encoding
            return response.text
        except:
            pass

    def exatin(self, string):
        try:
            rs_dict = json.loads(string)
            if rs_dict['error_code'] == 300000:
                pass
            else:
                names = (rs_dict['result']['items'])
                for name in names:
                    self.companys.append(name['name'])
        except:
            pass

    def foreign(self):

        html = self.http_request(self.Foreignurl)
        self.exatin(html)

    def branch(self):
        html = self.http_request(self.branchurl)
        self.exatin(html)

    def WebSite(self):
        self.foreign()
        # self.branch()
        save = {}
        save_1 = {}
        comman =[]
        with open('company.list', 'w+') as file:
            for company in (self.companys):
                print(company)
                file.write(company + "\n")
                name = urllib.quote_plus(company.encode('utf8'))
                html = self.http_request(self.WebSiteurl.format(name=name))
                uir = self.chinazurl.format(name)
                request = self.http_request(uir)
                # print(request)
                cids = re.findall('<a href="(.*?)"><li class="w70 blue2 arow">',(request))
                temp = json.loads(html)
                try:
                    weburls = (temp['result']['items'][0])
                    save['url'] = weburls['webSite']
                    save['name'] = weburls['companyName']
                    print(json.dumps(save, ensure_ascii=False, indent=4))
                except:
                    pass
                for cid in cids:
                    response = self.http_request(self.domain.format(cid))
                    qrcode = re.findall('<a href="http://seo.chinaz.com/(.*?)">',str(response))
                    try:
                        if qrcode is not None:
                            comman.append(qrcode[0])
                    except:
                        pass

        save_1['url_1'] = list(set(comman))
        print(json.dumps(save_1, ensure_ascii=False, indent=4))


if __name__ == '__main__':
    exp = expolit()
    exp.WebSite()
