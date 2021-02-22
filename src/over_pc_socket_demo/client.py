import socket
import os
import sys
import numpy as np
import pickle, struct

client = socket.socket()
print(client)

address = ('127.0.0.1', 8100)
client.connect(address)

header = client.recv(4)
print(sys.getsizeof(header))
size = struct.unpack('i', header)
# print(type(size)) # tuple
print('data size:', size)

#Receive data
recv_data = client.recv(size[0])

data = recv_data
data = pickle.loads(data)
print(data)


