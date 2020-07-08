# !/usr/bin/env python
# coding:utf-8

# from ATAttack.framework.prints import *
import _winreg
import os
import re
import win32con
import win32api
import getpass
import sqlite3


key = r"Software\Microsoft\Terminal Server Client\Servers"
values = []
ipadder_list = []
K = 0

class ipadders():

    def __init__(self):
        pass

    def ip_into_int(self, ip):
        # 先把 192.168.1.13 变成16进制的 c0.a8.01.0d ，再去了“.”后转成10进制的 3232235789 即可。
        # (((((192 * 256) + 168) * 256) + 1) * 256) + 13
        return reduce(lambda x, y: (x << 8) + y, map(int, ip.split('.')))

    def is_internal_ip(self, ip):
        ip = self.ip_into_int(ip)
        net_a = self.ip_into_int('10.255.255.255') >> 24
        net_b = self.ip_into_int('172.31.255.255') >> 20
        net_c = self.ip_into_int('192.168.255.255') >> 16
        return ip >> 24 == net_a or ip >> 20 == net_b or ip >> 16 == net_c

class findip():

    def __init__(self):
        self.path = r"Software\\Microsoft\\Internet Explorer\\typedURLs"

    def cmd(self, list):
        for i in list:
            ret = os.popen(i).read()
            ipadder_list.append(ret.decode('cp936').encode('utf-8').strip())
        ip = re.findall(
            r'1(?:\d{1,3}\.){3}\d{1,3}(?![\.\d])',
            str(ipadder_list),
            re.S)
        iplist = []
        for ipaddr in ip:
            ipadder = ipaddr.split(
                '.')[0] + '.' + ipaddr.split('.')[1] + '.' + ipaddr.split('.')[2]
            iplist.append(ipadder)
        return iplist

    def pings(self):
        plist = []
        cmdlist = [
            r'reg query "HKEY_CURRENT_USER\Software\Microsoft\Terminal Server Client\Servers"',
            'rout print',
            'net session',
            'arp -a',
            r'type C:\windows\system32\drivers\etc\hosts',
            'ipconfig /all',
            'netstat -na',
            'wevtutil qe security /rd:true /f:text /q:"Event[System[(EventID=4624 or EventID=4625) and TimeCreated[timediff(@SystemTime) <= 4449183132]]]"']
        ipadder = (set(self.cmd(cmdlist)))
        aparagraph = [x + ".1" for x in ipadder]
        for add in aparagraph:
            if ipadders().is_internal_ip(add):
                plist.append(add)
        regex = list(set(plist))
        return (regex)


    def ie(self):
        command_list = []
        reg_root = win32con.HKEY_CURRENT_USER
        reg_flags = win32con.WRITE_OWNER | win32con.KEY_WOW64_64KEY | win32con.KEY_ALL_ACCESS
        try:
            key = win32api.RegOpenKeyEx(reg_root, self.path, 0, reg_flags)
            i = 0
            while True:
                url = (win32api.RegEnumValue(key, i))
                command_list.append(url[1])
                i += 1
                win32api.RegCloseKey(key)
        except Exception:
            pass
        print(command_list)

    def Chrome(self):
        command_list = []
        drive = 'C'
        user = getpass.getuser()
        profile = {
            'APPDATA': drive + ":\\Users\\" + user + "\\AppData\\Roaming\\",
            'USERPROFILE': u'{drive}:\\Users\\{user}\\',
            'HOMEDRIVE': u'{drive}:',
            'HOMEPATH': u'{drive}:\\Users\\{user}',
            'ALLUSERSPROFILE': drive + ":\\ProgramData",
            'COMPOSER_HOME': u'{drive}:\\Users\\{user}\\AppData\\Roaming\\Composer\\',
            'LOCALAPPDATA': drive + ":\\Users\\" + user + "\\AppData\\Local\\",
        }
        self.path = os.path.join(
            profile['LOCALAPPDATA'], u'Google\Chrome\\User Data\Default\history')
        self.query = 'SELECT action_url, username_value, password_value FROM logins'
        self.logpath = os.path.join(
            profile['LOCALAPPDATA'], u'Google\Chrome\\User Data\Default\Login Data')
        try:
            if os.path.isfile(self.path):
                c = sqlite3.connect(self.path)
                cursor = c.cursor()
                select_statement = "SELECT urls.url FROM urls;"
                cursor.execute(select_statement)
                results = cursor.fetchall()
                for i in results:
                    print(i)
                c.close()
                print(command_list)
        except:
            print('Database is locked')

    def ListLogged_inUsers(self):
        # User = getpass.getuser()
        try:
            open_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, key)
            countkey = _winreg.QueryInfoKey(open_key)[0]
            for i in range(int(countkey)):
                db = _winreg.EnumKey(open_key, i)
                servers = _winreg.OpenKey(
                    _winreg.HKEY_CURRENT_USER, key + "\\" + db)
                name, value, type = _winreg.EnumValue(servers, K)
                values.append(db + "|" + value)
                # values['current_user'] = User
                # values['Server'] = db
                # values['UsernameHint'] = value
            _winreg.CloseKey(open_key)
        except WindowsError:
            pass

    # ListAllUsers RDP Connections History
    def powershell(self, cmd):
        powershell = os.popen(r"powershell.exe " + cmd).read()
        stdout = powershell.split('\n')
        return stdout

    def AllUser(self):

        cmd = "$AllUser = Get-WmiObject -Class Win32_UserAccount;" \
              "foreach($User in $AllUser)" \
              "{Write-Host $User.Caption};"
        cmder = "$AllUser = Get-WmiObject -Class Win32_UserAccount;" \
            "foreach($User in $AllUser)" \
            "{Write-Host $User.SID};"

        user_name = self.powershell(cmd)
        sid = self.powershell(cmder)

        num = {}
        for y in range(len(user_name)):
            num[user_name[y]] = sid[y]

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

                    # print values
                    for www in num:
                        if num[www] == id:
                            # print "SID :" + id
                            values.append(dbs + "|" + value)
                            # print "User:" + www + ":" + value + ":" + dbs
            except WindowsError:
                pass

    def main(self):

        print("ListLogged-inUsers and ListAllUsers RDP Connections History:")
        print('-------------------' * 3)
        self.ListLogged_inUsers()
        self.AllUser()
        administrator = (list(set(values)))
        for user in administrator:
            print(user)
        print('-------------------' * 3)
        print('Visit record of IE browser:')
        self.ie()
        print('-------------------' * 3)
        print('Visit record of Chrome browser:')
        self.Chrome()
        print('-------------------' * 3)
        print('Network remote connection ,Hosts file and Windows Log:')
        for ip in self.pings():
            print(ip)

if __name__ == '__main__':
    test = findip()
    test.main()