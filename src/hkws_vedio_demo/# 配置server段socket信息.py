import socket
import threading
import os
import sys
import numpy as np
import pickle
import struct
# 配置server段socket信息
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
address = ('127.0.0.1', 8002)  # 同一网段下的本ip地址与端口
server.bind(address)  # 服务器端，将Socket与网络地址和端口绑定起来，
server.listen(1)  # backlog 指定最大的连接数
connection, des_address = server.accept()  # 接受client
print(connection, des_address)
ASE_DIR = os.path.dirname(os.path.abspath(__file__))  # 当前目录
# print(ASE_DIR) # c:\Users\49393\Desktop\NK_\scout_mini_project\src...

lidar_data_list = [500 for i in range(1536)]

numArray = [1 for i in range(1000)]

# 现只测试图像
while(True):
    if(numArray == None or lidar_data_list == None):
        continue
    # 发送雷达数据
    lidar_data = pickle.dumps(
        lidar_data_list, protocol=0)  # 序列化为字节序列
    lidar_size = sys.getsizeof(lidar_data)  # 获得图片序列此长度
    lidar_header = struct.pack("i", lidar_size)  # 传输图片序列长度struct打包
    connection.sendall(lidar_header)
    connection.sendall(lidar_data)

    # 发送图像
    cam_data = pickle.dumps(numArray, protocol=0)  # 序列化为字节序列
    cam_size = sys.getsizeof(cam_data)  # 获得图片序列此长度
    cam_header = struct.pack("i", cam_size)  # 传输图片序列长度struct打包
    connection.sendall(cam_header)
    connection.sendall(cam_data)
    print("发送图像数据成功")
