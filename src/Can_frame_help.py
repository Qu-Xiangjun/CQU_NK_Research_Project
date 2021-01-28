from ctypes import *
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

"""
控制底盘的首次发送控制帧
@return: vci_can_obj 控制帧体
"""
def get_start_controller_inst():
    ubyte_array = c_ubyte*8
    a = ubyte_array(1, 0, 0, 0, 0, 0, 0, 0)
    ubyte_3array = c_ubyte*3
    reserved_temp = ubyte_3array(0, 0, 0) 
    vci_can_obj = VCI_CAN_OBJ(0x421, 0, 0, 1, 0, 0,  1, a, reserved_temp)  # 单次发送
    return vci_can_obj

"""
控制底盘的首次发送控制帧
@best_direction: 最优的移动方向
@return: vci_can_obj 控制帧体
"""
def get_move_inst(best_direction):
    pass