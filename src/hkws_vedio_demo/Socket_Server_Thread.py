import socket
import threading
import os
import sys
import numpy as np
import pickle
import struct


class Socket_Server_Thread(threading.Thread):
    """
    负责传输视频流,雷达，控制信号等
    """

    def __init__(self, numArray=np.array([]), lidar_data_list=[]):
        threading.Thread.__init__(self)  # 初始化父类
        # 图像信息
        self.numArray = numArray  # 此为numpy数组
        # 雷达信息
        self.lidar_data_list = lidar_data_list  # 为列表

    def run(self):
        '''
        发送图像和雷达信息
        '''
        # 配置server段socket信息
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.address = ('127.0.0.1', 8002)  # 同一网段下的本ip地址与端口
        self.server.bind(self.address)  # 服务器端，将Socket与网络地址和端口绑定起来，
        self.server.listen(1)  # backlog 指定最大的连接数
        self.connection, self.des_address = self.server.accept() # 接受client
        print(self.connection, self.des_address)
        self.ASE_DIR = os.path.dirname(os.path.abspath(__file__))  # 当前目录
        # print(ASE_DIR) # c:\Users\49393\Desktop\NK_\scout_mini_project\src...
        
        # 现只测试图像
        while(True):
            if(len(self.numArray) == 0 or len(self.lidar_data_list) == 0):
                continue
            # 发送雷达数据
            try:
                lidar_data = pickle.dumps(
                    self.lidar_data_list, protocol=0)  # 序列化为字节序列
                lidar_size = sys.getsizeof(lidar_data)  # 获得图片序列此长度
                lidar_header = struct.pack("i", lidar_size)  # 传输图片序列长度struct打包
                self.connection.sendall(lidar_header)
                self.connection.sendall(lidar_data)

                # 发送图像
                cam_data = pickle.dumps(self.numArray, protocol=0)  # 序列化为字节序列
                cam_size = sys.getsizeof(cam_data)  # 获得图片序列此长度
                cam_header = struct.pack("i", cam_size)  # 传输图片序列长度struct打包
                self.connection.sendall(cam_header)
                self.connection.sendall(cam_data)
                print("发送图像数据成功")
                
                #接收对齐数据
                temp_size = self.connection.recv(4)
                print(sys.getsizeof(temp_size))
            except(Exception):
                print(Exception)
                print("socket error")
                break
    
    def close_socket(self):
        """
        用于关闭socket
        """
        self.connection.close()