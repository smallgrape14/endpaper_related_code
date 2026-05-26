
# 存储服务器 抓包
sudo tcpdump -i eno8403 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8003

# 元数据服务器抓包
sudo tcpdump -i eno8403 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8005

# 结合GPUNetIO 的元数据抓包 162 server
sudo tcpdump -i mlx5_0 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8005
sudo tcpdump -i mlx5_0 -An -s 64 -w $(date +%Y%m%d_%H%M%S)_80.pcap

sudo tcpdump -i mlx5_0 -An -w $(date +%Y%m%d_%H%M%S)_80.pcap

# 发送方客户端抓包 164 server
sudo tcpdump -i mlx5_2 -An -s 64 -w $(date +%Y%m%d_%H%M%S)_80.pcap

sudo tcpdump -i mlx5_2 -An -w $(date +%Y%m%d_%H%M%S)_80.pcap
## 0328 begin
sudo tcpdump -i mlx5_2 -An -w aaa_$(date +%Y%m%d_%H%M%S)_164.pcap

sudo tcpdump -i tmfifo_net0 -w $(date +%Y%m%d_%H%M%S)_storage.pcap port 8008
sudo tcpdump -i tmfifo_net0 -w $(date +%Y%m%d_%H%M%S)_client.pcap port 43406

192.168.100.1:43406     192.168.100.1:8008      TIME_WAIT   - 
root@ubuntu-PowerEdge-R750xa:/home/ubuntu/xyp# netstat -tnpa | grep 8003
tcp        0      0 0.0.0.0:8003            0.0.0.0:*               LISTEN      1571174/beegfs-stor 
tcp        0      0 192.168.100.1:8003      192.168.100.1:35122     ESTABLISHED 1571174/beegfs-stor 
tcp        0      0 192.168.100.1:35122     192.168.100.1:8003      ESTABLISHED - 

dd if=/mnt/beegfs/test_1202/image_data.csv of=/home/ubuntu/xyp/beegfs_debug/outputfile_$(date +%Y%m%d_%H%M%S).csv bs=1024 count=1


## 162 server
192.168.0.216 
ens5f0np0
mlx5_0 

//通过指令 ibdev2netdev -v 查看网卡对应的名字
ifconfig

162 server:

mlx5_0 <=====> ens5f0np0 <========> 192.168.0.216

mlx5_2 <=====> ens4f0np0 <========> 192.168.5.210

164 server:

mlx5_0 <=====> ens5f0np0 <=====> 192.168.0.209

mlx5_2 <=====> ens4f0np0 <=====> 192.168.3.212

mlx5_3 <=====> ens4f1np1 <=====> 192.168.0.209

                                  .....
# 元数据服务器
sudo tcpdump -i ens5f0np0 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8005
sudo tcpdump -i mlx5_0 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8005

 mlx5_0
# 存储服务器
sudo tcpdump -i ens5f0np0 -w $(date +%Y%m%d_%H%M%S)_162_Receive_read_fashionMNIST_ens5f0np0.pcap port 8003
sudo tcpdump -i mlx5_0 -w $(date +%Y%m%d_%H%M%S)_162_Receive_read_fashionMNIST_mlx5_0.pcap port 8003




## 241218  164 -> 162 GPU_TCP  read 通信抓包

### xyp beegfs TCP
/* 164:mlx5_2:BF-3(ens4f0np0)---162:mlx5_0:CX-5(ens5f0np0) */
/* 192.168.3.212:12345 => 192.168.3.216:8003 */

(164) 192.168.3.212 ==> 192.168.3.216 (162) 


162: ens5f0np0 192.168.3.216 mlx5_0 

164: ens4f0np0 192.168.3.212 mlx5_2


### 抓包

162 Server:
sudo tcpdump -i mlx5_0 -w $(date +%Y%m%d_%H%M%S)_read_GPU_xyp_TCP_162Receive_mlx.pcap

sudo tcpdump -i ens5f0np0 -w $(date +%Y%m%d_%H%M%S)_read_GPU_xyp_TCP_162Receive.pcap
sudo tcpdump -i ens5f0np0 -w $(date +%Y%m%d_%H%M%S)_read_CPU_xyp_TCP_162Receive.pcap



