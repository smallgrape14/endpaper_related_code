from scapy.all import Ether, IP, UDP, sendp, Packet, ByteField, ShortField, IntField, StrFixedLenField

# 定义 RoCEv2 数据包的结构
# class RoCEv2(Packet):
#     name = "RoCEv2"
#     fields_desc = [
#         # 添加 RoCEv2 数据包的字段
#         # 例如：字段名、字段类型、默认值
#         # 这里只是一个示例，具体字段需要根据 RoCEv2 协议定义
#         ByteField("opcode", 0),
#         ByteField("flags", 0),
#         ShortField("rsvd", 0),
#         IntField("qpn", 0),
#         IntField("psn", 0),
#         # IntField("data", 0)
#         StrField("data", "")    # 字符串数据字段
#     ]


# 定义 RoCEv2 数据包的结构
class RoCEv2(Packet):
    name = "RoCEv2"
    fields_desc = [
        ByteField("opcode", 0),  # 1字节的操作码
        ByteField("flags", 0),  # 1字节的标志
        ShortField("rsvd", 0),  # 2字节的保留字段
        IntField("qpn", 0),     # 4字节的队列对号
        IntField("psn", 0),     # 4字节的包序列号
        StrFixedLenField("data", "", length=11)  # 固定长度为6的字符串数据字段
    ]

# 创建以太网层
# eth = Ether(dst="a0:88:c2:31:d8:66", src="d6:61:16:ee:68:83")
# 162 mlx5_0 ==> 164 mlx5_0 
# eth = Ether(src="a0:88:c2:31:d8:66", dst="98:03:9b:99:6d:f2")
# 164 mlx5_0 ==> 162 mlx5_0
# eth = Ether(dst="a0:88:c2:31:d8:66", src="98:03:9b:99:6d:f2")
# 164 mlx5_2 ==> 162 mlx5_0

# eth = Ether(dst="a0:88:c2:31:d8:66", src="a0:88:c2:31:d6:9e")

# 164 mlx5_2 ==> 162 mlx5_1
eth = Ether(dst="a0:88:c2:31:d8:67", src="a0:88:c2:31:d6:9e")
# ens4f0np0 192.168.3.212 mlx5_2 a0:88:c2:31:d6:9e



# 162:
# mlx5_0 ens4f0np0 192.168.5.210 a0:88:c2:31:d8:66
# mlx5_1 ens4f1np1 192.168.0.216 a0:88:c2:31:d8:67
# # 164上
# ens5f0np0 192.168.0.209 mlx5_0 98:03:9b:99:6d:f2
# ens4f0np0 192.168.3.212 mlx5_2 a0:88:c2:31:d6:9e
# 创建 IP 层
# ip = IP() 
# ip=IP(dst="192.168.5.210", src="192.168.5.209")
# ip=IP(dst="192.168.5.210", src="192.168.3.212")
ip=IP(dst="192.168.3.216", src="192.168.3.212")


# 创建 UDP 层，端口号为 4791
udp = UDP(sport=12345, dport=4791)

# 创建 RoCEv2 层
# rocev2 = RoCEv2(opcode=0x1, flags=0x2, qpn=0x123456, psn=0x789ABC, data=0x12345678)
rocev2 = RoCEv2(opcode=0x1, flags=0x2, qpn=0x123456, psn=0x789ABC, data="=1234567==")


# 组装数据包
packet = eth / ip / udp / rocev2

# 发送数据包
# sendp(packet, iface="ens4f0np0")
# sendp(packet, iface="ens5f0np0")
sendp(packet, iface="ens4f0np0")




