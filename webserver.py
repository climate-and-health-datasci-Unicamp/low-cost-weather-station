# -*- coding: utf-8 -*-
"""
Created on Sun Dec 27 20:25:49 2020

@author: guiii
"""

import threading
import socket as skt
import pandas as pd

PORT = 6666
terminate_flag = False
TERMINATEMSG = "END0XFFFFFFFF"
INFOCOLUMNS = ["Station ID","Name","Latitude","Longitude","Elevation","First Record"]
DATACOLUMNS = ["Station_ID","Date","Time","Temperature (C)","Humidity (%)"]
HEADERSIZE = 5

class serverHandlerThread(threading.Thread):
    def __init__(self, socket):
        threading.Thread.__init__(self)
        self._stop_event = threading.Event()
        self.socket = socket

    def stop(self):
        self.socket.close()
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()

    def run(self):
        global terminate_flag
        while not terminate_flag:
            pass
        self.stop()

class connHandlerThread(threading.Thread):
    def __init__(self, conn_socket, client_addr):
        threading.Thread.__init__(self)
        self._stop_event = threading.Event()
        self.socket = conn_socket
        self.addr = client_addr

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()

    def run(self):
        global terminate_flag
        rawdata = ''
        new_msg = True
        while True:
            msg = self.socket.recv(16)
            if new_msg:
                msglen = int(msg[:HEADERSIZE])
                new_msg = False

            rawdata += msg.decode("utf-8")

            if len(rawdata)-HEADERSIZE >= msglen:
                new_msg = True
                break
        self.socket.send(bytes("received","utf-8"))
        self.socket.close()

        if rawdata[HEADERSIZE:] == TERMINATEMSG:
            terminate_flag = True
        elif rawdata[HEADERSIZE:HEADERSIZE+4] == "DATA":
            row = rawdata[HEADERSIZE+4:]
            data = [row.split(',')]
            newData = pd.DataFrame(data, columns=DATACOLUMNS)
            
            try:
                df = pd.read_csv(data[0]+'.csv')
                df = df.append(newData)
                df.to_csv(data[0]+'.csv')
            except FileNotFoundError:
                newData.to_csv(data[0]+'.csv')
                
        elif rawdata[HEADERSIZE:HEADERSIZE+4] == "INFO":
            row = rawdata[HEADERSIZE+4:]
            data = [row.split(',')]
            newData = pd.DataFrame(data, columns=INFOCOLUMNS)
            try:
                df = pd.read_csv('INFO.csv')
                df = df.append(newData)
                df.to_csv('INFO.csv')
            except FileNotFoundError:
                newData.to_csv('INFO.csv')
            

        print("The connection with %s has ended!" %self.addr[0])

socket = skt.socket(skt.AF_INET, skt.SOCK_STREAM)

socket.bind(("", PORT))
socket.listen(skt.SOMAXCONN)

socket_thread = serverHandlerThread(socket)
socket_thread.start()

while not socket_thread.stopped():
    try:
        connection, address = socket.accept()
        print("A connection has started with ", address[0])
        new_thread = connHandlerThread(connection, address)
        new_thread.start()
    except OSError:
        pass

socket_thread.join()