164 Client:
sudo tcpdump -i mlx5_2 -w $(date +%Y%m%d_%H%M%S)_read_GPU_xyp_TCP_164_Send_mlx.pcap
sudo tcpdump -i mlx5_2 -w $(date +%Y%m%d_%H%M%S)_read_GPUNetIO.pcap
sudo tcpdump -i mlx5_2 -w $(date +%Y%m%d_%H%M%S)_164_GPUNetIO.pcap

sudo tcpdump -i ens4f0np0 -w $(date +%Y%m%d_%H%M%S)_read_GPU_xyp_TCP_164_Send.pcap



## 0126 测试flexio  packet_processor 功能



### 162 server
服务器	服务器IP	网口	网口IP	MAC
tl_162	10.26.42.230	mlx5_0:BF-3(ens4f0np0)	192.168.5.210	a0:88:c2:31:d8:66
tl_162	10.26.42.230	mlx5_1:BF-3(ens4f1np1)	192.168.0(3).216	a0:88:c2:31:d8:67
### 164 Server
服务器	服务器IP	网口	网口IP	MAC
tl_164	10.26.42.232	mlx5_0:CX-5(ens5f0np0)	192.168.0(5).209	98:03:9b:99:6d:f2
tl_164	10.26.42.232	mlx5_2:BF-3(ens4f0np0)	192.168.3.212	a0:88:c2:31:d6:9e
tl_164	10.26.42.232	mlx5_3:BF-3(ens4f1np1)	(未插线)192.168.4.213	a0:88:c2:31:d6:9f
### 运行指令

(162:mlx5_1 => 164:mlx5_2) //不用这个
python3 send.py ens4f1np1 192.168.3.212 

而是这个
(162:mlx5_1 => 164:mlx5_0) 
python3 send.py ens4f1np1 192.168.0.209
抓包：
sudo tcpdump -i ens4f1np1 -en host 192.168.0.209 -X  -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap

python3 send.py ens4f1np1 127.0.0.1

sudo tcpdump -i ens4f1np1 -en host 127.0.0.1 -X -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap
抓包：
<!-- sudo tcpdump -i <iface_src> -en host <ip_dst> -X -->
sudo tcpdump -i ens4f1np1 -en host 192.168.3.212 -X

164-BF3:ifconfig
en3f0pf0sf0: 0e:4a:46:24:7d:f0
en3f1pf1sf0: d6:61:16:ee:68:83
enp3s0f0s0: 02:96:3d:85:0a:2f
enp3s0f1s0: 02:29:16:85:7c:51 
ens4f0np0 mlx5_2
# 162:
mlx5_0 ens4f0np0 192.168.5.210 a0:88:c2:31:d8:66
mlx5_1 ens4f1np1 192.168.0.216 a0:88:c2:31:d8:67
# 164上
ens4f1np1 192.168.0.209 mlx5_0 98:03:9b:99:6d:f2
ens4f0np0 192.168.3.212 mlx5_2 a0:88:c2:31:d6:9e

sudo tcpdump -i ens4f1np1 -X -w $(date +%Y%m%d_%H%M%S)_meta.pcap


162 抓包
sudo tcpdump -i ens4f1np1 -X -w $(date +%Y%m%d_%H%M%S)_162S_164C_164D_Mlx1_2_TCP.pcap
sudo tcpdump -i mlx5_1 -X -w $(date +%Y%m%d_%H%M%S)_162S_164C_164D_Mlx1_2_Rdma.pcap


sudo tcpdump -i ens4f0np0 -en host 192.168.0.209 -X -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap
sudo tcpdump -i ens4f0np0 -en host 192.168.0.209 -X  


成功抓包：
```sh
python3 send2.py 
sudo tcpdump -i ens4f0np0 -en -X  
```

尝试 捕捉RDMA 也成功
```sh
python3 send_rdma.py 
sudo tcpdump -i mlx5_0 -en -X  
```

