#This script is ploting DBL for 3 .tr files. 
import matplotlib.pyplot as plt
import numpy as np
import sys

x=[]   # Usage: AODV, OLSR and then gpsr DBL files (or other DBL files, change figure titles accordingly)
y=[]
z=[]
A=[]
B=[]
C=[]
D=[]
E=[]
if len(sys.argv)<3:
    print "Usage: pass 3 probability of DBL files (output of calc_dbl.py) as parameters. to plot DBL."
    print "This script plot probability of DBL"
    sys.exit(0)
f = open(sys.argv[1], 'r')
f1 = open(sys.argv[2], 'r')
f2 = open(sys.argv[3], 'r')
splittt = f.read().split('\n')
split2 = f1.read().split('\n')
split3 = f2.read().split('\n')
f.close()
f1.close()
f2.close()
for item12 in splittt:
    try:
        xyz = item12.split()
        x.append(int(xyz[0]))
        y.append(float(xyz[1]))

        print xyz[0] +" "+ xyz[1]
    except: IndexError,  
for item13 in split2:
    try:
        ABC = item13.split()
        A.append(int(ABC[0]))
        B.append(float(ABC[1]))

        print ABC[0] +" "+ ABC[1]
    except: IndexError,
for item14 in split3:
    try:
        DEF = item14.split()
        D.append(int(DEF[0]))
        E.append(float(DEF[1]))
#	C.append(float(ABC[2]))
        print DEF[0] +" "+ DEF[1]
    except: IndexError, 

plt.plot (x, y, linewidth=2, marker ='o', color='red',  label = 'aodv')
plt.plot (A, B, linewidth=2, marker ='x', color='blue',  label = 'olsr')
plt.plot (D, E, linewidth=2, marker ='*', color='green',  label = 'gpsr')
#plt.axis([94.0, 100.0, 0.0, 0.10])   #to zoomin 
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)

plt.legend(loc='upper right')
plt.title('A zoomed portion of  DBL with 1000 connections in MAN map')
plt.xlabel('DBL', fontsize=14)
plt.ylabel('Probability of DBL',fontsize=14)
plt.grid()
plt.tight_layout()
plt.savefig('longdbl1000.pdf')
plt.show()
