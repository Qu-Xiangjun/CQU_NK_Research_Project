# -- coding: utf-8 --

from Yolo_Detect_Thread import *
from MvCameraControl_class import *
from CamOperation_class import *
from tkinter import *
from tkinter.messagebox import *
import _tkinter
import tkinter.messagebox
from tkinter import ttk
import tkinter as tk
import sys
import os
from PIL import Image, ImageTk
from Navigate import *
from Draw_Lidar_Help import *
from Socket_Server_Thread import *
from Computer_Camera_Thread import *
from Arduino_Controller_Thread import *
sys.path.append("./GUI_Main")
sys.path.append("./GUI_Main/MvImport")
sys.path.append("./Yolo_Thread")


def TxtWrapBy(start_str, end, all):
    """
    获取选取设备信息的索引，通过[]之间的字符去解析
    :param start_str: 开始字符
    :param: end: 结束字符
    :param: all: 总字符串
    :return: 返回相应解析的字符串
    """
    start = all.find(start_str)
    if start >= 0:
        start += len(start_str)
        end = all.find(end, start)
        if end >= 0:
            # strip() 方法用于移除字符串头尾指定的字符（默认为空格或换行符）或字符序列。
            return all[start:end].strip()


def ToHexStr(num):
    """
    将返回的错误码转换为十六进制显示
    :param num: 错误码 字符串
    :return: 十六进制字符串
    """
    chaDic = {10: 'a', 11: 'b', 12: 'c', 13: 'd', 14: 'e', 15: 'f'}
    hexStr = ""
    if num < 0:
        num = num + 2**32
    while num >= 16:
        digit = num % 16
        hexStr = chaDic.get(digit, str(digit)) + hexStr
        num //= 16
    hexStr = chaDic.get(num, str(num)) + hexStr
    return hexStr


