import socket
import sys
import gmplot 
from urllib.request import urlopen
from json import load
# Create a TCP/IP socket

def find_geo_loc(address):
	
	if address == '':
    		url = 'https://ipinfo.io/json'
	else:
		url = 'https://ipinfo.io/' + address + '/json'
		#Open the url 
		response = urlopen(url)
		#The following line will load the json data into the 'info' dictionary
		info = load(response)
	    
		for attr in info.keys():
		#printing all the information loaded into the key
			print(attr,' '*14+'\t->\t',info[attr])
	
		#to load the longitude and latitude into a list of strings containing ('longitude','latitude')
		location = info['loc'].split(',')

		#The following creates an object containing the information regarding the longitude and the latitude
		gmap1 = gmplot.GoogleMapPlotter(location[0],location[1], 13 ) 
  
		# Pass the absolute path 
		gmap1.draw( "/home/siddhartha/networklab/as5/locationofIP.html" ) 



try: 
    #Open a socket conecting to the internet
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    print ("Socket successfully created")

#If there is an error creating the socket.
except socket.error as err: 
    print ("socket creation failed with error %s" %(err) )
    
try: 
    #Enter any website name. I have chosen www.tutorialspoint.com as a test host
    host_ip = socket.gethostbyname('www.tutorialspoint.com')

except socket.gaierror:
	print("there was an error connecting to the host")
	sys.exit()

# Connecting to the HTTP port number	
port=80

#function call
find_geo_loc(host_ip)
#connect to the host
s.connect((host_ip,port))

#close the port
s.close()
