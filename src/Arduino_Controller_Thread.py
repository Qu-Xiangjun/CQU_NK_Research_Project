"""
@Author: Qu Xiangjun
@Time: 2021.03.05
@Describe: 控制下位机arduino板子的线程类定义文件
"""

from tkinter import *
import time
import threading
import serial

class Arduino_Controller_Thread(threading.Thread):
    """
    控制下位机arduino板子线程
    """
    def __init__(self,yolo_detect_thread):
        """
        :param yolo_detect_thread: yolo目标检测线程类
        """
        threading.Thread.__init__(self)  # 初始化父类
        self.serialPort = "COM8"  # 串口
        self.baudRate = 9600  # 波特率
        self.yolo_detect_thread = yolo_detect_thread # yolo检测线程
        
    def run(self):
        """
        线程主函数
        """
        self.ser = serial.Serial(self.serialPort, self.baudRate, timeout=0.5)
        print("参数设置：串口=%s ，波特率=%d" % (self.serialPort, self.baudRate))
        
        demo1 = b"0"  # 将0转换为ASCII码方便发送
        demo2 = b"1"  # 同理
        
        self.ser.write(demo2)  # 开始时响动
        while 1:
            result_list = []
            for x in self.yolo_detect_thread.result:
                result_list.append(x[1])
            control = ("bird" in result_list) # 识别结果中有鸟
            if(control):
                self.ser.write(demo2)  # ser.write在于向串口中写入数据
                time.sleep(5)
            else:
                self.ser.write(demo1)
                # time.sleep(5)

        self.ser.close()