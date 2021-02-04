'''
can接口官方sdk demo
'''
# python3.8.0 64位（python 32位要用32位的DLL）
#
from ctypes import *

VCI_USBCAN2 = 4
STATUS_OK = 1

# 定义初始化CAN的数据类型
class VCI_INIT_CONFIG(Structure):
    _fields_ = [("AccCode", c_uint),# 接收滤波验收码
                ("AccMask", c_uint),# 接收滤波屏蔽码
                ("Reserved", c_uint),
                ("Filter", c_ubyte),# '滤波方式 0,1接收所有帧。2标准帧滤波，3是扩展帧滤波。
                ("Timing0", c_ubyte), # 波特率参数，具体配置，请查看二次开发库函数说明书。
                ("Timing1", c_ubyte),
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

ret = canDLL.VCI_OpenDevice(VCI_USBCAN2, 0, 0)
if ret == STATUS_OK:
    print('调用 VCI_OpenDevice成功\r\n')
if ret != STATUS_OK:
    print('调用 VCI_OpenDevice出错\r\n')

# 初始0通道
vci_initconfig = VCI_INIT_CONFIG(0x80000008, 0xFFFFFFFF, 0,
                                 0, 0x03, 0x1C, 0)  # 波特率125k，正常模式
ret = canDLL.VCI_InitCAN(VCI_USBCAN2, 0, 0, byref(vci_initconfig))
if ret == STATUS_OK:
    print('调用 VCI_InitCAN1成功\r\n')
if ret != STATUS_OK:
    print('调用 VCI_InitCAN1出错\r\n')

ret = canDLL.VCI_StartCAN(VCI_USBCAN2, 0, 0)
if ret == STATUS_OK:
    print('调用 VCI_StartCAN1成功\r\n')
if ret != STATUS_OK:
    print('调用 VCI_StartCAN1出错\r\n')

# 初始1通道
ret = canDLL.VCI_InitCAN(VCI_USBCAN2, 0, 1, byref(vci_initconfig))
if ret == STATUS_OK:
    print('调用 VCI_InitCAN2 成功\r\n')
if ret != STATUS_OK:
    print('调用 VCI_InitCAN2 出错\r\n')

ret = canDLL.VCI_StartCAN(VCI_USBCAN2, 0, 1)
if ret == STATUS_OK:
    print('调用 VCI_StartCAN2 成功\r\n')
if ret != STATUS_OK:
    print('调用 VCI_StartCAN2 出错\r\n')

# 通道1发送数据
ubyte_array = c_ubyte*8
a = ubyte_array(1, 2, 3, 4, 5, 6, 7, 8)
ubyte_3array = c_ubyte*3
b = ubyte_3array(0, 0, 0)
vci_can_obj = VCI_CAN_OBJ(0x1, 0, 0, 1, 0, 0,  8, a, b)  # 单次发送

ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, byref(vci_can_obj), 1)
if ret == STATUS_OK:
    print('CAN1通道发送成功\r\n')
if ret != STATUS_OK:
    print('CAN1通道发送失败\r\n')

# 通道2接收数据
a = ubyte_array(0, 0, 0, 0, 0, 0, 0, 0)
vci_can_obj = VCI_CAN_OBJ(0x0, 0, 0, 0, 0, 0,  0, a, b)  # 复位接收缓存
ret = canDLL.VCI_Receive(VCI_USBCAN2, 0, 1, byref(vci_can_obj), 2500, 0)
# print(ret)
while ret <= 0:  # 如果没有接收到数据，一直循环查询接收。
    ret = canDLL.VCI_Receive(VCI_USBCAN2, 0, 1, byref(vci_can_obj), 2500, 0)
if ret > 0:  # 接收到一帧数据
    print('CAN2通道接收成功\r\n')
    print('ID：')
    print(vci_can_obj.ID)
    print('DataLen：')
    print(vci_can_obj.DataLen)
    print('Data：')
    print(list(vci_can_obj.Data))

# 关闭
canDLL.VCI_CloseDevice(VCI_USBCAN2, 0)
