from scapy.all import *

# This file is for Node2 only

FIFO_pipe = "./py2cpp"
try:
    os.mkfifo(FIFO_pipe)
except OSError as oe:
    print(oe)
    if oe.errno != errno.EEXIST:
        raise

while True:
    pipe = open(FIFO_pipe, "w")
    print("Open")
    pkt = sniff(filter='icmp', count=1)
    ip_payload = bytes(pkt[0]['IP'].payload)
    ip_payload_str = ''.join('{:02x}'.format(x)
                             for x in bytearray(bytes(pkt[0]['IP'].payload)))
    src_ip_addr = str(pkt[0]['IP'].src)
    dst_ip_addr = str(pkt[0]['IP'].dst)

    print(src_ip_addr)
    print(dst_ip_addr)
    print(ip_payload_str)
    # first two hex
    icmp_type = ip_payload_str[0:2]
    if icmp_type == "08":
        icmp_type = "request"
    elif icmp_type == "00":
        icmp_type = "reply"
    else:
        print("Discard due to invalid icmp type!")
        pipe.close()
        continue

    # I don't know what the code do, just take it.
    icmp_code = ip_payload_str[2:4]
    # I trust the checksum is correct. QwQ
    icmp_checksum = ip_payload_str[4:8]

    icmp_identifier = ip_payload_str[8:12]
    icmp_seq = ip_payload_str[12:16]
    icmp_seq = str(int(icmp_seq, 16))

    if len(ip_payload_str) == 16:
        print("Discard, I don't like packages with no payload!")
        pipe.close()
        continue
    icmp_payload = ip_payload_str[16:]

    # hello
    if "68656c6c6f" not in icmp_payload:
        print("Discard due to wrong payload")
        pipe.close()
        continue

    pipe.write(src_ip_addr + " " + dst_ip_addr + " " + icmp_type + " " +
               icmp_identifier + " " + icmp_seq + " " + icmp_payload)
    pipe.write('\n')
    pipe.close()
