import socket

IP_ADDR = '127.0.0.1'
TCP_PORT = 5005
BUFFER_SIZE = 100
MESSAGE = "Hello server"

s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)

s.connect((IP_ADDR,TCP_PORT))
s.send(MESSAGE)

data = s.recv(BUFFER_SIZE)
s.close()

print("Received data from server: ",data)