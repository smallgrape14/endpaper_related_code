from scapy.all import IP, TCP, sr1

def send_tcp_syn_packet(ip, port):
    try:
        # 创建 IP 层
        ip_layer = IP(dst=ip)
        
        # 创建 TCP 层，设置 SYN 标志
        tcp_layer = TCP(dport=port, flags="S")  # "S" 表示 SYN 标志
        
        # 构建完整的数据包
        packet = ip_layer / tcp_layer
        
        # 发送数据包并等待响应（可选）
        response = sr1(packet, timeout=2, verbose=0)  # timeout 设置为 2 秒
        
        # if response:
        #     print(f"收到响应: {response.summary()}")
        # else:
        #     print("未收到响应")
    except Exception as e:
        print(f"发生错误: {e}")

# 使用示例
ip = "192.168.3.216"  # 替换为目标 IP 地址
port = 12345         # 替换为目标端口号
send_tcp_syn_packet(ip, port)