import socket
import os
client = socket.socket()
print(client)

address = ('192.168.1.152', 8001)
client.connect(address)

print(client.recv(1024))


