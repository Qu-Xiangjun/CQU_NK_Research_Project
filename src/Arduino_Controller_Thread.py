from tkinter import *
import threading
import serial

class Arduino_Controller_Thread(threading.Thread):
    """
    控制下位机arduino板子线程
    """
    def __init__(self,yolo_detect_thread):
        threading.Thread.__init__(self)  # 初始化父类
        self.serialPort = "COM8"  # 串口
        self.baudRate = 9600  # 波特率
        self.yolo_detect_thread = yolo_detect_thread # yolo检测线程
        
    def run(self):
        self.ser = serial.Serial(self.serialPort, self.baudRate, timeout=0.5)
        print("参数设置：串口=%s ，波特率=%d" % (self.serialPort, self.baudRate))
        demo1 = b"0"  # 将0转换为ASCII码方便发送
        demo2 = b"1"  # 同理
        while 1:
            control = ("bird" in self.yolo_detect_thread.result) # 识别结果中有鸟
            if(control):
                self.ser.write(demo1)  # ser.write在于向串口中写入数据
            if(control):
                self.ser.write(demo2)

        self.ser.close()