```sh
# 162 
sudo tcpdump -i mlx5_0 -en host 192.168.0.209 -X 
sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap

sudo tcpdump -i ens4f0np0 -en -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap

sudo tcpdump -i mlx5_0 -en host 192.168.5.209 -X -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap


# TCP 
sudo tcpdump -i ens4f0np0 -en -w $(date +%Y%m%d_%H%M%S)_test_tcp_application.pcap

# RDMA
sudo tcpdump -i mlx5_1 -en -w $(date +%Y%m%d_%H%M%S)_test_flexio_packet_processor.pcap


tcpreplay -i ens4f1np1 test_first.pcap

sudo tcpdump -i mlx5_1 -en -w $(date +%Y%m%d_%H%M%S)_debug_GPUNetIO_162_.pcap

sudo tcpdump -i mlx5_1 -en -w $(date +%Y%m%d_%H%M%S)_debug_GPUNetIO_162_.pcap
```

## 250227 debug GPUNetIO
```sh
# RDMA
sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_debug_GPUNetIO_164_.pcap

sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_164_mlx5_2.pcap
sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_164_mlx5_0.pcap

sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_164_mlx5_2.pcap
sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_CpuMultiThread_CPPcode_test_dir_4KB_t1_mlx5_0.pcap
sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_CpuMultiThread_CPPcode_test_dir_4KB_t1_mlx5_0.pcap


sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_162.pcap

sudo tcpdump -i mlx5_3 -en -w $(date +%Y%m%d_%H%M%S)_Gds_164.pcap



sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_CpuMultiThread_CPPcode_image_32KB_10428_t1_mlx5_0.pcap


sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_test_32kB_10428_b1_t1_mlx5_2.pcap

sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_test_4kB_10428_b4_t16_mlx5_2.pcap

sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_test_1MB_1024_mlx5_2.pcap

sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_164.pcap

sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_b1_t1_mlx5_2.pcap


sudo tcpdump -i mlx5_2 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO_mlx5_2.pcap

sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_GPUNetIO__mlx5_0.pcap
sudo tcpdump -i ens4f0np0 -en -w $(date +%Y%m%d_%H%M%S)_Baseline_ens4f0np0.pcap



sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_Baseline__mlx5_0.pcap
sudo tcpdump -i ens4f1np1 -en -w $(date +%Y%m%d_%H%M%S)_Baseline_ens4f1np1.pcap

sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_NFS_mlx5_0.pcap
sudo tcpdump -i ens4f1np1 -en -w $(date +%Y%m%d_%H%M%S)_NFS_209.pcap

ens4f0np0 mlx5_2

# TCP 
ens4f0np0 192.168.3.212 mlx5_2
ens5f0np0 192.168.0.209
tmfifo_net0 192.168.100.1
sudo tcpdump -i mlx5_0 -en -w $(date +%Y%m%d_%H%M%S)_mlx5_0.pcap
sudo tcpdump -i ens5f0np0 -en -w $(date +%Y%m%d_%H%M%S)_ens5f0np0.pcap
sudo tcpdump -i tmfifo_net0 -en -w $(date +%Y%m%d_%H%M%S)_tmfifo_net0.pcap


sudo tcpdump -i eno8403 -en -w $(date +%Y%m%d_%H%M%S)_test_tcp_application.pcap
telnet 192.168.5.210 8005

目录
0-67C86A7D-6
文件
0-67C86ABC-6


infiniband.bth.psn == 5279198


## 0223
//通过指令 ibdev2netdev -v 查看网卡对应的名字
ifconfig

162 server:

mlx5_0 <=====> ens5f0np0 <========> 192.168.0.216

mlx5_2 <=====> ens4f0np0 <========> 192.168.5.210

164 server:


mlx5_2 <=====> ens4f0np0 <=====> 192.168.3.212

mlx5_3 <=====> ens4f1np1 <=====> 192.168.0.209
sudo tcpdump -i mlx5_2 -w $(date +%Y%m%d_%H%M%S)_164_GPUNetIO.pcap
```