def main():
    # 设备列表
    global deviceList
    deviceList = MV_CC_DEVICE_INFO_LIST()
    # 设备使用类型
    global tlayerType
    tlayerType = MV_GIGE_DEVICE | MV_USB_DEVICE
    # 初始化相机类 from MvCameraControl_class
    # 其从c++的dll动态库转化并封装好了所有的相机操作基本函数
    global cam
    cam = MvCamera()
    # 定义设备在设备列表的位置
    global nSelCamIndex
    nSelCamIndex = 0
    # 定义CamOperation_class中的CameraOperation类实例化变量对应
    global obj_cam_operation
    obj_cam_operation = 0
    # 定义表示相机正在使用，初始化为未使用
    global b_is_run
    b_is_run = False

    # 设计简单GUI界面
    window = tk.Tk()
    window.title('BasicDemo')
    window.geometry('1150x700')
    model_val = tk.StringVar()  # 摄像头模式
    global triggercheck_val  # 摄像头trigger模式
    triggercheck_val = tk.IntVar()
    page = Frame(window, height=400, width=60,
                 relief=GROOVE, bd=5, borderwidth=4)  # 控制面板
    page.pack(expand=True, fill=BOTH)
    panel = Label(page)
    panel.place(x=190, y=10, height=600, width=1000)  # 摄像头图像放置

    # 目标检测线程实例化
    global yolo_detect_thread
    yolo_detect_thread = Yolo_Detect_Thread()   # 在开始视频流时自动开始检测

    # arduino下位机控制线程实例化
    global arduino_controller_thread
    arduino_controller_thread = Arduino_Controller_Thread(yolo_detect_thread)

    # 雷达图绘画线程实例化
    global thread_draw_lidar
    thread_draw_lidar = Draw_Lidar_Help()

    # 远程图像传输线程实例化
    global socket_server_thread
    socket_server_thread = Socket_Server_Thread()

    # 导航线程实例化
    global navigate_thread
    navigate_thread = Navigate_Thread(thread_draw_lidar, socket_server_thread)

    # 绑定下拉列表至设备信息索引

    def xFunc(event):
        global nSelCamIndex  # 相机在相机列表的位置
        nSelCamIndex = TxtWrapBy("[", "]", device_list.get())  # .get()获取下拉框的值

    # ch:枚举相机 | en:enum devices
    def enum_devices():
        global deviceList  # 设备列表
        global obj_cam_operation  # 相机操作类
        deviceList = MV_CC_DEVICE_INFO_LIST()
        tlayerType = MV_GIGE_DEVICE | MV_USB_DEVICE
        ret = MvCamera.MV_CC_EnumDevices(
            tlayerType, deviceList)  # 获得设备列表数据到deviceList中
        if ret != 0:  # 函数返回异常
            tkinter.messagebox.showerror(
                'show error', 'enum devices fail! ret = ' + ToHexStr(ret))

        if deviceList.nDeviceNum == 0:  # 没有设备
            tkinter.messagebox.showinfo('show info', 'find no device!')

        print("Find %d devices!" % deviceList.nDeviceNum)

        devList = []
        for i in range(0, deviceList.nDeviceNum):
            mvcc_dev_info = cast(deviceList.pDeviceInfo[i], POINTER(
                MV_CC_DEVICE_INFO)).contents  # 变化类型为设备信息类
            if mvcc_dev_info.nTLayerType == MV_GIGE_DEVICE:  # 若设备的类型为GIGE
                print("\ngige device: [%d]" % i)
                chUserDefinedName = ""
                for per in mvcc_dev_info.SpecialInfo.stGigEInfo.chUserDefinedName:  # chUserDefinedName：用户定义的名称
                    if 0 == per:
                        break
                    chUserDefinedName = chUserDefinedName + chr(per)
                print("device model name: %s" % chUserDefinedName)

                nip1 = (
                    (mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24)
                nip2 = (
                    (mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16)
                nip3 = (
                    (mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8)
                nip4 = (mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff)
                print("current ip: %d.%d.%d.%d\n" %
                      (nip1, nip2, nip3, nip4))  # ip地址
                devList.append("["+str(i)+"]GigE: " + chUserDefinedName +
                               "(" + str(nip1)+"."+str(nip2)+"."+str(nip3)+"."+str(nip4) + ")")
            elif mvcc_dev_info.nTLayerType == MV_USB_DEVICE:  # 若设备的类型为USB
                print("\nu3v device: [%d]" % i)
                chUserDefinedName = ""
                for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chUserDefinedName:
                    if per == 0:
                        break
                    chUserDefinedName = chUserDefinedName + chr(per)
                print("device model name: %s" % chUserDefinedName)

                strSerialNumber = ""
                for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chSerialNumber:
                    if per == 0:
                        break
                    strSerialNumber = strSerialNumber + chr(per)
                print("user serial number: %s" % strSerialNumber)
                devList.append(
                    "["+str(i)+"]USB: " + chUserDefinedName + "(" + str(strSerialNumber) + ")")
        device_list["value"] = devList
        device_list.current(0)  # device_list为下拉框Combobox，此为默认显示第0个

    # ch:打开相机 | en:open device
    def open_device():
        global deviceList
        global nSelCamIndex
        global obj_cam_operation
        global b_is_run
        if True == b_is_run:
            tkinter.messagebox.showinfo('show info', 'Camera is Running!')
            return
        obj_cam_operation = CameraOperation(
            socket_server_thread,  # 图像传输线程对象
            yolo_detect_thread,  # 目标检测线程对象
            cam, deviceList, nSelCamIndex)  # 实例化相机辅助操作类
        ret = obj_cam_operation.Open_device()
        if 0 != ret:
            b_is_run = False
        else:
            model_val.set('continuous')
            b_is_run = True

    # ch:开始取流 | en:Start grab image
    def start_grabbing():
        global obj_cam_operation
        obj_cam_operation.Start_grabbing(window, panel)
        yolo_detect_thread.start()  # 开启目标检测

    # ch:停止取流 | en:Stop grab image
    def stop_grabbing():
        global obj_cam_operation
        obj_cam_operation.Stop_grabbing()

    # ch:关闭设备 | Close device
    def close_device():
        global b_is_run
        global obj_cam_operation
        obj_cam_operation.Close_device()
        b_is_run = False
        # 清除文本框的数值
        text_frame_rate.delete(1.0, tk.END)
        text_exposure_time.delete(1.0, tk.END)
        text_gain.delete(1.0, tk.END)

    # ch:设置触发模式 | en:set trigger mode
    def set_triggermode():
        global obj_cam_operation
        strMode = model_val.get()
        obj_cam_operation.Set_trigger_mode(strMode)

    # ch:设置触发命令 | en:set trigger software
    def trigger_once():
        global triggercheck_val
        global obj_cam_operation
        nCommand = triggercheck_val.get()
        obj_cam_operation.Trigger_once(nCommand)

    # ch:保存bmp图片 | en:save bmp image
    def bmp_save():
        global obj_cam_operation
        obj_cam_operation.b_save_bmp = True

    # ch:保存jpg图片 | en:save jpg image
    def jpg_save():
        global obj_cam_operation
        obj_cam_operation.b_save_jpg = True

    # ch:获取摄像头参数与设置面板
    def get_parameter():
        global obj_cam_operation
        obj_cam_operation.Get_parameter()
        text_frame_rate.delete(1.0, tk.END)
        text_frame_rate.insert(1.0, obj_cam_operation.frame_rate)
        text_exposure_time.delete(1.0, tk.END)
        text_exposure_time.insert(1.0, obj_cam_operation.exposure_time)
        text_gain.delete(1.0, tk.END)
        text_gain.insert(1.0, obj_cam_operation.gain)

    # ch:设置摄像头的参数
    def set_parameter():
        global obj_cam_operation
        obj_cam_operation.exposure_time = text_exposure_time.get(1.0, tk.END)
        obj_cam_operation.exposure_time = obj_cam_operation.exposure_time.rstrip(
            "\n")
        obj_cam_operation.gain = text_gain.get(1.0, tk.END)
        obj_cam_operation.gain = obj_cam_operation.gain.rstrip("\n")
        obj_cam_operation.frame_rate = text_frame_rate.get(1.0, tk.END)
        obj_cam_operation.frame_rate = obj_cam_operation.frame_rate.rstrip(
            "\n")
        obj_cam_operation.Set_parameter(
            obj_cam_operation.frame_rate, obj_cam_operation.exposure_time, obj_cam_operation.gain)

    # ch:启动远程图像传输
    def start_camera_transfor():
        # 打开电脑摄像头
        computer_camera_thread = Computer_Camera_Thread(socket_server_thread)
        computer_camera_thread.start()
        # 远程图像传输线程实例化
        socket_server_thread.start()

    # ch:关闭远程图像传输

    def close_camera_transfor():
        try:
            socket_server_thread.close_socket()
        except:
            tkinter.messagebox.showerror(
                title='show error', message='socket关闭失败。')

    # ch: 打开导航
    def start_navigate():
        navigate_thread.start()

    # ch: 打开雷达图
    def start_lidar_image():
        thread_draw_lidar.start()

    # ch: 打开驱鸟版控制的下位机arduino单片机
    def start_arduino_controller():
        arduino_controller_thread.start()

    xVariable = tkinter.StringVar()  # 将Combobox的内容设置为字符类型，用var来接收传出内容用以显示
    device_list = ttk.Combobox(window, textvariable=xVariable, width=30)
    device_list.place(x=20, y=20)
    device_list.bind("<<ComboboxSelected>>", xFunc)

    # label_exposure_time = tk.Label(
    #     window, text='Exposure Time', width=15, height=1)
    # label_exposure_time.place(x=20, y=350)
    # text_exposure_time = tk.Text(window, width=15, height=1)
    # text_exposure_time.place(x=160, y=350)

    # label_gain = tk.Label(window, text='Gain', width=15, height=1)
    # label_gain.place(x=20, y=400)
    # text_gain = tk.Text(window, width=15, height=1)
    # text_gain.place(x=160, y=400)

    # label_frame_rate = tk.Label(window, text='Frame Rate', width=15, height=1)
    # label_frame_rate.place(x=20, y=450)
    # text_frame_rate = tk.Text(window, width=15, height=1)
    # text_frame_rate.place(x=160, y=450)

    # 查找设备
    btn_enum_devices = tk.Button(
        window, text='Enum Devices', width=35, height=1, command=enum_devices)
    btn_enum_devices.place(x=20, y=50)
    # 打开摄像头
    btn_open_device = tk.Button(
        window, text='Open Device', width=15, height=1, command=open_device)
    btn_open_device.place(x=20, y=100)
    # 获取视频流
    btn_close_device = tk.Button(
        window, text='Close Device', width=15, height=1, command=close_device)
    btn_close_device.place(x=160, y=100)

    # 设置Continuous模式，单选
    radio_continuous = tk.Radiobutton(window, text='Continuous', variable=model_val,
                                      value='continuous', width=15, height=1, command=set_triggermode)
    radio_continuous.place(x=20, y=150)
    # 设置triggermode模式，单选
    radio_trigger = tk.Radiobutton(window, text='Trigger Mode', variable=model_val,
                                   value='triggermode', width=15, height=1, command=set_triggermode)
    radio_trigger.place(x=160, y=150)
    model_val.set(1)  # 1位默认trigger_once函数的Tigger by Software

    # 取视频流按钮
    btn_start_grabbing = tk.Button(
        window, text='Start Grabbing', width=15, height=1, command=start_grabbing)
    btn_start_grabbing.place(x=20, y=200)
    # 停止视频流
    btn_stop_grabbing = tk.Button(
        window, text='Stop Grabbing', width=15, height=1, command=stop_grabbing)
    btn_stop_grabbing.place(x=160, y=200)

    # # Tigger by Software 单选框，用于trigger_once
    # checkbtn_trigger_software = tk.Checkbutton(
    #     window, text='Tigger by Software', variable=triggercheck_val, onvalue=1, offvalue=0)
    # checkbtn_trigger_software.place(x=20, y=250)
    # # 用Tigger by Software 单选框的选择情况作函数执行
    # btn_trigger_once = tk.Button(
    #     window, text='Trigger Once', width=15, height=1, command=trigger_once)
    # btn_trigger_once.place(x=160, y=250)

    # 保存图片按钮
    btn_save_bmp = tk.Button(window, text='Save as BMP',
                             width=15, height=1, command=bmp_save)
    btn_save_bmp.place(x=20, y=250)
    btn_save_jpg = tk.Button(window, text='Save as JPG',
                             width=15, height=1, command=jpg_save)
    btn_save_jpg.place(x=160, y=250)

    # # 参数设置与获得
    # btn_get_parameter = tk.Button(
    #     window, text='Get Parameter', width=15, height=1, command=get_parameter)
    # btn_get_parameter.place(x=20, y=500)
    # btn_set_parameter = tk.Button(
    #     window, text='Set Parameter', width=15, height=1, command=set_parameter)
    # btn_set_parameter.place(x=160, y=500)

    # 开启远程图像传输
    btn_start_camera_transfor = tk.Button(window, text='Start camera transfor',
                                          width=20, height=1, command=start_camera_transfor)
    btn_start_camera_transfor.place(x=80, y=350)

    # 关闭远程图像传输
    btn_close_camera_transfor = tk.Button(window, text='Close camera transfor',
                                          width=20, height=1, command=close_camera_transfor)
    btn_close_camera_transfor.place(x=80, y=400)

    btn_start_navigate = tk.Button(window, text='Start Navigate',
                                   width=20, height=1, command=start_navigate)
    btn_start_navigate.place(x=80, y=450)

    btn_start_lidar_image = tk.Button(window, text='Lidar Image',
                                      width=20, height=1, command=start_lidar_image)
    btn_start_lidar_image.place(x=80, y=500)

    btn_start_arduino = tk.Button(window, text='Start Arduino Controller',
                                  width=20, height=1, command=start_arduino_controller)
    btn_start_arduino.place(x=80, y=550)

    window.mainloop()


if __name__ == "__main__":
    main()
