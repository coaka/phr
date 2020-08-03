#This .py script is process the output file of pre_dbl.py and print out the probability of dbl
from __future__ import division
from collections import Counter
import subprocess
import os
import sys
import collections
import re
import operator
if len(sys.argv)<2:
    print "Usage: pass pre_dbl.py output file as a parameter to get probaibilty of dbl"
    sys.exit(0)
current = 0
curpair=[]
pairs=[]
f = open(sys.argv[1], 'r')
lines = f.readlines()
for line in lines:
    split1 = line.split()
    fid = int(re.search("\d+",split1[1]).group())
    status =  str(re.search("\w+",split1[2]).group())
    t = (fid, status)
    pairs.append(t)
#print pairs    
output = {}  # (f,t) -> [dbl, dbl, ...] (f:from t: to)
last = {}    # (f,t) -> [dropped/received]

for (fid, s) in pairs:
    #ft = (f,t)

    if fid in output:    # if a pair exists in the output dict
        if s == 'Dropped':   # D->D or R->D
            output[fid][-1] += 1
        elif last[fid] == 'Dropped':  # D->R
            output[fid].append(0)
    else:
        output[fid] = [0]
        if s == 'Dropped':
            output[fid]=[1]
    last[fid] = s

alldbls = []
for x in output:
   if output[x][-1] == 0:
      del output[x][-1]
   #print x,"---", output[x]
   alldbls = alldbls + output[x]
#sys.exit(0)   
#print sorted(alldbls), len(alldbls)

a = Counter(alldbls)

DD=[]
DD = sorted(a.items())
A=0
#print DD
L1=0
for L in DD:
   L1 =L[1]+L1
#print L1
for k in DD:  #calculate the probability
    K=k[1]/L1
    print '{0} {1:.5f}'.format(k[0], K)
    A = A+K
#print A
