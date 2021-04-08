import socket
import os
import sys
import numpy as np
import pickle, struct

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
address = ('10.10.47.34', 8002)  # 同一网段下的本ip地址与端口
server.bind(address) # 服务器端，将Socket与网络地址和端口绑定起来，
server.listen(2) # backlog 指定最大的连接数
# connection, address = server.accept()
ASE_DIR = os.path.dirname(os.path.abspath(__file__))  # 当前目录
# print(ASE_DIR) # c:\Users\49393\Desktop\NK_\scout_mini_project\src
connection, address = server.accept()

data = np.ones(shape=(10, 10), dtype=np.float32)

data = pickle.dumps(data, protocol=0)
print(data)
size = sys.getsizeof(data)
print("data size:", size)
header = struct.pack("i", size)
print(type(header))
connection.sendall(header)
connection.sendall(data)


