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
        if 250-item >= 0: # 在0.5m内有障碍物
            begin = end
        end += 18
        if(end - begin >= 12888): # 大于128.88°的空闲
            if( int((begin+end)/2 % 18) != 0 ):
                angle0_5_metre.append(int((begin + end-18)/2))
            angle0_5_metre.append(int((begin + end)/2))
            if( int((begin+end)/2 % 18) != 0 and int((begin + end-18)/2 %18) != 0):
                print("取值错误")
                print(begin,end,(begin+end)/2,(begin + end)/2 %18,(begin + end-18)/2)
                exit(1)
            begin += 18
    return angle0_5_metre

"""
从有效角度中选一个最好的方向
从前向开始选，优先选择离中线最近的,相同优先左侧
@valid_angle: 有效角度集合
@best_direction : 最优前进方向，以中线为0点，左为+，右为-
"""
def findbest_direction(valid_angle):
    best_direction = 18000
    for item in valid_angle:
        if(abs(item - 13824) <= abs(best_direction)):
            best_direction = item - 13824
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
    if(len(angle2_5_metre) != 0 & len(valid_angle) == 0):
        for item in angle2_5_metre:
            if(item in angle1_metre and item in angle0_5_metre):
                valid_angle.append(item)
    if(len(angle1_metre) != 0 & len(valid_angle) == 0):
        for item in angle1_metre:
            if(item in angle0_5_metre):
                valid_angle.append(item)
    if(len(angle0_5_metre) != 0 & len(valid_angle) == 0):
        valid_angle = angle0_5_metre
        
    if(len(valid_angle) == 0):
        print("[WARNING] There is no effective direction.\n")
        # time.sleep(0.5)
        best_direction = None
    else:
        best_direction = findbest_direction(valid_angle)/100.0
        if(abs(best_direction) < 1.5):
            best_direction = 0
    
    return best_direction