# scout_mini_project

```python
+--src                                  # 源文件目录
| +--arduino_controller_program         # 
| | +--arduino_controller_program.ino   # 
| +--Arduino_Controller_Thread.py       # 控制arduino线程
| +--Can_frame_help.py                  # Can接口发送辅助程序
| +--Classroom_client                   # 
| | +--classroom_client.py              # 教室端程序
| +--Computer_Camera_Thread.py          # 电脑摄像头开启线程
| +--ControlCAN.dll                     # CAN协议动态库
| +--demo&learn                         # 学习的小demo例程
| | +-- ...
| +--Draw_Lidar_Help.py                 # 画雷达图辅助函数代码
| +--Global_Define_Var.py               # 全局变量定义
| +--GUI_Main                           # 工业相机GUI面板控制相关引用文件库
| | +--CamOperation_class.py            # 工业相机SDK封装函数库
| | +-- ...                             # 工业相机其他相关文件
| +--GUI_main.py                        # 程序启动入口，GUI程序界面
| +--Navigate.py                        # 导航线程
| +--Navigation_help.py                 # 导航辅助函数
| +--Socket_Server_Thread.py            # 远程传输数据（图像、雷达）Server端线程                         
| +--test_demo.py                       # 测试文件（临时文件）
| +--Yolo_Thread                        # YOLOV3相关文件和     
| | +--cfg                              # config                                            
| | | +--yolov3-spp.cfg                 # 本系统默认使用的config                             
| | +--data                             # 训练、测试数据等
| | | +--coco.data                      # 本系统默认使用的数据目录                      
| | | +--coco.names                     # 本系统的识别类型目录    
| | +--weights                          # 权重文件目录   
| | +--Yolo_Detect_Thread.py            # YOLOV3识别线程
| | +-- ...                             # YOLOV3其他相关文件 
```



