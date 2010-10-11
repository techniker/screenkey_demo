//# Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
//# This code is licensed under GNU/ GPL

#!/usr/bin/python

import os;
import struct;
import sys;
import math;


# Generate a header file intended for use on the screenkey
# from a bmp

df = open(sys.argv[1]);

(type, size, offset) = struct.unpack("<2sI4xI",df.read(14));
sh = struct.unpack("<IiiHHII8xII",df.read(40));

colors = []
for i in range(0, sh[7]):
	colors.append(struct.unpack("<BBBx", df.read(4)))


data = df.read(sh[6]);

df.close()


def data_1bit(data):	
	sdata = "";
	accuum = 0;
	cnt = 0;
	for i in range(0, 24):
		sdata += "\t"
		for j in range(0,36):
			bit = 35 - j
			dpos = 8 * (23-i) + bit/8 
			bpos = 7-(bit % 8);
			accuum >>= 1
			if (((ord(data[dpos]) >> bpos) & 0x1) ^ 0x1):
				accuum |= 0x80 
			cnt = cnt + 1;
			if (cnt == 8):
				sdata += "%#02x, " % accuum
				accuum = 0;
				cnt = 0
		sdata += "\n"
	return sdata

rv= data_1bit(data);

fname = sys.argv[1]
fname = fname.replace(".bmp",".h");
bname = fname.replace(".h","_img");
fo = open(fname, "w")
fo.write("uint8_t %s[108] = {\n" %bname);
fo.write(rv)
fo.write("};\n");
fo.close()
