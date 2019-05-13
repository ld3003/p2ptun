#coding=utf-8

import socket
import threading
import SocketServer
import time
import datetime
import binascii

import thread
import json
import os
import sys
import traceback


HOST, PORT = '0.0.0.0', 29001


#from ipdb import
#set_trace set_trace()

reload(sys) # Python2.5 初始化后删除了 sys.setdefaultencoding 方法，我们需要重新载入 
sys.setdefaultencoding('utf-8')

peermap = dict();

class p2ptun_handler(SocketServer.BaseRequestHandler):
    def handle(self):
        try:
            data = self.request[0]
            socket = self.request[1]
	    jsondata = json.loads(data)
            print data
            peermap[jsondata['from']] = self.client_address
            socket.sendto(data, peermap[jsondata['to']])

            print str(jsondata['from'])+" ---> "+str(jsondata['to'])
            
        except Exception, e:
            print traceback.format_exc()

class echo_handler(SocketServer.BaseRequestHandler):
    def handle(self):
        try:
            ap = dict()
	    ap['cmd'] = 101
	    ap['seq'] = 1
	    ap['addr'] = self.client_address[0]
	    ap['port'] = self.client_address[1]
	    data = json.dumps(ap)
	    socket = self.request[1]
            socket.sendto(data,self.client_address)         


      	except Exception, e:
            logging.warn(traceback.format_exc())

 

class ThreadedUDPServer(SocketServer.ThreadingMixIn, SocketServer.UDPServer):
    pass

def process_udppkg(pkg):
    socket = pkg[0]
    address = pkg[1]
    data = pkg[2];
       
if __name__ == "__main__":

        
    #logging.info('SRV START ' + __file__)

    #print database.get_userdevice_info('opP9Sv8ImKryDRGTA5Mg6UDVZRBY','862991528784447f')['NAME']

    server_p2ptun = ThreadedUDPServer((HOST, PORT), p2ptun_handler)
    server_p2ptun_thread = threading.Thread(target=server_p2ptun.serve_forever)
    server_echo1 = ThreadedUDPServer((HOST, PORT+1), echo_handler)
    server_echo1_thread = threading.Thread(target=server_echo1.serve_forever)
    server_echo2 = ThreadedUDPServer((HOST, PORT+2), echo_handler)
    server_echo2_thread = threading.Thread(target=server_echo2.serve_forever)
    
    server_p2ptun_thread.daemon = True
    server_p2ptun_thread.start()

    server_echo1_thread.daemon = True
    server_echo1_thread.start()

    server_echo2_thread.daemon = True
    server_echo2_thread.start()



   
    while True:
        #print ( 'RUNNING ' + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        time.sleep(2)
