#for running this code use root process and not the current user process in linux
#use "$python3 packetsniffer.py " as the command to run the code

import socket
import struct
import textwrap

TAB_1 = '\t   '
TAB_2 = '\t\t   '
TAB_3 = '\t\t\t   '
TAB_4 = '\t\t\t\t   '



def main():

    conn = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_TCP) #creating a socket which connects to the RAW NIC of the computer

    while True:
        raw_data, addr  = conn.recvfrom(65536)
        destmac, srcmac, ethprotocol, data= ethernet_frame(raw_data) #for retrieving the contents of the ethernet packet 
        print('\nEthernet Frame:')
        print('Destination:{}, Source: {}, Protocol: {}'.format(destmac, srcmac, ethprotocol)) #ethernet frame
        print('\n\n')
        #8 for Ipv4
        if ethprotocol == 8:
            version, header_length,total_length,ttl,protocol,header_checksum,src,target,data1 = ip_packet(data)
            print(TAB_1 + 'IPv4 Packet:')
            print(TAB_2 + 'Version:{}, Header Length:{}, Total Length:{},TTL: {}'.format(version, header_length,total_length,ttl))
            print(TAB_2 + 'Protocol:{}, Header Checksum:{}, Source:{}, Destination:{}'.format(protocol,header_checksum,src,target))
            print('\n\n')
            #protocol = 6 is for TCP 
            if protocol == 6:
                 src_port, dest_port, sequence, ack, offset, flag_urg, flag_ack, flag_psh, flag_rest, flag_syn,flag_fin, data2 = unpack_tcp(data1)
                 print(TAB_1 + 'TCP Packet:')
                 print(TAB_2 + 'Source Post:{}, Dest port:{}, SeqNo:{},ACK: {}'.format(src_port, dest_port, sequence, ack))
                 print(TAB_2 + 'Offset:{}, f_URG:{}, f_ACK:{},f_PSH:{},f_REST:{}'.format(offset, flag_urg, flag_ack, flag_psh, flag_rest))
                 print(TAB_2 + 'f_SYN:{}, f_FIN:{}, TCP_Data:{}'.format(flag_syn,flag_fin, data2))
                 print('\n\n')






#Unpacking Ethernet header
def ethernet_frame(data):
    #unpack the data using struct.unpack library function
    #Im unpacking only the destination mac address, source mac address and protocol from -
    # the header. '6s 6s H' means- 6 bytes for the dest_mac
    #                              6 bytes for the src_mac
    #                              2 bytes for the protocol
    destmac, srcmac, protocol = struct.unpack('! 6s 6s H',data[:14])

    # returning the formatted destination mac, source mac, prototype and payload
    return format_mac_addr(destmac),format_mac_addr(srcmac),socket.htons(protocol), data[14:]


#return properly formatted mac address (i.e. AA:BB:CC:DD:EE:FF)
def format_mac_addr(bytes_addr):

    #mapping each binary bytes into a string in the format of two decimal places (i.e AA , BB etc)
    bytes_str = map('{:02x}'.format,bytes_addr)

    #joining the two decimal formatted byte_str using ':' 
    mac_addr = ':'.join(bytes_str).upper()

    return mac_addr 

#Unpack IPv4 packet
def ip_packet(data):

    version_header_length = data[0]
    version = version_header_length >> 4
    header_length = (version_header_length&15)*4
    # the first 20 bytes in an ipv4 header has all the header information

    #extraction total length, time to live, protocol, header_checksum, src address, dest address.
    # x - byte padding, s- char[], B-byte
    total_length,ttl,protocol,header_checksum,src,target = struct.unpack('! 2x H 4x B B H 4s 4s', data[:20])

    return version, header_length,total_length,ttl,protocol,header_checksum,ipv4_add_trans(src),ipv4_add_trans(target),data[header_length:] 

#for translating ipv4 address in human readable decimal format
def ipv4_add_trans(address):
    return '.'.join(map(str,address))

#Unpack TCP packet
def unpack_tcp(data):
    src_port, dest_port, sequence, ack, offset_reserved_flag = struct.unpack('! H H L L H',data[:14])

    #extracting all the flags
    #first 4 bits represent offset
    offset = (offset_reserved_flag >> 12)*4
    flag_urg = (offset_reserved_flag & 32) >> 5
    flag_ack = (offset_reserved_flag & 16) >> 4
    flag_psh= (offset_reserved_flag & 8) >> 3
    flag_rest = (offset_reserved_flag & 4) >> 2
    flag_syn = (offset_reserved_flag & 2) >> 1
    flag_fin = offset_reserved_flag & 1

    return src_port, dest_port, sequence, ack, offset, flag_urg, flag_ack, flag_psh, flag_rest, flag_syn,flag_fin, data[offset:] 



#Call the main function
main()
