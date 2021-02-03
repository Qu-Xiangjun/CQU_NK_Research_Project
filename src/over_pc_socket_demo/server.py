import socket
import os
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
address = ('127.0.0.1', 8000)
server.bind(address) # 服务器端，将Socket与网络地址和端口绑定起来，
server.listen(2) # backlog 指定最大的连接数
# connection, address = server.accept()
ASE_DIR = os.path.dirname(os.path.abspath(__file__))  # 当前目录
# print(ASE_DIR) # c:\Users\49393\Desktop\NK_\scout_mini_project\src
connection, address = server.accept()
connection.send("test")


