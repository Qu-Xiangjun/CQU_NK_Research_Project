"""
@Author: Qu Xiangjun
@Time: 2021.02.01
@Describe: 负责生成发送到底盘车的frame帧
"""
from ctypes import *
import math

# 定义初始化CAN的数据类型
class VCI_INIT_CONFIG(Structure):
    _fields_ = [("AccCode", c_uint),  # 接收滤波验收码
                ("AccMask", c_uint),  # 接收滤波屏蔽码
                ("Reserved", c_uint),
                ("Filter", c_ubyte),  # '滤波方式 0,1接收所有帧。2标准帧滤波，3是扩展帧滤波。
                # 500kbps Timing0=0x00 Timing1=0x1C
                ("Timing0", c_ubyte),  # 波特率参数1，具体配置，请查看二次开发库函数说明书。
                ("Timing1", c_ubyte),  # 波特率参数1
                ("Mode", c_ubyte)  # '模式，0表示正常模式，1表示只听模式,2自测模式
                ]

# 定义CAN信息帧的数据类型。
class VCI_CAN_OBJ(Structure):
    _fields_ = [("ID", c_uint),
                ("TimeStamp", c_uint),   # 时间标识
                ("TimeFlag", c_ubyte),   # 是否使用时间标识
                ("SendType", c_ubyte),   # 发送标志。保留，未用
                ("RemoteFlag", c_ubyte),  # 是否是远程帧
                ("ExternFlag", c_ubyte),  # 是否是扩展帧
                ("DataLen", c_ubyte),    # 数据长度
                ("Data", c_ubyte*8),     # 数据
                ("Reserved", c_ubyte*3)  # 保留位
                ]

def get_start_controller_inst():
    """
    控制底盘的首次发送控制帧
    :return vci_can_obj: vci_can_obj 控制帧体
    """ 
    ubyte_array = c_ubyte*8
    a = ubyte_array(1, 0, 0, 0, 0, 0, 0, 0)
    print(1, 0, 0, 0, 0, 0, 0, 0)
    ubyte_3array = c_ubyte*3
    reserved_temp = ubyte_3array(0, 0, 0)
    vci_can_obj = VCI_CAN_OBJ(0x421, 0, 0, 1, 0, 0,
                              1, a, reserved_temp)  # 单次发送
    return vci_can_obj

def dec_converse(dec):
    """
    将一个十进制的数目转化为16位的两个8bit的十进制
    :param dec: 输入的十进制数
    :return: (first,second)
    """
    if(dec < 0):
        b_str = bin(2**16+(dec))
        # print(b_str)
        dec = int(b_str, 2) + 1
        b_str = bin(dec)[2:]
        # print(b_str)
        # print(int(b_str[0:7], 2), int(b_str[8:15], 2))
        return (int(b_str[0:7], 2), int(b_str[8:15], 2))
    else:
        b_str = bin(dec)[2:]
        # print(b_str)
        while(len(b_str) < 16):
            b_str = '0' + b_str
        # print(b_str)
        # print(int(b_str[0:7], 2), int(b_str[8:15], 2))
        return (int(b_str[0:7], 2), int(b_str[8:15], 2))

def get_move_inst(best_direction=0, best_speed=0.15):
    """
    控制底盘的首次发送控制帧生成
    :param best_direction: 最优的移动方向
    :param best_speed: 最优的移动速度
    :return: vci_can_obj 控制帧体
    """
    # 计算车命令的对应行进速度和转向
    # 前进为mm/s,[-3000,3000]
    # 转向为0.001rad/s,[-2523,2523]
    best_direction %= 180
    best_direction *= 1.3 # 增大转弯幅度
    direc_rad = 0
    speed = best_speed  # 0.015m/s
    if(best_direction >= -90 or best_direction <= 90):  # 前进
        
        if(best_direction >= 45):
            best_direction = 45
            speed = 0
        if(best_direction <= -45):
            best_direction = -45
            speed = 0
    elif(best_direction < -90):  # 向右后退 == 后退+左转
        best_direction = -(best_direction + 90)
        speed = -speed
    else:  # (best_direction > 90)向左后退 == 后退+右转
        best_direction = -(best_direction - 90)
        speed = -speed
    
    # 转弯降速
    speed = (90-abs(best_direction))/90 * speed

    direc_rad = math.pi / 180 * best_direction * 1000
    direc_rad = int(direc_rad)
    speed = int(speed * 1000)

    (direc1, direc2) = dec_converse(direc_rad)
    (speed1, speed2) = dec_converse(speed)

    ubyte_array = c_ubyte*8
    a = ubyte_array(speed1, speed2, direc1, direc2, 0, 0, 0, 0)
    print(speed1, speed2, direc1, direc2, 0, 0, 0, 0)
    ubyte_3array = c_ubyte*3
    reserved_temp = ubyte_3array(0, 0, 0)
    vci_can_obj = VCI_CAN_OBJ(0x111, 0, 0, 1, 0, 0,8, a, reserved_temp)  # 单次发送
    return vci_can_obj
