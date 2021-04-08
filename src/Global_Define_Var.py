# -- coding: utf-8 --
"""
@Author: Qu Xiangjun
@Time: 2021.03.29
@Describe: 定义整个项目的全局控制变量值
"""

# 车底盘控制相关
default_best_direction = 0 # 默认车方向为前方0度
default_best_speed = 0.15 # 默认车的速度为0.3m/s

# 导航相关参数
default_last_distance = 400 # 单位2mm，1米以内的最近开始避障检测距离

# 与arduino下位机通信相关
default_serialPort = "COM3" # 默认与arduino连接的com串口
default_baudRate = 9600 # 默认波特率
default_detect_object = ['bird'] # 默认识别的对象集， 可行识别对象目录参考Yolo_Thread\data\coco.names中

# 远程数据传输通信server端相关参数
default_address = ('10.10.47.34', 8002) # 默认的socket通信ip地址与端口
socket_buf = 20480 # client 端接受数据的缓冲buffer大小

