import socket

IP_ADDR = '127.0.0.1'
TCP_PORT = 5005
BUFFER_SIZE = 100

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.bind((IP_ADDR,TCP_PORT))
s.listen(1)

conn, addr=s.accept()
server = "Hello client"
print('Connection address: ',addr)

data = conn.recv(BUFFER_SIZE)
    
print("Received data from client: ", data)
conn.send(server)

conn.close()    