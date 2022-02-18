import socket
import select #for selecting a socket from a given list of sockets

HEADER_LENGTH = 10
IP = "127.0.0.1"
PORT = 1234

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1 )

server_socket.bind((IP,PORT))

server_socket.listen()

sockets_list = [server_socket]
clients = {} #Dictionary where client sockets are the key and message is the value

def receive_message(client_socket):
    try:
        message_header = client_socket.recv(HEADER_LENGTH)

        if not len(message_header): #If no message is received
            return False

        message_length = int(message_header.decode("utf-8").strip()) #message length should be equal to the int value of the message_header
        return{"header":message_header,"data":client_socket.recv(message_length)}
    except:
        return False


while True:
    #selecting sockets that are active
    read_sockets,_, exception_socks = select.select(sockets_list, [], sockets_list) 

    #looping for all active connections
    for notified_socket in read_sockets:

    	#If a new client has connected to the server
        if notified_socket == server_socket:
        	#Accepting the connection: Receiving the client_socket and client_address
            client_socket,client_address = server_socket.accept()

            #This first message will be the username of the user client
            user = receive_message(client_socket)

            #if no username is received 
            if user is False:
                continue

            #Append the client to the list of sockets that the server currently holds   
            sockets_list.append(client_socket)

            #add the client user = {"header","data"} to the client dictionary
            clients[client_socket] = user

            #print the information about the newly accepted client.
            print(f"Accepted new connection from {client_address[0]}:{client_address[1]} username:{user['data'].decode('utf-8')}")

        else:
        	#An already registered client has sent a message
        	message = receive_message(notified_socket)

        	#If the client doesnt send any actual message then the client connection has gone off.
        	#Delete the information of the client from the sockets_list and clients dictionary
        	if message is False:
        		print(f"Closed connection from {clients[notified_socket]['data'].decode('utf-8')}")
        		sockets_list.remove(notified_socket)
        		del clients[notified_socket]
        		continue

        	#else the client has actually sent a message
        	user = clients[notified_socket]

        	#print the received message 
        	print(f"Received message from {user['data'].decode('utf-8')}:{message['data'].decode('utf-8')}")

        	#broadcast this message to the chat room. Note: If there are two clients then only one client will 
        	#receive the message
        	for client_socket in clients:

        		if client_socket != notified_socket:
        			#Send the message in the following format to all the clients:
        			#user['header']+user['data'] - these are the values for identifying the username
        			#message['header']+message['data'] - these are the values for the actual message sent
        			client_socket.send(user['header'] + user['data'] + message['header'] + message['data'])  			

    #This loop is just to check for sockets that are erroneously created
    for notified_socket in exception_socks:
    	sockets_list.remove(notified_socket)
    	del clients[notified_socket]