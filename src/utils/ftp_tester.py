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
print(ftp.login("username", "password"))
print(ftp.cwd("."))
print(ftp.cwd("/dev/dev2/xd"))
print(ftp.cwd(".."))
print(ftp.cwd(".."))
print(ftp.cwd(".."))
print(ftp.cwd(".."))
print(ftp.cwd(".."))
print(ftp.cwd("xddd/lol"))
print(ftp.cwd("/"))
print(ftp.cwd("/hello/here"))
print(ftp.sendcmd("REIN"))
time.sleep(2)
# print(ftp.cwd("xd")) should return 530 not logged in
print(ftp.login("username", "password"))
print(ftp.cwd("hello"))
print(ftp.quit())
time.sleep(3)
