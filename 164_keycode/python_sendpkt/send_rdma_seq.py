import sys
from scapy.all import Ether, IP, UDP, sendp, Packet, ByteField, ShortField, IntField, StrFixedLenField

# 定义 RoCEv2 数据包的结构
class RoCEv2(Packet):
    name = "RoCEv2"
    fields_desc = [
        ByteField("opcode", 0),  # 1字节的操作码
        ByteField("flags", 0),  # 1字节的标志
        ShortField("rsvd", 0),  # 2字节的保留字段
        IntField("qpn", 0),     # 4字节的队列对号
        IntField("psn", 0),     # 4字节的包序列号
        StrFixedLenField("data", "", length=20)  # 固定长度为20的字符串数据字段
    ]

# 检查传入参数
if len(sys.argv) > 1 and sys.argv[1].isdigit():
    pkt_num = int(sys.argv[1])
    payload = f"this is {pkt_num}th pkt".encode()  # 将字符串转换为字节
    print(f"Using payload: {payload}")
else:
    payload = b"this is 1th pkt"  # 默认负载
    pkt_num = 1
    print(f"Using default payload: {payload}")

# 创建以太网层
eth = Ether(dst="a0:88:c2:31:d8:67", src="a0:88:c2:31:d6:9e")

# 创建 IP 层
ip = IP(dst="192.168.3.216", src="192.168.3.212")

# 创建 UDP 层，端口号为 4791
udp = UDP(sport=12345, dport=4791)

# 创建 RoCEv2 层
rocev2 = RoCEv2(opcode=0x1, flags=0x2, qpn=0x123456, psn=0x789ABC, data=payload)

# 组装数据包
packet = eth / ip / udp / rocev2

try:
    # 发送数据包
    sendp(packet, iface="ens4f0np0", verbose=1)
    print("Packet sent successfully. Assuming the packet was delivered.")
except Exception as e:
    print(f"An error occurred while sending the packet: {e}")

    
# try:
#     # 发送数据包
#     sent_packets = sendp(packet, iface="ens4f0np0", verbose=1)
    
#     # 检查返回值是否为 None
#     if sent_packets is None:
#         print("Failed to send packet: sendp returned None.")
#     elif sent_packets > 0:
#         print(f"Packet sent successfully. Packets sent: {sent_packets}")
#     else:
#         print("Failed to send packet: sendp returned a non-positive value.")
# except Exception as e:
#     print(f"An error occurred while sending the packet: {e}")