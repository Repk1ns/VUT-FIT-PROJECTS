# IPK - Project1 - HTTP Domain name resolver
# Author : Vojtech Mimochodek
# Login : xmimoc01
# Date : 8.3.2020

import socket
import re
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

HOST = '127.0.0.1'
PORT = int(sys.argv[1])

# Function for sending errors
def send_err(code_message):
    response = code_message
    response = response.encode('utf-8')
    conn.sendall(response)

### SERVER ###
##############

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Bind and listening to incoming socket
    try:
        s.bind((HOST, PORT))
    except:
        print("ERROR: Bad Port\n")
        sys.exit(1)

    s.listen()
    
    while True:
        # Accept Request Socket
        conn, addr = s.accept()
        
        # Get Request Data
        data = conn.recv(2048)
        
        if not data:
            print("ERROR - Bad DATA\n")
            break
        
        # Decode and prepare data request data
        request = data.decode("utf-8")
        split_request = request.split( )

        if split_request[0] == 'GET':

            # Check correct request input
            if split_request[1].find('/resolve?name=') != 0:
                send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                conn.close()
                continue
            if split_request[1].find('&type=') == -1:
                send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                conn.close()
                continue

            # State GET - Type A
            if split_request[1].endswith("A"):

                address_ip = split_request[1].split('name=')
                address_ip = address_ip[1].split('&')
                
                # Check if get empty domain name
                if address_ip[0] == '':
                    send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                    conn.close()
                    continue

                # Get IP and check if IP address exists on that domain name
                try:
                    ip = socket.gethostbyname(address_ip[0])
                except socket.error:
                    send_err("HTTP/1.1 404 Not Found\r\n\r\n")
                    conn.close()
                    continue

                # Check if IP:A instead of NAME:A
                if re.match(r"^\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3}$|^\d{1,3}.\d{1,3}.\d{1,3}$|^\d{1,3}.\d{1,3}$|^\d{1,3}$",address_ip[0]):
                    send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                    conn.close()
                    continue

                answer_ip = address_ip[0] + ":A=" + ip
                response_ip = "HTTP/1.1 200 OK\r\n\r\n"+answer_ip+"\r\n"
                response_ip = response_ip.encode('utf-8')
                
                # Sending Response
                conn.sendall(response_ip)
                conn.close()
            
            # State GET - Type PTR
            elif split_request[1].endswith("PTR"):

                address_name = split_request[1].split('name=')
                address_name = address_name[1].split('&')
                
                # Validate IP Address
                try:
                    socket.inet_aton(address_name[0])
                except socket.error:
                    send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                    conn.close()
                    continue
                
                # Check if domain name exists on that IP
                try:
                    name = socket.gethostbyaddr(address_name[0])
                except socket.error:
                    send_err("HTTP/1.1 404 Not Found\r\n\r\n")
                    conn.close()
                    continue
                
                answer_name = address_name[0] + ":PTR=" + name[0]
                response_name = "HTTP/1.1 200 OK\r\n\r\n"+answer_name+"\r\n"
                response_name = response_name.encode('utf-8')
                
                # Sending Response
                conn.sendall(response_name)
                conn.close()
            
            # Unknown type - not A or PTR
            else: 
                send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                conn.close()

        # State POST
        elif split_request[0] == 'POST':
            
            if split_request[1] != '/dns-query':
                send_err("HTTP/1.1 400 Bad Request\r\n\r\n")
                conn.close()
                continue

            split_addresses = request.split('\r\n\r\n')
            split_addresses = split_addresses[1].split('\n')
            addr_count = len(split_addresses)

            response_post = "HTTP/1.1 200 OK\r\n\r\n"
            response_post_check = ''
            
            # Getting addresses from txt document in POST request
            for i in range(addr_count):
                spec_addr = split_addresses[i].split(':')

                split_addresses[i] = split_addresses[i].strip()
                if split_addresses[i].endswith("A"):
                    
                    # Validate domain names and IP addresses
                    try:
                        ip_post = socket.gethostbyname(spec_addr[0])
                        answer_ip_post = spec_addr[0] + ":A=" + ip_post+"\r\n"
                        response_post_check += answer_ip_post
                    except socket.error:
                        response_post_check += ''

                elif split_addresses[i].endswith("PTR"):

                    try:
                        name_post = socket.gethostbyaddr(spec_addr[0])
                        answer_name_post = spec_addr[0] + ":PTR=" + name_post[0]+"\r\n"
                        response_post_check += answer_name_post
                    except socket.error:
                        response_post_check += ''
                
                # If type is not A or PTR skip line
                else:
                    response_post += ''
            
            # Sending Response 
            if response_post_check is '':
                send_err("HTTP/1.1 404 Not Found\r\n\r\n")
                conn.close()
            else:   
                response_post += response_post_check
                response_post = response_post.encode('utf-8')
                conn.sendall(response_post)
                conn.close()
            
        # Unknown Method
        else:
            send_err("HTTP/1.1 405 Method Not Allowed\r\n\r\n")
            conn.close()