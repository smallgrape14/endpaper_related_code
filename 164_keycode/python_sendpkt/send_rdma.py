import sys
import time
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

# 创建以太网层
# eth = Ether(dst="a0:88:c2:31:d8:67", src="a0:88:c2:31:d6:9e")
eth = Ether(dst="a0:88:c2:31:d8:66", src="a0:88:c2:31:d6:9e")

# 创建 IP 层
# ip = IP(dst="192.168.3.216", src="192.168.3.212")
ip = IP(dst="192.168.3.216", src="192.168.5.210")

# 创建 UDP 层，端口号为 4791
udp = UDP(sport=12345, dport=4791)

# 循环发送 100 个数据包
for pkt_num in range(1, 101):
    # 每次发送前暂停 1 秒
    # time.sleep(5)
    # 动态生成负载内容
    payload = f"this is {pkt_num}th pkt".encode()  # 将字符串转换为字节
    print(f"Sending packet {pkt_num}: Payload: {payload}")

    # 创建 RoCEv2 层
    rocev2 = RoCEv2(opcode=0x1, flags=0x2, qpn=0x123456, psn=0x789ABC, data=payload)

    # 组装数据包
    packet = eth / ip / udp / rocev2

    try:
        # 发送数据包
        sendp(packet, iface="ens4f0np0", verbose=1)
        print(f"Packet {pkt_num} sent successfully. Assuming the packet was delivered.")
    except Exception as e:
        print(f"An error occurred while sending packet {pkt_num}: {e}")

    # # 每次发送后暂停 1 秒
    time.sleep(1)

print("All packets sent.")