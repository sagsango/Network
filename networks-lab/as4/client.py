import socket
import select
import errno #for handling errors
import sys

HEADER_LENGTH = 10
IP = "127.0.0.1"
PORT = 1234

my_username = input("Username : ")

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((IP,PORT))
client_socket.setblocking(False)

#send the username of the client
username = my_username.encode("utf-8")
username_header = f"{len(username):<{HEADER_LENGTH}}".encode("utf-8")
#send the username to the server for registering the name of the user 
client_socket.send(username_header+username)


while True:
	#Enter the message from user
	message = input(f"{my_username} > ")

	if message:
		#Encode the message and send it to the server
		message = message.encode("utf-8")
		message_header = f"{len(message) :< {HEADER_LENGTH}}".encode("utf-8")
		client_socket.send(message_header + message)
	try:
		while True:
			#Receive Messages
			#Receive the username from the server 
			username_header = client_socket.recv(HEADER_LENGTH)
			if not len(username_header):
				print("Connection closed by the server")
				sys.exit(0)
			#First receive the username_header
			#Take its username_length and that username_length is used to receive the bytes from the server -
			#containing the username of the other client trying to send a text
			username_length = int(username_header.decode("utf-8").strip())
			username = client_socket.recv(username_length).decode("utf-8")

			#Receive the message_header and using the header length of message_header receive the message -
			#from the server
			message_header = client_socket.recv(HEADER_LENGTH)
			message_length = int(message_header.decode("utf-8").strip())
			message = client_socket.recv(message_length).decode("utf-8")

			print(f"{username}>{message}")
	#This is included as an IO error check. This varies from OS to OS. Works with WIN 10
	except IOError as e:
		if e.errno != errno.EAGAIN and e.errno != errno.EWOULDBLOCK:
			print('Reading error',str(e))
			sys.exit()
		continue

	except Exception as e:
		print('General error', str(e))