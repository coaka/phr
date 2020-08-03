#This .py script is process 4 ns2 .tr files to plot CDF delay
from __future__ import division
from collections import Counter
from matplotlib.font_manager import FontProperties
import subprocess
import os
import sys
import collections
import matplotlib.pyplot as plt
import numpy as np

if len(sys.argv) < 3:
     print "Usage: Insert AODV, OLSR, GPSR and PHR trace files. to plot CDF delay"
     sys.exit(0)
snd =0
rec=0
pdr=0.0
drop=0
a=[]
kk={}
rece={}

pkts_id=[]
xx=[]
kktime=[]
kkktime=[]
sendid={}
sendidtime=[]
recetime=[]
kkk=[]
kkkk=[]
sub=0
count=0
pktid={}
receid={}
#f = open(sys.argv[1], 'r')
#f1 = open(sys.argv[2], 'r')
#readlines = f.readlines()
#readlines2 = f1.readlines()
with open(sys.argv[1], 'r') as f:
     for line in f:
         split1 = line.split()
         if split1[0]=='s' and  split1[18]=='AGT'and split1[34]=='cbr':
            #snd +=1
            #fid = split1[38]
        #pktid[int(snd)] = float(split1[1])
            sendid[int(split1[40])]= float(split1[2])
      
         if split1[0] == 'r' and  split1[18]=='AGT'and split1[34]=='cbr':

            rece[int(split1[40])]= float(split1[2])
        #rec +=1
sendid2={}
rece2={}
with open(sys.argv[2], 'r') as f1:
     for line2 in f1:
         split2 = line2.split()
         if split2[0]=='s' and  split2[18]=='AGT'and split2[34]=='cbr':
        
        #fid = split1[38]
        #pktid[int(snd)] = float(split1[1])
            sendid2[int(split2[40])]= float(split2[2])
      
         if split2[0] == 'r' and split2[18]=='AGT'and split2[34]=='cbr':

            rece2[int(split2[40])]= float(split2[2])
#to extract delay from gpsr trace file
sendid3={}
rece3={}
with open(sys.argv[3], 'r') as f3:
     for line3 in f3:
         split3 = line3.split()
         if split3[0]=='s' and  split3[18]=='AGT'and split3[34]=='cbr':
        
        #fid = split1[38]
        #pktid[int(snd)] = float(split1[1])
            sendid3[int(split3[40])]= float(split3[2])
      
         if split3[0] == 'r' and split3[18]=='AGT'and split3[34]=='cbr':

            rece3[int(split3[40])]= float(split3[2])
#to extract delay from phr trace file
sendid4={}
rece4={}
with open(sys.argv[4], 'r') as f4:
     for line4 in f4:
         split4 = line4.split()
         if split4[0]=='s' and  split4[18]=='AGT'and split4[34]=='cbr':
        
        #fid = split1[38]
        #pktid[int(snd)] = float(split1[1])
            sendid4[int(split4[40])]= float(split4[2])
      
         if split4[0] == 'r' and split4[18]=='AGT'and split4[34]=='cbr':

            rece4[int(split4[40])]= float(split4[2])

     
#dropfile = open('dropfile.dat', 'w')
#receivedfile = open('receivedfile.dat', 'w')                    


m=0
delay=0


pdfdelayaodv=[]
for j in sorted(sendid.keys()):
    if j in sorted(rece.keys()):
       delay = ((float(rece[j]) - float(sendid[j])))
       pdfdelayaodv.append(float(delay))
       delay = 0
       #receivedfile.write('\n')
delay2=0
pdfdelayolsr=[]
for n in sorted(sendid2.keys()):
    if n in sorted(rece2.keys()):
       delay2 = ((float(rece2[n]) - float(sendid2[n])))
       pdfdelayolsr.append(float(delay2))
       delay2 = 0
       #receivedfile.write('\n')

delay3=0
pdfdelaygpsr=[]
for ll in sorted(sendid3.keys()):
    if ll in sorted(rece3.keys()):
       delay3 = ((float(rece3[ll]) - float(sendid3[ll])))
       pdfdelaygpsr.append(float(delay3))
       delay3 = 0

delay4=0
pdfdelayphr=[]
for pp in sorted(sendid4.keys()):
    if pp in sorted(rece4.keys()):
       delay4 = ((float(rece4[pp]) - float(sendid4[pp])))
       pdfdelayphr.append(float(delay4))
       delay4 = 0  
n1, bins1, patches1=plt.hist(pdfdelayaodv, 50, range=[0, 0.5],histtype='step' , normed=True ,alpha=0.0, cumulative=True)
n2, bins2, patches2 =plt.hist(pdfdelayolsr, 50, range=[0, 0.5], histtype='step', normed=True, alpha = 0.0, cumulative=True)
n3, bins3, patches3 =plt.hist(pdfdelaygpsr, 50, range=[0, 0.5], histtype='step', normed=True, alpha = 0.0, cumulative=True)

n4, bins4, patches4 =plt.hist(pdfdelayphr, 50, range=[0, 0.5], histtype='step', normed=True, alpha = 0.0, cumulative=True)

#plt.figure(figsize=(20,20),dpi=300)
plt.xlim(xmax=0.5)
plt.ylim(ymax=1.03)
plt.xticks(fontsize = 16)
plt.yticks(fontsize = 16)
plt.plot(bins1[0:-1],n1,marker='o',linewidth=1.2,color='blue',label='aodv')
plt.plot(bins2[0:-1],n2,marker='x',linewidth=1.2,color='green',label='olsr')
plt.plot(bins3[0:-1],n3,marker='*',linewidth=1.2,color='red',label='gpsr')
plt.plot(bins4[0:-1],n4,marker='^',linewidth=1.2,color='black',label='phr')
plt.title("CDF of Delay with 200 calls on Leic map with high mobility profile")
plt.xlabel("Delay",fontsize=14)
plt.ylabel("CDF",fontsize=14)
plt.legend(loc='lower right')#,bbox_to_anchor=(0.5,-0.1), ncol=2)
plt.grid()
plt.savefig('delay_big_leic_200.pdf', bbox_incehs="tight")
plt.show()


