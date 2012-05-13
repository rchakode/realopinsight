#!/usr/bin/python
import sys
import fileinput
import re

file = open(sys.argv[1])
line = file.read()
#lines = file.readlines(1073741824)
line.replace(".*<BODY >(.*)", "<BODY>")
file.close()
