#!/usr/bin/env python
# -*- coding: utf-8 -*-
import win32gui as gui
import win32con,win32gui

def findwindow(pHandle):
    handle=None
    while handle!=0:
        handle=gui.FindWindowEx(pHandle,handle,None,None)        
        if handle!=0:
            findwindow(handle)
            buffer = '0' * 17
            len = win32gui.SendMessage(handle, win32con.WM_GETTEXTLENGTH) + 1
            win32gui.SendMessage(handle, win32con.WM_GETTEXT, len, buffer)
            print buffer[:len]

findwindow(gui.FindWindow(None,'TeamViewer'))
