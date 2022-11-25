import errno
import socket
import time
import os
from scapy.all import *


# This file is for Node2 only


def calculateChecksum(data):
    checksum = 0
    count = (len(data) // 2) * 2
    i = 0

    while i < count:
        temp = data[i + 1] * 256 + data[i]
        checksum = checksum + temp
        checksum = checksum & 0xffffffff
        i = i + 2

    if i < len(data):
        checksum = checksum + data[len(data) - 1]
        checksum = checksum & 0xffffffff

    # 32-bit to 16-bit
    checksum = (checksum >> 16) + (checksum & 0xffff)
    checksum = checksum + (checksum >> 16)
    checksum = ~checksum
    checksum = checksum & 0xffff

    checksum = checksum >> 8 | (checksum << 8 & 0xff00)
    return checksum


currentIP = "192.168.1.101"

FIFO_pipe = "./pipe"
try:
    os.mkfifo(FIFO_pipe)
except OSError as oe:
    print(oe)
    if oe.errno != errno.EEXIST:
        raise

print("Open")

while True:
    pipe = open(FIFO_pipe, "w")
    print(1)
    pkt = sniff(filter='icmp', count=1)
    ip_payload = bytes(pkt[0]['IP'].payload)
    ip_payload_str = ''.join('{:02x}'.format(x)
                             for x in bytearray(bytes(pkt[0]['IP'].payload)))
    src_ip_addr = str(pkt[0]['IP'].src)
    dst_ip_addr = str(pkt[0]['IP'].dst)

    print(src_ip_addr)
    print(dst_ip_addr)
    print(ip_payload_str)

    # hello
    # TODO: if not ping reply and "hello" in payload
    if "68656c6c6f" not in ip_payload_str:
        print("Discard!")
        pipe.close()
        continue

    pipe.write(src_ip_addr + " " + dst_ip_addr + " " + ip_payload_str)
    pipe.write('\n')

    # TODO: wait for response from C++

    icmp = socket.getprotobyname('icmp')
    ping_reply = socket.socket(socket.AF_INET, socket.SOCK_RAW, icmp)
    icmp_raw = bytearray(ip_payload)
    icmp_raw[0:1] = b'\x00'
    icmp_raw[2:4] = b'\x00\x00'
    checksum = int(calculateChecksum(icmp_raw)).to_bytes(length=2, byteorder='big')
    icmp_raw[2:4] = checksum
    ping_reply.sendto(bytes(icmp_raw), (src_ip_addr, 80))

    pipe.close()

print("Close")
