#!/usr/bin/env python

import sys
import re

alloc = {}

cnt = 0
acnt = 0
fcnt = 0
if __name__ == '__main__':
	with open(sys.argv[1],"r") as f:
		lines = f.readlines()
	for ln in lines:
		cnt+=1
		if "make_flatten_pointer_call" in ln:
			acnt+=1
			addr = ln.split("=>")[1].strip()
			alloc[int(addr,16)] = acnt
		elif "free_flatten_pointer_call" in ln:
			fcnt+=1
			addr = ln.split("=>")[1].strip()
			del alloc[int(addr,16)]
	print alloc