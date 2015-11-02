#
# -*- coding: utf-8 -*-

from django.shortcuts import render
#
from django.http import HttpResponse
import json
import time
import telnetlib

def send_cmd(cmd, params):
    cmd_array={}
    cmd_array["cmd"] = cmd
    cmd_array["param"] = params
    json_str = json.dumps(cmd_array)
    try:
        tn = telnetlib.Telnet('localhost', 6001)
        tn.write(json_str.encode('ascii'))
        result = tn.read_until(b'}', 2)
        tn.close()
        res=json.loads(result.decode('ascii'))
    except:
        res={}
        res['result']='error'
    return res['result']


def index(request):
    return render(request, "index.html")


def on(request):
    # send 
    res = send_cmd('setDigit', '1')
    res = send_cmd('setMode', 'On')
    #
    return HttpResponse(json.dumps({"result": "OK"}), content_type="application/json")


def off(request):
    # send 
    res = send_cmd('setDigit', '0')
    res = send_cmd('setMode', 'Off')
    #
    return HttpResponse(json.dumps({"result": "OK"}), content_type="application/json")

def automatic(request):
    # send 
    res = send_cmd('setDigit', '2')
    res = send_cmd('setMode', 'Automatic')
    #
    return HttpResponse(json.dumps({"result": "OK"}), content_type="application/json")

def status(request):
    # get the time
    #tm_str = send_cmd('getTime', '0')
    tm=time.localtime(time.time())
    tm_str="%04d-%02d-%02d %02d:%02d" % (tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min)
    # get the current Mode
    mode_str = send_cmd('getMode', '0')
    result={}
    result['time'] = tm_str
    result['mode'] = mode_str
    return HttpResponse(json.dumps(result), content_type="application/json")






