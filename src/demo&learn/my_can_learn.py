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

# # USB-CAN系列接口卡的设备信息
# class VCI_BOARD_INFO(Structure):
#     _fields_ = [( "hw_Version","USHORT"),
#                 ( "fw_Version","USHORT"),
#                 ( "dr_Version","USHORT"),
#                 ( "in_Version","USHORT"),
#                 ( "irq_Num","USHORT"),
#                 ( "can_Num","BYTE"),
#                 ( "str_Serial_Num[20]","CHAR"),
#                 ( "str_hw_Type[40]","CHAR"),
#                 ( "Reserved[4]","USHORT")
#                 ]


# VCI_OpenDevice  打开设备
# 返回值=1，表示操作成功；=0表示操作失败；=-1表示USB-CAN设备不存在或USB掉线。
#   ("DevType",c_uint),  # 设备类型
#   ("DevIndex",c_uint), # 设备索引 
#   ("Reserved",c_uint)  # 保留参数

# VCI_CloseDevice 关闭设备
# 返回值=1，表示操作成功；=0表示操作失败；=-1表示USB-CAN设备不存在或USB掉线。
#   ("DevType",c_uint),  # 设备类型
#   ("DevIndex",c_uint), # 设备索引 

# VCI_InitCan 初始化指定的can通道
# 返回值=1，表示操作成功；=0表示操作失败；=-1表示USB-CAN设备不存在或USB掉线。
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。
#   ("pInitConfig",),  # 初始化参数结构VCI_INIT_CONFIG

# VCI_ReadBoardInfo 获取设备信息。
# 返回值=1，表示操作成功；=0表示操作失败；=-1表示USB-CAN设备不存在或USB掉线。
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("pInfo",),  # 用来存储设备信息的VCI_BOARD_INFO结构指针。

# VCI_GetReceiveNum 获取指定CAN通道的接收缓冲区中，接收到但尚未被读取的帧数量。
# 返回尚未被读取的帧数，=-1表示USB-CAN设备不存在或USB掉线
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。

# VCI_ClearBuffer 清空指定CAN通道的缓冲区。主要用于需要清除接收缓冲区数据的情况,同时发送
#                 缓冲区数据也会一并清除。
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。

# VCI_StartCAN  启动CAN卡的某一个CAN通道。有多个CAN通道时，需要多次调用。
# 返回值=1，表示操作成功；=0表示操作失败；=-1表示USB-CAN设备不存在或USB掉线
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。

# VCI_ResetCAN  复位 CAN。主要用与 VCI_StartCAN配合使用，无需再初始化，即可恢复CAN卡
# 返回值=1，表示操作成功；=0表示操作失败；=-1表示USB-CAN设备不存在或USB掉线
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。

# VCI_Transmit
# 返回实际发送的帧数，=-1表示USB-CAN设备不存在或USB掉线。
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。
#   ("pSend",),  # 要发送的帧结构体 VCI_CAN_OBJ数组的首指针。
#   ("Length",),  # 要发送的帧结构体数组的长度（发送的帧数量）。最大为1000，建议设为1，每次发送单帧，以提高发送效率。

# VCI_Receive  从指定的设备CAN通道的接收缓冲区中读取数据
# 返回实际发送的帧数，=-1表示USB-CAN设备不存在或USB掉线。
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("CANIndex",),  # CAN通道索引。 即对应卡的CAN通道号，CAN1为0，CAN2为1。
#   ("pReceive",),  # 用来接收的帧结构体VCI_CAN_OBJ数组的首指针。注意：数组的大小一定要比下面的len
#                    参数大，否则会出现内存读写错误。
#   ("Len",),  # 用来接收的帧结构体数组的长度（本次接收的最大帧数，实际返回值小于等于这个值）。
# 该值为所提供的存储空间大小，适配器中为每个通道设置了2000帧左右的接收缓存区，用户
# 根据自身系统和工作环境需求，在1到2000之间选取适当的接收数组长度。一般pReceive数
# 组大小与Len都设置大于2000，如：2500为宜，可有效防止数据溢出导致地址冲突。同时每
# 隔30ms调用一次VCI_Receive为宜。（在满足应用的时效性情况下，尽量降低调用VCI_Receive
# 的频率，只要保证内部缓存不溢出，每次读取并处理更多帧，可以提高运行效率。
# WaitTime 保留参数。

# VCI_UsbDeviceReset  复位USB-CAN适配器，复位后需要重新使用VCI_OpenDevice打开设备。等同于插拔一次
# USB设备。
#   ("DevType",),  #
#   ("DevIndex",),  #
#   ("Reserved",),  #

# VCI_FindUsbDevice2  当同一台PC上使用多个USB-CAN的时候，可用此函数查找当前的设备，并获取所有设
# 备的序列号。最多支持50个设备。
# 返回计算机中已插入的USB-CAN适配器的数量。
#   ("pInfo",),  #结构体数组首地址，用来存储设备序列号等信息的结构体数组。数组长度建议定义为50，
# 如：VCI_BOARD_INFO pInfo[50]。


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
b = ubyte_array(1, 796, 0, 0, 0, 0, 0, 0)
ubyte_3array = c_ubyte*3
reserved_temp = ubyte_3array(0, 0, 0)
vci_can_obj1 = VCI_CAN_OBJ(0x421, 0, 0, 1, 0, 0,  1, a, reserved_temp)  # 单次发送
vci_can_obj2 = VCI_CAN_OBJ(0x111, 0, 0, 1, 0, 0,  8, b, reserved_temp)  # 单次发送

ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, byref(vci_can_obj1), 1)
if ret == STATUS_OK:
    print('CAN1通道发送成功\r\n')
if ret != STATUS_OK:
    print('CAN1通道发送失败\r\n')
tempI = 100000
while(tempI):
    tempI -= 1
    ret = canDLL.VCI_Transmit(VCI_USBCAN2, 0, 0, byref(vci_can_obj2), 1)
    if ret == STATUS_OK:
        print('CAN1通道发送成功\r\n')
    if ret != STATUS_OK:
        print('CAN1通道发送失败\r\n')

# 关闭设备
ret = canDLL.VCI_CloseDevice(VCI_USBCAN2,0)
if ret == STATUS_OK:
    print('关闭 VCI_CloseDevice成功\r\n')
if ret != STATUS_OK:
    print('关闭 VCI_CloseDevice出错\r\n')