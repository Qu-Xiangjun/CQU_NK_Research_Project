import socket
import math
import threading
import os
import sys
import cv2
import numpy as np
import pickle
import struct
from tkinter import *
from tkinter.messagebox import *
import _tkinter
import tkinter.messagebox
from tkinter import ttk
import tkinter as tk
from PIL import Image, ImageTk


class Cammer_Work_Thread(threading.Thread):
    def __init__(self, window, panel_camera, panel_lidar):
        threading.Thread.__init__(self)  # 初始化父类

        self.window = window
        self.panel_camera = panel_camera
        self.panel_lidar = panel_lidar

        # 保存图像的信息
        self.count = 0
        self.ASE_DIR = os.path.dirname(os.path.abspath(__file__))  # 当前目录
        # print(ASE_DIR) # c:\Users\49393\Desktop\NK_\scout_mini_project\src...

        # 图像信息
        self.numArray = None
        # 雷达信息
        self.lidar_data_list = None

    def run(self):
        """
        负责接收数据并显示
        """
        # 配置server段socket信息
        try:
            self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.address = ('127.0.0.1', 8002)  # 同一网段下的本ip地址与端口
            self.client.connect(self.address)  # 服务器端，将Socket与网络地址和端口绑定起来，
            print("连接成功")
        except:
            tkinter.messagebox.showerror(
                title='show error', message='请首先打开服务器端的socket。')
            return

        while(True):
            # 接收雷达数据
            header_lidar = self.client.recv(4)
            lidar_size = struct.unpack('i', header_lidar)
            lidar_size_temp = lidar_size[0]
            if(lidar_size_temp > 1024):
                recv_lidar = self.client.recv(1024)  # 1536个数据
                lidar_size_temp -= 1024
                while(lidar_size_temp > 1024):
                    recv_lidar += self.client.recv(1024)
                    lidar_size_temp -= 1024
                recv_lidar += self.client.recv(lidar_size_temp)
            else:
                recv_lidar = self.client.recv(lidar_size_temp)  # 1536个数据

            self.lidar_data_list = pickle.loads(recv_lidar)
            print(sys.getsizeof(recv_lidar), lidar_size[0])
            # print(self.lidar_data_list)

            # 接收图像数据
            header_cam = self.client.recv(4)
            cam_size = struct.unpack('i', header_cam)
            cam_size_temp = cam_size[0]
            if(cam_size_temp > 1024):
                recv_cam = self.client.recv(1024)  # 1536个数据
                cam_size_temp -= 1024
                while(cam_size_temp > 1024):
                    recv_cam += self.client.recv(1024)
                    cam_size_temp -= 1024
                recv_cam += self.client.recv(cam_size_temp)
            else:
                recv_cam = self.client.recv(cam_size_temp)  # 1536个数据
            print(sys.getsizeof(recv_cam), cam_size[0])
            # print(recv_cam)
            self.numArray = pickle.loads(recv_cam)
            # print(self.numArray)

            # 对齐数据作用
            self.client.sendall(header_cam)
            print(sys.getsizeof(header_cam))

            print("接收数据成功")

            # 显示雷达图像
            graph = np.zeros((500, 500, 3), np.uint8)
            angle = -48.42+90  # +90为将图像顺时针转动90°
            for radius in self.lidar_data_list:  # 遍历半径
                radius = radius*0.1/2  # 换为0.5cm为单位
                Abscissa = int(math.cos(angle*math.pi/180) * radius)  # 横坐标
                Ordinate = int(math.sin(angle*math.pi/180) * radius)  # 纵坐标
                angle -= 0.18
                if(abs(int(Abscissa)) >= 250 or abs(int(Ordinate)) >= 250):  # 超出地图5m
                    continue
                # 用CV2画线，中心位置在(250,250),和目标点，颜色是(255,0,0),线宽1
                cv2.line(graph, (250, 250), (Abscissa+250,
                                             Ordinate+250), (255, 0, 0), 1)
            current_image1 = Image.fromarray(graph)
            imgtk1 = ImageTk.PhotoImage(image=current_image1)
            self.panel_lidar.config(image=imgtk1)

            # 显示摄像头图像
            current_image = Image.fromarray(
                self.numArray).resize((800, 600), Image.ANTIALIAS)
            imgtk = ImageTk.PhotoImage(image=current_image)
            self.panel_camera.config(image=imgtk)

    def close_socket(self):
        """
        用于关闭socket
        """
        self.client.close()

    def jpg_save():
        current_image = Image.fromarray(
            self.numArray).resize((800, 600), Image.ANTIALIAS)
        current_image.save("Photo"+str(self.count)+".jpg", "jpg")


if __name__ == "__main__":
    global cammer_work_thread

    # 设计简单GUI界面
    window = tk.Tk()
    window.title('ClassroomServer')
    window.geometry('1400x750')  # 原只有摄像头 '1150x650'
    # page = Frame(window, height=700, width=60,
    #              relief=GROOVE, bd=5, borderwidth=4)  #
    # page.pack(expand=True, fill=BOTH)
    panel_camera = Label(window)
    panel_camera.pack(expand=True, fill=BOTH)
    # panel_camera.place(x=10, y=10, height=600, width=1000)  # 摄像头图像放置

    # page_lidar = Frame(window, height=500, width=60,
    #              relief=GROOVE, bd=5, borderwidth=4)  #
    # page_lidar.pack(expand=True, fill=BOTH)
    panel_lidar = Label(window)
    panel_lidar.pack(expand=True, fill=BOTH)
    # panel_lidar.place(x=1020, y=110, height=380, width=380)  # 摄像头图像放置

    # 开始获取画面
    def start_grabbing():
        global cammer_work_thread
        cammer_work_thread = Cammer_Work_Thread(
            window, panel_camera, panel_lidar)
        cammer_work_thread.start()

    # 保存图像
    def jpg_save():
        if(cammer_work_thread != None):
            cammer_work_thread.jpg_save()
        else:
            tkinter.messagebox.showerror(
                'show error', 'Failed to save image!')

    # 关闭socket
    def close_scoket_client():
        try:
            cammer_work_thread.close_scoket()
        except:
            tkinter.messagebox.showerror(
                title='show error', message='socket关闭失败。')

    # 获取画面
    btn_start_grabbing = tk.Button(
        window, text='Start Grabbing', width=35, height=1, command=start_grabbing)
    btn_start_grabbing.pack()

    # 保存图像
    btn_jpg_save = tk.Button(
        window, text='Save Photo', width=35, height=1, command=jpg_save)
    btn_start_grabbing.pack()

    # 关闭socket
    btn_close_grabbing = tk.Button(
        window, text='Close socket client', width=35, height=1, command=jpg_save)
    btn_close_grabbing.pack()

    window.mainloop()
