"""
@Author: Qu Xiangjun
@Time: 2021.02.08
@Describe: 用cv2打开电脑摄像头图像
"""

import cv2
from tkinter import *
import cv2
from PIL import Image,ImageTk
import threading
import numpy as np

class Computer_Camera_Thread(threading.Thread):
    """
    读取电脑摄像头线程
    """
    def __init__(self,socket_server_thread):
        """
        :param socket_server_thread: socket服务端线程类（发送图像信息）
        """
        threading.Thread.__init__(self)  # 初始化父类
        self.socket_server_thread = socket_server_thread
        self.camera = cv2.VideoCapture(0)    #摄像头
        print("开启电脑摄像头成功")
        self.socket_server_thread.computer_cam_flag =False
    
    def run(self):
        while True:
            success, img_frame = self.camera.read()  # 从摄像头读取照片
            if success:
                self.socket_server_thread.computer_cam_flag = True
                cv2.waitKey(10)
                cv2image = cv2.cvtColor(img_frame, cv2.COLOR_BGR2RGBA)#转换颜色从BGR到RGBAcv2.VideoCapture(0,cv2.CAP_DSHOW).read()
                current_image = Image.fromarray(cv2image)#将图像转换成Image对象
                self.socket_server_thread.computer_image = np.array(current_image)