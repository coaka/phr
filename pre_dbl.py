#This .py script is format the delived & dropped packet and generates" 
from __future__ import division
#from collections import Counter
import subprocess
import os
import gzip
import sys
import re
import operator
if len(sys.argv)<2:
    print "Usage: pass ns2 .tr file as a parameter and print out the results as a file"
    print "This .py file process ns2 .tr file and print out delivered/dropped packet that needed later to get probability of DBL"
    sys.exit(0)
snd =0
rec=0
pdr=0.0
drop=0
a=[]
kk=[]
rece=[]
dbl=[]
DBL=[]
pkts_id=[]
xx=[]
sub=0
count=0
frm={}
drop={}
received={}
dropped={}

f = open(sys.argv[1], 'r')
readlines = f.readlines()
for line in readlines:
    split1 = line.split()
    
    
    if split1[0] == 'r' and split1[18]=='AGT'and split1[34]=='cbr':
        pkt_id = split1[40]
    
        fid = split1[38]
        received[int(pkt_id)] = [fid, "Received", float(split1[2])]
        rece.append(int(split1[40])) 
    
    if split1[0] == 'd' and split1[34] == 'cbr'and split1[20]!='END' and (int(split1[40]) not in rece) and (int(split1[40]) not in kk) :
        dpkt_id = split1[40]
        dfid = split1[38]
        dropped[int(dpkt_id)] = [dfid, "Dropped", float(split1[2])]
 
        kk.append(int(split1[40]))   
         
aa={}
bb={}
aa = dropped.copy()
aa.update(received)

bb=sorted(aa.items(), key=lambda element:element[0])

for k in bb:
    #print k[0], k[1]
    print k

