# !/usr/bin/env python
# coding:utf-8

import _winreg
import getpass
import subprocess
import os

key = r"Software\Microsoft\Terminal Server Client\Servers"
open_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, key)
countkey = _winreg.QueryInfoKey(open_key)[0]
values = {}
K = 0

def ListLogged_inUsers():
    User = getpass.getuser()
    try:
        for i in range(int(countkey)):
            db = _winreg.EnumKey(open_key, i)  # 获取子键名
            servers = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, key + "\\" + db)
            name, value, type = _winreg.EnumValue(servers, K)
            values['current_user'] = User
            values['Server'] = db
            values['UsernameHint'] = value
            print values
        _winreg.CloseKey(open_key)
    except WindowsError:
        print "No RDP Connections History"

# ListAllUsers RDP Connections History
def AllUser():

    cmd = "$AllUser = Get-WmiObject -Class Win32_UserAccount;" \
          "foreach($User in $AllUser)" \
          "{Write-Host $User.Caption};"

    cmder = "$AllUser = Get-WmiObject -Class Win32_UserAccount;" \
        "foreach($User in $AllUser)" \
        "{Write-Host $User.SID};"

    arg = [r"powershell.exe", cmd]
    ps = subprocess.Popen(arg, stdout=subprocess.PIPE)
    dc = ps.stdout.read()
    dc = dc.split('\n')

    args = [r"powershell.exe", cmder, ]
    p = subprocess.Popen(args, stdout=subprocess.PIPE)
    dt = p.stdout.read()
    sid = dt.split('\n')

    num = {}
    for y in range(len(dc)):
        num[dc[y]] = sid[y]

    for id in sid:
        try:
            dba = _winreg.OpenKey(_winreg.HKEY_USERS, id + "\\" + key)
            count = _winreg.QueryInfoKey(dba)[0]
            for s in range(int(count)):
                dbs = _winreg.EnumKey(dba, s)
                server = _winreg.OpenKey(
                    _winreg.HKEY_USERS,
                    id + os.sep + key + "\\" + dbs)
                name, value, type = _winreg.EnumValue(server, K)
                for www in num:
                    if num[www] == id:
                        # print "User:" + www
                        # print "SID :" + id
                        print name + ":" + value + ":" + dbs + ":" + www
        except WindowsError:
            pass

def main():

    print "ListLogged-inUsers RDP Connections History"
    ListLogged_inUsers()
    print "ListAllUsers RDP Connections History"
    print "-" * 100
    AllUser()


main()
