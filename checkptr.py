#!/usr/bin/env python

import sys
import re

ptrre = re.compile(".*\|\s+(\d+)\s<-\s+(\d+)")
ptrm = {}

if __name__ == '__main__':
	with open(sys.argv[1],"r") as f:
		lines = f.readlines()
	for ln in lines:
		m = ptrre.match(ln)
		if m:
			orig = int(m.groups()[0])
			new = int(m.groups()[1])
			if orig in ptrm:
				if new!=ptrm[orig]:
					print "Different fixup for pointer %d"%(orig)
			else:
				ptrm[orig] = new
	print len(ptrm)