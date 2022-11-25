import errno
import socket
import time
import os
from scapy.all import *

# This file is for Node2 only

# TODO: read config file

FIFO_pipe = "./pipe"
try:
    os.mkfifo(FIFO_pipe)
except OSError as oe:
    print(oe)
    if oe.errno != errno.EEXIST:
        raise

a = 0
print("Open")
# with open(FIFO_pipe, "a") as pipe:
while True:
    pkt = sniff(filter='icmp', count=1)
    ip_payload = bytes(pkt[0]['IP'].payload)
    src_ip_addr = str(pkt[0]['IP'].src)
    dst_ip_addr = str(pkt[0]['IP'].dst)
    # pipe.write(src_ip_addr)
    # pipe.write(dst_ip_addr)
    # pipe.write(pkt)
    print(ip_payload)
    print(src_ip_addr)
    print(dst_ip_addr)

    icmp = socket.getprotobyname('icmp')
    ping_reply = socket.socket(socket.AF_INET, socket.SOCK_RAW, icmp)
    icmp_raw = bytearray(ip_payload)
    icmp_raw[0:1] = b'\x00'
    icmp_raw[2:4] = b'\x00\x00'
    checksum = # TODO: here
    icmp_raw[2:4] = checksum
    ping_reply.sendto(bytes(icmp_raw), (src_ip_addr, 80))

print("Close")
