from scapy.all import *
from scapy.layers.inet import IP, UDP, Ether
import sys

# ! iface name 会随着网卡在服务器中的情况改变
# 检查是否有足够的参数传入
if len(sys.argv) != 3:
    print("Usage: python3 send.py <iface_src> <ip_dst>")
    # e.g. python3 send.py ens4f1np1 192.168.0.212 (162:mlx5_3 => 164:mlx5_2)

    # e.g. python3 send.py ens5f0np0 192.168.3.216 (162:mlx5_1 <= 164:mlx5_0)

    # e.g. python3 send.py ens4f0np0 192.168.3.216 (162:mlx5_1 <= 164:mlx5_2)

    # e.g. python3 send.py ens4f0np0 192.168.5.210 (162:mlx5_0 <= 164:mlx5_2)

    sys.exit(1)

# 按顺序解析参数
iface_src = sys.argv[1] 
ip_src = get_if_addr(iface_src)
ip_dst = sys.argv[2] 

print(f"iface_src: {iface_src}, ip_src: {ip_src}, ip_dst: {ip_dst}")

# ! 抓包 sudo tcpdump -i <iface_src> -en host <ip_dst> -X
# get_if_hwaddr : 本机的 mac 地址; get_if_addr : 本机的 ip 地址; getmacbyip : 目标机器的 mac 地址
sendp(Ether(src=get_if_hwaddr(iface_src), dst=getmacbyip(ip_dst))/IP(src=ip_src, dst=ip_dst)/UDP()/Raw(load="===============12345678"), iface=iface_src)

# udp
# sendp(Ether(src=get_if_hwaddr(iface_src), dst=getmacbyip(ip_dst))/IP(src=ip_src, dst=ip_dst)/UDP(sport=12345, dport=100)/Raw(load="1=UDP==========12345678"), iface=iface_src)
# sendp(Ether(src=get_if_hwaddr(iface_src), dst=getmacbyip(ip_dst))/IP(src=ip_src, dst=ip_dst)/UDP(sport=101, dport=12345)/Raw(load="2=UDP==========12345678"), iface=iface_src)
# # tcp
# sendp(Ether(src=get_if_hwaddr(iface_src), dst=getmacbyip(ip_dst))/IP(src=ip_src, dst=ip_dst)/TCP(sport=12345, dport=102)/Raw(load="3=TCP==========12345678"), iface=iface_src)
# sendp(Ether(src=get_if_hwaddr(iface_src), dst=getmacbyip(ip_dst))/IP(src=ip_src, dst=ip_dst)/TCP(sport=103, dport=12345)/Raw(load="4=TCP==========12345678"), iface=iface_src)
# # ipv4 : test MODE_IPV4, 使用非指定的 src ipv4
# sendp(Ether(src=get_if_hwaddr(iface_src), dst=getmacbyip(ip_dst))/IP(src="1.2.3.4", dst=ip_dst)/Raw(load="5=IPv4==========12345678"), iface=iface_src)
# # test 交换机转发方式 : # !使用错误的 MAC，能根据正确的 IP 转发
# sendp(Ether(src=get_if_hwaddr(iface_src), dst="12:34:56:78:90:ab")/IP(src=ip_src, dst=ip_dst)/UDP(sport=12345, dport=100)/Raw(load="6=test=MAC=====12345678"), iface=iface_src)