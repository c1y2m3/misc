#!/usr/bin/python2
import winrm as pywinrm
import argparse


parse = argparse.ArgumentParser(description="WinRM Shell")
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
        conn = pywinrm.Session(
            self.hostname,
            auth=(
                '{}\\{}'.format(
                    self.domain,
                    self.username),
                self.password),
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