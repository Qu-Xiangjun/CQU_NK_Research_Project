import socket
import os
import time

if __name__ == '__main__':
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("localhost", 8888)) # 服务器端，将Socket与网络地址和端口绑定起来，
    server.listen(0) # backlog 指定最大的连接数
    connection, address = server.accept()
    print(connection, address)
    i = 0
    # while True:
    # i += 1
    # print(i)
    recv_str=connection.recv(5000)
    #recv_str=str(recv_str)  这样不行带有了b''
    recv_str=recv_str.decode("GBK")
    # print( recv_str) 
    # print(type(recv_str)) #str 
    lidar_data_list = recv_str.split(",")
    lidar_data_list = lidar_data_list[0:-1]
    print( lidar_data_list) 
    # connection.send( bytes("test: %s" % recv_str,encoding="GBK") )
    # time.sleep( 0.5 )
    connection.close()
    input()