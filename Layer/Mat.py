#!/usr/bin/python
# -*- coding: utf-8 –*-
import os
import glob
list=glob.glob(os.path.join("./", "*.h"))
for  i in list:
    print(i)
    outfilename='outfile\\%s' %i
    print(outfilename)
    file1 = open(i,"r")
    file2 = open(outfilename,"w")
    s = file1.read()
    w = file2.write(s)
    file1.close()
    file2.close()
	
list=glob.glob(os.path.join("./", "*.cpp"))
for  i in list:
    print(i)
    outfilename='outfile\\%s' %i
    print(outfilename)
    file1 = open(i,"r")
    file2 = open(outfilename,"w")
    s = file1.read()
    w = file2.write(s)
    file1.close()
    file2.close()