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
    def __init__(self):
        threading.Thread.__init__(self)  # 初始化父类

        # 保存图像的信息
        self.count = 0
        self.ASE_DIR = os.path.dirname(os.path.abspath(__file__))  # 当前目录
        # print(ASE_DIR) # c:\Users\49393\Desktop\NK_\scout_mini_project\src...

        # 图像信息
        self.numArray = None
        # 雷达信息
        self.lidar_data_list = None
        # 电脑摄像头信息
        self.computer_image = None

        # 开始标志
        self.flag = False

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
            # 对齐数据作用
            self.client.sendall(header_lidar)

            lidar_size = struct.unpack('i', header_lidar)
            lidar_size_temp = lidar_size[0]
            if(lidar_size_temp > 40960):
                recv_lidar = self.client.recv(40960)  # 1536个数据
                lidar_size_temp -= 40960
                while(lidar_size_temp > 40960):
                    recv_lidar += self.client.recv(40960)
                    lidar_size_temp -= 40960
                recv_lidar += self.client.recv(lidar_size_temp)
            else:
                recv_lidar = self.client.recv(lidar_size_temp)  # 1536个数据
            self.lidar_data_list = pickle.loads(recv_lidar)
            print(sys.getsizeof(recv_lidar), lidar_size[0])
            # 对齐数据作用
            self.client.sendall(header_lidar)

            # 接收图像数据
            header_cam = self.client.recv(4)
            # 对齐数据作用
            self.client.sendall(header_lidar)

            cam_size = struct.unpack('i', header_cam)
            cam_size_temp = cam_size[0]
            if(cam_size_temp > 40960):
                recv_cam = self.client.recv(40960)  # 1536个数据
                cam_size_temp -= 40960
                while(cam_size_temp > 40960):
                    recv_cam += self.client.recv(40960)
                    cam_size_temp -= 40960
                recv_cam += self.client.recv(cam_size_temp)
            else:
                recv_cam = self.client.recv(cam_size_temp)  # 1536个数据
            print(sys.getsizeof(recv_cam), cam_size[0])
            self.numArray = pickle.loads(recv_cam)
            # 对齐数据作用
            self.client.sendall(header_cam)

            # 接收电脑摄像头图像数据
            header_comp_img = self.client.recv(4)
            # 对齐数据作用
            self.client.sendall(header_lidar)

            comp_img_size = struct.unpack('i', header_comp_img)
            comp_img_size_temp = comp_img_size[0]
            if(comp_img_size_temp > 40960):
                recv_comp_img = self.client.recv(40960)  # 1536个数据
                comp_img_size_temp -= 40960
                while(comp_img_size_temp > 40960):
                    recv_comp_img += self.client.recv(40960)
                    comp_img_size_temp -= 40960
                recv_comp_img += self.client.recv(comp_img_size_temp)
            else:
                recv_comp_img = self.client.recv(comp_img_size_temp)  # 1536个数据
            print(sys.getsizeof(recv_comp_img), comp_img_size[0])
            self.computer_image = pickle.loads(recv_comp_img)
            # 对齐数据作用
            self.client.sendall(header_cam)

            print("接收数据成功")
            self.flag = True

    def close_socket(self):
        """
        用于关闭socket
        """
        self.client.close()

    def jpg_save(self):
        current_image = Image.fromarray(
            self.numArray).resize((800, 600), Image.ANTIALIAS)
        current_image.save("Photo"+str(self.count)+".jpg", "jpg")


class Main_Image_Work_Thread(threading.Thread):
    """
    绘制主摄像头图像线程
    """

    def __init__(self, cammer_work_thread, window, panel_camera):
        threading.Thread.__init__(self)  # 初始化父类
        self.cammer_work_thread = cammer_work_thread  # 数据接收线程
        self.window = window  # 窗体
        self.panel_camera = panel_camera  # 摄像头panel
        self.numArray = []

    def run(self):
        """
        绘制图像（主摄像头）
        """
        while(True):
            self.numArray = self.cammer_work_thread.numArray
            if(self.cammer_work_thread.flag):
                # 显示摄像头图像
                current_image = Image.fromarray(
                    self.numArray).resize((800, 600), Image.ANTIALIAS)
                imgtk = ImageTk.PhotoImage(image=current_image)
                self.panel_camera.config(image=imgtk)
                self.panel_camera.imgtk = imgtk


