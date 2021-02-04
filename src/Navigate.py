import socket
import time
from threading import Thread
import numpy as np
# python3.8.0 64位（python 32位要用32位的DLL）
from ctypes import *
from Navigation_help import *
from Can_frame_help import *
import draw_lidar_help

VCI_USBCAN2 = 4 # 设备类型 USBCAN-2A或USBCAN-2C或CANalyst-II 
STATUS_OK = 1 

# 定义初始化CAN的数据类型
class VCI_INIT_CONFIG(Structure):
    _fields_ = [("AccCode", c_uint),# 接收滤波验收码
                ("AccMask", c_uint),# 接收滤波屏蔽码
                ("Reserved", c_uint),
                ("Filter", c_ubyte),# '滤波方式 0,1接收所有帧。2标准帧滤波，3是扩展帧滤波。
                # 500kbps Timing0=0x00 Timing1=0x1C
                ("Timing0", c_ubyte), # 波特率参数1，具体配置，请查看二次开发库函数说明书。
                ("Timing1", c_ubyte), # 波特率参数1
                ("Mode", c_ubyte) # '模式，0表示正常模式，1表示只听模式,2自测模式
                ]

# 定义CAN信息帧的数据类型。
class VCI_CAN_OBJ(Structure):
    _fields_ = [("ID", c_uint),          
                ("TimeStamp", c_uint),   # 时间标识
                ("TimeFlag", c_ubyte),   # 是否使用时间标识
                ("SendType", c_ubyte),   # 发送标志。保留，未用
                ("RemoteFlag", c_ubyte), # 是否是远程帧
                ("ExternFlag", c_ubyte), # 是否是扩展帧
                ("DataLen", c_ubyte),    # 数据长度
                ("Data", c_ubyte*8),     # 数据
                ("Reserved", c_ubyte*3)  # 保留位
                ]

CanDLLName = './ControlCAN.dll'  # 把DLL放到对应的目录下
canDLL = windll.LoadLibrary('./ControlCAN.dll')
# Linux系统下使用下面语句，编译命令：python3 python3.8.0.py
#canDLL = cdll.LoadLibrary('./libcontrolcan.so')

if __name__ == '__main__':
    """
    Can接口连接scout——mini 底盘
    """
    # 打开设备
    ret = canDLL.VCI_OpenDevice(VCI_USBCAN2, 0, 0)
    if ret == STATUS_OK:
        print('调用 VCI_OpenDevice成功\r\n')
    if ret != STATUS_OK:
        print('调用 VCI_OpenDevice出错\r\n')

    # 初始0通道
    vci_initconfig = VCI_INIT_CONFIG(0x80000008, 0xFFFFFFFF, 0,
                                     0, 0x00, 0x1C, 0)  # 波特率500k，正常模式
    ret = canDLL.VCI_InitCAN(VCI_USBCAN2, 0, 0, byref(vci_initconfig))
    if ret == STATUS_OK:
        print('调用 VCI_InitCAN0成功\r\n')
    if ret != STATUS_OK:
        print('调用 VCI_InitCAN0出错\r\n')

    # 开启通道
    ret = canDLL.VCI_StartCAN(VCI_USBCAN2, 0, 0)
    if ret == STATUS_OK:
        print('调用 VCI_StartCAN0成功\r\n')
    if ret != STATUS_OK:
        print('调用 VCI_StartCAN0出错\r\n')
    
    # 设置底盘为指令控制模式
    ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, byref(get_start_controller_inst()), 1)
    if ret == STATUS_OK:
        print('CAN1通道发送成功\r\n')
    if ret != STATUS_OK:
        print('CAN1通道发送失败\r\n')
    
    """
    socket配置
    """
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("localhost", 8888)) # 服务器端，将Socket与网络地址和端口绑定起来，
    server.listen(0) # backlog 指定最大的连接数
    connection, address = server.accept()
    print("socket connect:",connection)
    print("socket ip address:", address)
    
    global lidar_data_list
    lidar_data_list = [0 for i in range(1536)]  # 初始化
    
    """
    绘制雷达图线程函数
    """
    thread_draw_lidar = draw_lidar_help.draw_lidar_help(lidar_data_list)
    thread_draw_lidar.start()    
    
    """
    执行导航
    """
    while True:
        try:
            recv_str=connection.recv(9216) # 1536个数据，每个为6bytes
        except(ConnectionResetError):
            print("[ConnectionResetError] Lost lidar socket connnetion.")
            break
        #recv_str=str(recv_str)  这样不行带有了b''
        recv_str=recv_str.decode("GBK")  # type(recv_str) = str 
        lidar_data_bytes = recv_str.split(",")
        lidar_data_bytes = lidar_data_bytes[0:-1]

        dirty_count = 0
        for i in range(len(lidar_data_bytes)): # 1536个数据
            lidar_data_bytes[i] = int(lidar_data_bytes[i])  # 单位从毫米
            if(lidar_data_bytes[i] == 0):
                if(i==0):
                    lidar_data_bytes[i] = 10000
                else:
                    lidar_data_bytes[i] = lidar_data_bytes[i-1]
                dirty_count += 1
        lidar_data_list = lidar_data_bytes
        
        # if(dirty_count > 200): # 脏点太多，设置界限报错
        #     print("[WARNING] Lidar is very dirty.")
        #     exit(1)
        # print("lidar_data_list",lidar_data_list)
        
        # 数据不规整报错
        if(len(lidar_data_list) != 1536):
            print("[ERROR] Lidar frame's length is not 1536*6 bytes.")
            continue 
        
        # 写入文件查看数据
        # f = open('test.txt', 'w')
        # f.write(str(lidar_data_list))
        # f.close()
        
        thread_draw_lidar.lidar_data_list = lidar_data_list # 更新绘图线程的雷达数据
        best_direction = navigate(lidar_data_list) # 导航得到的方向
        print("best_direction",best_direction)
        # time.sleep(1)
        
        # 发送控制命令给小车
        if(best_direction == None):
            # 没有方向时就自转找方向
            best_direction = 20
            ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, get_move_inst(best_direction,0), 1)
            if ret == STATUS_OK:
                print('CAN1通道发送成功\r\n')
            if ret != STATUS_OK:
                print('CAN1通道发送失败\r\n')
            continue
        for i in range(1): # 只用发送一次即可，这里可设置循环增强控制效果
            ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, get_move_inst(best_direction,best_speed = 0.25), 1)
            if ret == STATUS_OK:
                print('CAN1通道发送成功\r\n')
            if ret != STATUS_OK:
                print('CAN1通道发送失败\r\n')
        
    connection.close()
    