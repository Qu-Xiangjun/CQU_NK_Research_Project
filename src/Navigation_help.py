import time 
"""
五米的距离判断可以前进的
@lidar_data_list: 原始雷达数据
@return : angle5_metre[] 5米判断出来的可以前进的方向集合，每0.18度一个方向 
"""
def judge5_metre(lidar_data_list):
    angle5_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 2500-item >= 0: # 在5m 内有障碍物
            begin = end
        end += 0.18
        if(end - begin >= 12.24): # 大于12.24°的空闲
            angle5_metre.append((begin + end)/2)
            if( (begin+end)/2 % 0.18 != 0 ):
                print("取值错误")
                exit(1)
    
    return angle5_metre
"""
二点五米的距离判断可以前进的
@lidar_data_list: 原始雷达数据
@return : angle2_5_metre[] 2.5米判断出来的可以前进的方向集合，每0.18度一个方向 
"""
def judge2_5_metre(lidar_data_list):
    angle2_5_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 1250-item >= 0: # 在2.5m内有障碍物
            begin = end
        end += 0.18
        if(end - begin >= 24.12): # 大于24°的空闲
            angle2_5_metre.append((begin + end)/2)
            if( (begin+end)/2 % 0.18 != 0 ):
                print("取值错误")
                exit(1)
    
    return angle2_5_metre

"""
一米的距离判断可以前进的
@lidar_data_list: 原始雷达数据
@return : angle1_metre[] 1米判断出来的可以前进的方向集合，每0.18度一个方向 
"""
def judge1metre(lidar_data_list):
    angle1_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 500-item >= 0: # 在2.5m±0.6cm间有障碍物
            begin = end
        end += 0.18
        if(end - begin >= 60.84): # 大于60.84°的空闲
            angle1_metre.append((begin + end)/2)
            if( (begin+end)/2 % 0.18 != 0 ):
                print("取值错误")
                exit(1)
    return angle1_metre

"""
0.5米的距离判断可以前进的
@lidar_data_list: 原始雷达数据
@return : angle0_5_metre[] 0.5米判断出来的可以前进的方向集合，每0.18度一个方向 
"""
def judge0_5metre(lidar_data_list):
    angle0_5_metre = []
    begin = 0
    end = 0
    for i in range(len(lidar_data_list)-1): # 减一是忽略最后一个数据，以匹配角度的数量
        item = lidar_data_list[i]
        if 500-item >= 0: # 在0.5m内有障碍物
            begin = end
        end += 0.18
        if(end - begin >= 128.88): # 大于128.88°的空闲
            angle0_5_metre.append((begin + end)/2)
            if( (begin+end)/2 % 0.18 != 0 ):
                print("取值错误")
                exit(1)
    return angle0_5_metre

"""
从有效角度中选一个最好的方向
从前向开始选，优先选择离中线最近的,相同优先左侧
@valid_angle: 有效角度集合
@best_direction : 最优前进方向，以中线为0点，左为+，右为-
"""
def findbest_direction(valid_angle):
    best_direction = 180.0
    for item in range(valid_angle):
        if(abs(item - 88.24) <= best_direction):
            best_direction = item - 88.24
    return best_direction

"""
用5米 2.5米 1米 0.5米三个判断删选出最优的前进或转向方向
其最靠近前方的前进中心方向
即向右转最靠近左侧中线方向
即向左转最靠近右侧中线方向
@lidar_data_list: 原始雷达数据
"""
def navigate(lidar_data_list):
    angle0_5_metre = judge0_5metre(lidar_data_list)
    angle1_metre = judge1metre(lidar_data_list)
    angle2_5_metre = judge2_5_metre(lidar_data_list)
    angle5_metre = judge5_metre(lidar_data_list)
    valid_angle = []
    if(len(angle5_metre) != 0 ):
        for item in angle5_metre:
            if(item in angle1_metre and item in angle2_5_metre and item in angle0_5_metre):
                valid_angle.append(item)
    elif(len(angle2_5_metre) != 0 ):
        for item in angle2_5_metre:
            if(item in angle1_metre and item in angle0_5_metre):
                valid_angle.append(item)
    elif(len(angle1_metre) != 0 ):
        for item in angle1_metre:
            if(item in angle0_5_metre):
                valid_angle.append(item)
    elif(len(angle0_5_metre) != 0 ):
        valid_angle = angle0_5_metre
        
    if(len(valid_angle) == 0):
        print("[WARNING] There is no effective direction.\n")
        time.sleep(5)
        best_direction = None
    else:
        best_direction = findbest_direction(valid_angle)
    return best_direction
