# python3.8.0 64位（python 32位要用32位的DLL）
#
from ctypes import *

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

print(CanDLLName)

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

# 通道0发送数据
ubyte_array = c_ubyte*8
a = ubyte_array(1, 0, 0, 0, 0, 0, 0, 0)
b = ubyte_array(0b10000001,0b1111111, 0, 0, 0, 0, 0, 0)
ubyte_3array = c_ubyte*3
reserved_temp = ubyte_3array(0, 0, 0)
vci_can_obj1 = VCI_CAN_OBJ(0x421, 0, 0, 1, 0, 0,  1, a, reserved_temp)  # 单次发送
vci_can_obj2 = VCI_CAN_OBJ(0x111, 0, 0, 1, 0, 0,  8, b, reserved_temp)  # 单次发送

ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, byref(vci_can_obj1), 1)
if ret == STATUS_OK:
    print('CAN1通道发送成功\r\n')
if ret != STATUS_OK:
    print('CAN1通道发送失败\r\n')
tempI = 100
while(tempI):
    tempI -= 1
    ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, byref(vci_can_obj2), 1)
    if ret == STATUS_OK:
        print('CAN1通道发送成功\r\n')
    if ret != STATUS_OK:
        print('CAN1通道发送失败\r\n')

# # 接收数据
# ubyte_array = c_ubyte*8
# receive_array = ubyte_array(0, 0, 0, 0, 0, 0, 0, 0)
# ubyte_3array = c_ubyte*3
# reserved_temp = ubyte_3array(0, 0, 0)
# vci_can_obj = VCI_CAN_OBJ(0x0, 0, 0, 0, 0, 0,  0, receive_array, reserved_temp)  # 复位接收缓存

# # 如果没有接收到数据，一直循环查询接收。
# tempI = 1000
# while(tempI):
#     tempI -= 1
#     ret = canDLL.VCI_Receive(VCI_USBCAN2, 0, 0, byref(vci_can_obj), 2500, 0)
#     if(ret == -1):
#         print('CAN2通道掉线\r\n')
#         break
#     if ret > 0:  # 接收到一帧数据
#         print('CAN2通道接收成功\r\n')
#         print('ID：')
#         print(vci_can_obj.ID)
#         print('DataLen：')
#         print(vci_can_obj.DataLen)
#         print('Data：')
#         print(list(vci_can_obj.Data))
#     else:
#         print('CAN2通道接收失败\r\n')

# 关闭设备
ret = canDLL.VCI_CloseDevice(VCI_USBCAN2,0)
if ret == STATUS_OK:
    print('关闭 VCI_CloseDevice成功\r\n')
if ret != STATUS_OK:
    print('关闭 VCI_CloseDevice出错\r\n')