class Lidar_Image_Work_Thread(threading.Thread):
    """
    绘制雷达图像线程
    """

    def __init__(self, cammer_work_thread, window, panel_lidar):
        threading.Thread.__init__(self)  # 初始化父类
        self.cammer_work_thread = cammer_work_thread  # 数据接收线程
        self.window = window  # 窗体
        self.panel_lidar = panel_lidar  # 雷达图panel
        self.lidar_data_list = []

    def run(self):
        """
        绘制雷达图
        """
        while(True):
            self.lidar_data_list = self.cammer_work_thread.lidar_data_list
            if(self.cammer_work_thread.flag):
                # 显示雷达图像
                graph = np.zeros((500, 500, 3), np.uint8)
                angle = -48.42+90  # +90为将图像顺时针转动90°
                for radius in self.lidar_data_list:  # 遍历半径
                    radius = radius*0.1/2  # 换为0.5cm为单位
                    radius *= 6  # 放大图像
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
                self.panel_lidar.imgtk = imgtk1


class Computer_Image_Work_Thread(threading.Thread):
    """
    绘制电脑摄像头图像线程
    """

    def __init__(self, cammer_work_thread, window, panel_computer_img):
        threading.Thread.__init__(self)  # 初始化父类
        self.cammer_work_thread = cammer_work_thread  # 数据接收线程
        self.window = window  # 窗体
        self.panel_computer_img = panel_computer_img  # 摄像头panel
        self.computer_image = []

    def run(self):
        """
        绘制图像（电脑摄像头）
        """
        while(True):
            self.computer_image = self.cammer_work_thread.computer_image
            if(self.cammer_work_thread.flag):
                # 显示摄像头图像
                current_image = Image.fromarray(
                    self.computer_image).resize((400, 300), Image.ANTIALIAS)
                imgtk = ImageTk.PhotoImage(image=current_image)
                self.panel_computer_img.config(image=imgtk)
                self.panel_computer_img.imgtk = imgtk


if __name__ == "__main__":
    global cammer_work_thread

    # 设计简单GUI界面
    window = tk.Tk()
    window.title('ClassroomServer')
    window.geometry('1400x750')  # 原面板 '1150x650'
    panel_camera = Label(window)
    panel_camera.place(x=10, y=20, height=600, width=800)  # 摄像头图像放置
    panel_lidar = Label(window)
    panel_lidar.place(x=950, y=10, height=300, width=300)  # 雷达图像放置
    panel_computer_img = Label(window)
    panel_computer_img.place(x=900, y=320, height=300, width=400)

    # 开始获取画面
    def start_grabbing():
        try:
            global cammer_work_thread
            cammer_work_thread = Cammer_Work_Thread()
            main_image_work_thread = Main_Image_Work_Thread(
                cammer_work_thread, window, panel_camera)
            lidar_image_work_thread = Lidar_Image_Work_Thread(
                cammer_work_thread, window, panel_lidar)
            computer_image_work_thread = Computer_Image_Work_Thread(
                cammer_work_thread, window, panel_computer_img)
            cammer_work_thread.start()
            main_image_work_thread.start()
            lidar_image_work_thread.start()
            computer_image_work_thread.start()
        except:
            return

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
            cammer_work_thread.close_socket()
        except:
            tkinter.messagebox.showerror(
                title='show error', message='socket关闭失败。')

    # 获取画面
    btn_start_grabbing = tk.Button(
        window, text='Start Grabbing', width=35, height=1, command=start_grabbing)
    btn_start_grabbing.place(x=300, y=700)

    # 保存图像
    btn_jpg_save = tk.Button(
        window, text='Save Photo', width=35, height=1, command=jpg_save)
    btn_jpg_save.place(x=600, y=700)

    # 关闭socket
    btn_close_grabbing = tk.Button(
        window, text='Close socket client', width=35, height=1, command=close_scoket_client)
    btn_close_grabbing.place(x=900, y=700)

    window.mainloop()
