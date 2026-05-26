from scapy.all import *
from scapy.layers.inet import IP, UDP, Ether

# sendp(Ether(src="a0:88:c2:31:d8:66", dst='98:03:9b:99:6d:f2')/IP()/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")
# sendp(Ether(src="a0:88:c2:31:d6:9e", dst='a0:88:c2:31:d8:66')/IP(src=192.168.3.212,dst=192.168.5.210)/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")
# sendp(Ether(src="a0:88:c2:31:d6:9e", dst='a0:88:c2:31:d8:66')/IP()/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")
sendp(Ether(src="a0:88:c2:31:d6:9e", dst='a0:88:c2:31:d8:67')/IP()/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")#成功 BF-3

#成功 BF-3
# sendp(Ether(src="a0:88:c2:31:d6:9e", dst='a0:88:c2:31:d8:67')/IP(src=192.168.3.212,dst=192.168.3.216)/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")#成功 BF-3

# sendp(Ether(src="a0:88:c2:31:d8:66", dst='d6:61:16:ee:68:83')/IP()/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")#成功 BF-3
# sendp(Ether(src="a0:88:c2:31:d8:66", dst='d6:61:16:ee:68:83')/IP()/UDP()/Raw(load="===============12345678"), iface="ens4f0np0")#成功 BF-3