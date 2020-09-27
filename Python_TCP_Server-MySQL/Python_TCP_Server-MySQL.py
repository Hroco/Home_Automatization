#testing tcp conenction from arduino tcp client to python server

import socket
import time
import threading
from multiprocessing.pool import ThreadPool

BUFFER_SIZE = 300  

TCP_IP = '' # local ip
TCP_PORT =  # port

print("Starting server")
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))
s.listen(1)
print("Listening")
conn, addr = s.accept()
print (addr)
while 1:
    data = conn.recv(BUFFER_SIZE)
    if not data: break
    print (data)
conn.close()
