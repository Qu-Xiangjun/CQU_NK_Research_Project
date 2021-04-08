"""
@Author: Qu Xiangjun
@Time: 2021.01.26
@Describe: 此文件负责根据雷达数据进行导航辅助函数以及导航算法实现定义
"""

import time 
from Global_Define_Var import *

def judge5_metre(lidar_data_list):
    """
    五米的距离判断可以前进的
    即在5m 内无障碍物可行的角度
    :param lidar_data_list: 原始雷达数据
    :return angle5_metre[]: 5米判断出来的可以前进的方向集合，每0.18度一个方向 
    """
    angle5_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 2500-item >= 0: # 在5m 内有障碍物
            begin = end
        end += 18 # 防止浮点数误差
        if(end - begin >= 1224): # 大于12.24°的空闲
            if( int((begin+end)/2 % 18) != 0 ):
                angle5_metre.append(int((begin + end-18)/2))
            angle5_metre.append(int((begin + end)/2))
            if( int((begin+end)/2 % 18) != 0 and int((begin + end-18)/2 %18) != 0):
                print("取值错误")
                print(begin,end,(begin+end)/2,(begin + end)/2 %18,(begin + end-18)/2)
                exit(1)
            begin += 18
    return angle5_metre

def judge2_5_metre(lidar_data_list):
    """
    二点五米的距离判断可以前进的
    :param lidar_data_list: 原始雷达数据
    :return angle2_5_metre[]: 2.5米判断出来的可以前进的方向集合，每0.18度一个方向 
    """
    angle2_5_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 1250-item >= 0: # 在2.5m内有障碍物
            begin = end
        end += 18
        if(end - begin >= 2412): # 大于24.12°的空闲
            if( int((begin+end)/2 % 18) != 0 ):
                angle2_5_metre.append(int((begin + end-18)/2))
            angle2_5_metre.append(int((begin + end)/2))
            if( int((begin+end)/2 % 18) != 0 and int((begin + end-18)/2 %18) != 0):
                print("取值错误")
                print(begin,end,(begin+end)/2,(begin + end)/2 %18,(begin + end-18)/2)
                exit(1)
            begin += 18
    return angle2_5_metre

def judge1metre(lidar_data_list):
    """
    一米的距离判断可以前进的
    :param lidar_data_list: 原始雷达数据
    :return angle1_metre[]: 1米判断出来的可以前进的方向集合，每0.18度一个方向 
    """
    angle1_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 500-item >= 0: # 在2.5m±0.6cm间有障碍物
            begin = end
        end += 18
        if(end - begin >= 6084): # 大于60.84°的空闲
            if( int((begin+end)/2 % 18) != 0 ):
                angle1_metre.append(int((begin + end-18)/2))
            angle1_metre.append(int((begin + end)/2))
            if( int((begin+end)/2 % 18) != 0 and int((begin + end-18)/2 %18) != 0):
                print("取值错误")
                print(begin,end,(begin+end)/2,(begin + end)/2 %18,(begin + end-18)/2)
                exit(1)
            begin += 18
    return angle1_metre

def judge0_5metre(lidar_data_list):
    """
    0.5米的距离判断可以前进的
    :param lidar_data_list: 原始雷达数据
    :return angle0_5_metre[]: 0.5米判断出来的可以前进的方向集合，每0.18度一个方向 
    """
    angle0_5_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if default_last_distance-item >= 0: # 在0.5m内有障碍物
            begin = end
        end += 18
        if(end - begin >= 12888*250/default_last_distance): # 大于128.88°的空闲
            if( int((begin+end)/2 % 18) != 0 ):
                angle0_5_metre.append(int((begin + end-18)/2))
            angle0_5_metre.append(int((begin + end)/2))
            if( int((begin+end)/2 % 18) != 0 and int((begin + end-18)/2 %18) != 0):
                print("取值错误")
                print(begin,end,(begin+end)/2,(begin + end)/2 %18,(begin + end-18)/2)
                exit(1)
            begin += 18
    return angle0_5_metre

def findbest_direction(valid_angle):
    """
    从有效角度中选一个最好的方向
    从前向开始选，优先选择离中线最近的,相同优先左侧
    :param valid_angle: 有效角度集合
    :return best_direction: 最优前进方向，以中线为0点，左为+，右为-
    """
    best_direction = 18000 # 初始化为右侧180度
    for item in valid_angle:
        if(abs(item - 13824) <= abs(best_direction)):
            best_direction = item - 13824
    return best_direction

def navigate(lidar_data_list):
    """
    用5米 2.5米 1米 0.5米四个判断删选出最优的前进或转向方向
    首先判断5米内有满足无障碍物的角度，有则添加到可行的集合（注意此时得到的角度肯定满足其他距离的选择条件）
    然后依次判断2.5 1 0.5 米的距离，添加到可行的集合
    最后进行选优方向：
        即选择其中最靠近前方的前进中心方向    即向右转最靠近左侧中线方向    即向左转最靠近右侧中线方向
    :param lidar_data_list: 原始雷达数据
    :return best_direction: 最优前进方向，以中线为0点，左为+，右为-
    """
    angle0_5_metre = judge0_5metre(lidar_data_list)
    angle1_metre = judge1metre(lidar_data_list)
    angle2_5_metre = judge2_5_metre(lidar_data_list)
    angle5_metre = judge5_metre(lidar_data_list)
    valid_angle = [] # 有效方向集合 即满足所有集合的要求
    if(len(angle5_metre) != 0 ):  # 若五米ok
        for item in angle5_metre:
            if(item in angle1_metre and item in angle2_5_metre and item in angle0_5_metre):
                valid_angle.append(item)
    if(len(angle2_5_metre) != 0 & len(valid_angle) == 0): # 若2.5米ok
        for item in angle2_5_metre:
            if(item in angle1_metre and item in angle0_5_metre):
                valid_angle.append(item)
    if(len(angle1_metre) != 0 & len(valid_angle) == 0): # 若1米ok
        for item in angle1_metre:
            if(item in angle0_5_metre):
                valid_angle.append(item)   
    elif(len(angle0_5_metre) != 0 & len(valid_angle) == 0): # 若1米内的0.5米ok
        valid_angle = angle0_5_metre

    if(len(valid_angle) == 0):
        print("[WARNING] There is no effective direction.\n")
        # time.sleep(0.5)
        best_direction = None
    else:
        best_direction = findbest_direction(valid_angle)/100.0 # 从所有有效方向找最好的
        if(abs(best_direction) < 5): # 优化：方向小于5度，即为0度
            best_direction = 0
    
    return best_direction