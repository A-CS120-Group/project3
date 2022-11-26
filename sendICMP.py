import socket
import os
import errno


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


FIFO_pipe = "./cpp2py"
try:
    os.mkfifo(FIFO_pipe)
except OSError as oe:
    print(oe)
    if oe.errno != errno.EEXIST:
        raise

while True:
    pipe = open(FIFO_pipe, "r")
    print(1)
    message = pipe.readline()
    message = message.split(" ")
    print(message)
    if len(message) == 2:
        pipe.close()
        continue
    mode = message[0]
    dst_ip_addr = message[1]
    identifier = message[2]
    seq = message[3]
    icmp_payload = message[4].strip("\n")
    # mode = "3"  # or 4
    # dst_ip_addr = "192.168.1.101"
    # identifier = "6566"
    # seq = "14"
    # icmp_payload = "61626364"

    seq = bytearray.fromhex("{:04x}".format(int(seq)))
    icmp_payload = bytearray.fromhex(icmp_payload)
    identifier = bytearray.fromhex(identifier.rjust(4, '0'))

    icmp_request_head = bytearray(b"\x08\x00\x00\x00\x00\x00\x00\x00")
    icmp_reply_head = bytearray(b"\x00\x00\x00\x00\x00\x00\x00\x00")
    ip_payload = bytearray()
    if mode == "3":
        ip_payload = icmp_request_head + icmp_payload
    elif mode == "4":
        ip_payload = icmp_reply_head + icmp_payload
    else:
        print(f"Wrong format!: {mode}")
        pipe.close()
        continue
        # discard
    ip_payload[4:6] = identifier
    ip_payload[6:8] = seq
    ip_payload[2:4] = int(calculateChecksum(ip_payload)).to_bytes(length=2, byteorder='big')
    print(ip_payload)

    icmp = socket.getprotobyname('icmp')
    ping_reply = socket.socket(socket.AF_INET, socket.SOCK_RAW, icmp)

    ping_reply.sendto(bytes(ip_payload), (dst_ip_addr, 80))
    pipe.close()
