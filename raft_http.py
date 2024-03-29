#!/usr/bin/env python
# -*- coding: utf8 -*-

import sys
import json
import socket
import random

from flask import Flask, Response

debug = True
app = Flask(__name__)

partners = []
leader = None


def handle(req):
    global partners, leader

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.settimeout(1)

    for _ in range(3):
        try:
            udp_socket.sendto(json.dumps(req).encode(), leader)
            buff, addr = udp_socket.recvfrom(1024)
            rsp = json.loads(buff)
            if rsp['ret'] == -999:
                if debug:
                    print('Get Leader %s' % rsp['redirect'])
                if rsp['redirect']:
                    leader = tuple(rsp['redirect'])
                else:
                    random.shuffle(partners)
                    leader = tuple(partners[0])
                continue
            return rsp
        except Exception as e:
            if debug:
                print('Err: %s' % e)
            random.shuffle(partners)
            leader = tuple(partners[0])

    return {"ret": -9, "err": "Timeout"}

@app.route('/')
def usage():
    return Response('''Usage:
    /<key>/get          Get a key
    /<key>/set/<val>    Set a key by val
    /<key>/del          Del a key
''', mimetype='text/plain')

@app.route('/<key>/get')
def app_get(key):
    return json.dumps(handle({'cmd': 'get', 'key': key}))

@app.route('/<key>/set/<val>')
def app_set(key, val):
    return json.dumps(handle({'cmd': 'set', 'key': key, 'val': val}))

@app.route('/<key>/del')
def app_del(key):
    return json.dumps(handle({'cmd': 'del', 'key': key}))

if __name__ == '__main__':
    # python raft_http.py 9909 127.0.0.1:9901 127.0.0.1:9902 127.0.0.1:9903
    if len(sys.argv) < 5 or '-h' in sys.argv or '--help' in sys.argv:
        print('Usage: python kv_http.py httpPort partner1Host:port partner2Host:port partner3Host:port ...')
        sys.exit()

    def get_addr_by_str(s):
        hps = s.split(':')
        return (hps[0], int(hps[1]))

    for i in range(2, len(sys.argv)):
        partners.append(get_addr_by_str(sys.argv[i]))

    random.shuffle(partners)
    leader = tuple(partners[0])

    app.run(host='0.0.0.0', port=int(sys.argv[1]))
