import socket

def send_udp_packet(ip, port, payload):
    try:
        # 创建一个 UDP 套接字
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            # 设置目标地址和端口
            target_address = (ip, port)
            
            # 发送数据
            sock.sendto(payload.encode('utf-8'), target_address)
            print(f"已向 {ip}:{port} 发送数据: {payload}")
            
            # 接收服务器响应（可选）
            # response, address = sock.recvfrom(1024)  # 设置接收缓冲区大小
            # print(f"从 {address} 收到响应: {response.decode('utf-8')}")
    except Exception as e:
        print(f"发生错误: {e}")

# 使用示例
ip = "192.168.3.216"  # 替换为目标 IP 地址
port = 12345      # 替换为目标端口号
payload = "123456789"  # 要发送的数据
send_udp_packet(ip, port, payload)