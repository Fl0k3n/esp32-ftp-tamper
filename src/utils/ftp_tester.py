import ftplib
import socket
import time


# sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# sock.connect(("192.168.0.80", 21))

# print('connected')


# sock.send("USER testUser\r\n".encode('ascii'))

# print('sent')

# resp = sock.recv(100).decode('ascii')

# print(f'got response: {resp}')

# input("press to disconnect")

ftp = ftplib.FTP('192.168.0.80')
ftp.login("test", "passtest")
