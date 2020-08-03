#This .py file process ns2 .tr file and print out Packet Delivery Ratio(PDR)
from __future__ import division
from collections import Counter
import subprocess
import os
import gzip
import os.path
import sys
import collections

if len(sys.argv)<2:
    print "Usage: pass ns2 .tr file as a parameter to get PDR"
    print "This .py file process ns2 .tr file and print out Packet Delivery Ratio(PDR)"
    sys.exit(0)
snd =0
rec=0
pdr=0.0
drop=0
a=[]
kk=[]
rece=[]
reason=[]
k=0
sum1=0
aaa=0
RET=0
ARP=0
DUP=0
NRTE=0
CBK=0
sent={}
receive={}
tcp_snt={}
tcp_rcv={}
with open(sys.argv[1], 'r') as f:
  try:   
     for readlines in f:
         split1 = readlines.split()
         if split1[0]=='s' and split1[18]=='AGT'and split1[34]=='cbr':
            snd +=1
            sent[split1[40]]='s'
	 elif split1[0]=='s' and split1[18]=='AGT'and split1[34]=='tcp':
            tcp_snt[split1[40]]='s'

         elif split1[0] == 'r' and split1[18]=='AGT'and split1[34]=='cbr':
	    receive[split1[40]]='r' 
            rec +=1
	 elif split1[0] == 'r' and split1[18]=='AGT'and split1[34]=='tcp':
            tcp_rcv[split1[40]]='r'

            
         elif split1[0] == 'd' and split1[34] == 'cbr'and split1[20]!='END'  and (split1[40] not in kk) and (split1[40] not in rece):
	    kk.append(split1[40])        
	    drop +=1
  except: IndexError,  #drop1.write('{0} \n'.format(int(float(split1[1]))))

#print 'Number of (application layer) sent packets is: ', snd
#print 'Number of (application layer) received packets is: ', rec
#print 'Number of (link layer) dropped packets is: ', drop
#print 'Number of Droped packeds because of RET is: ', RET
#print 'Number of Droped packeds because of ARP is: ', ARP
#print 'Number of Droped packeds because of DUP is: ', DUP
#print 'Number of Droped packeds because of NRTE is: ', NRTE
#print 'Number of Droped packeds because of CBK is: ', CBK

pdr = (rec/snd)*100
PDR = (len(receive)/len(sent))*100
tcp_pdr =(len(tcp_rcv)/len(tcp_snt))*100
total_pdr=((len(receive)+len(tcp_rcv))/(len(sent)+len(tcp_snt)))*100

fname = os.path.basename(sys.argv[1])
#fname =  int(re.search("\d+",fname).group())
fname=fname.split(".tr.gz")#remove the extension
#print '\n{0} {1:.3f} '.format(fname[0],pdr)
#print 'CBR PDR'
#print '\n{0} {1:.3f} '.format(fname[0],PDR)
#print 'TCP PDR'
#print '\n{0} {1:.3f} '.format(fname[0],tcp_pdr)
#print 'TOTAL PDR'
print '\n{0} {1:.3f} '.format(fname[0],total_pdr)


