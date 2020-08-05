Previous Hop Routing:
A new routing protocol for VANETs.

A tutorial on how to install ns-2.35 on Ubuntu 18.04 then add the PHR code to it is provided as follows:

The new version of Ubuntu 18.04 does not support the direct installation of ns-2.35, due to the latest C/C++ compiler (7.x), however, the C/C++ compiler version 4.8 is supported by ns2.  Here, we’re going to install ns2.35 on Ubuntu version 18.04:

1. Download ns2.35 package.
https://sourceforge.net/projects/nsnam/files/latest/download/


2. tar zxvf ns-allinone-2.35.tar.gz

3. sudo apt install gcc-4.8 g++-4.8

4. sudo apt update

5. sudo apt install build-essential autoconf automake libxmu-dev

6. gedit  ns-allinone-2.35/ns-2.35/linkstate/ls.h
    in line number 137, change erase to this->erase 

7. cd   ns-allinone-2.35/

8. export CC=gcc-4.8 CXX=g++-4.8 && ./install

9. sudo gedit ~/.bashrc


add the following lines and change "yourname" accordingly. 

~~~
#LD_LIBRARY_PATH

OTCL_LIB=/home/yourname/ns-allinone-2.35/otcl-1.14
NS2_LIB=/home/yourname/ns-allinone-2.35/lib
X11_LIB=/usr/X11R6/lib
USR_LOCAL_LIB=/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OTCL_LIB:$NS2_LIB:$X11_LIB:$USR_LOCAL_LIB

#TCL_LIBRARY

TCL_LIB=/home/yourname/ns-allinone-2.35/tcl8.5.10/library
USR_LIB=/usr/lib
export TCL_LIBRARY=$TCL_LIB:$USR_LIB

#PATH

XGRAPH=/home/yourname/ns-allinone-2.35/bin:/home/yourname/ns-allinone-2.35/tcl8.5.10/unix:/home/yourname/ns-allinone-2.35/tk8.5.10/unix
#the above two lines beginning from xgraph and ending with unix should come on the same line
NS=/home/yourname/ns-allinone-2.35/ns-2.35/ 
NAM=/home/yourname/ns-allinone-2.35/nam-1.15/ 
PATH=$PATH:$XGRAPH:$NS:$NAM
~~~
Then
~~~
source ~/.bashrc
~~~
10. patch the PHR protocol by to place  phr.patch in the same directory of ns-allinone-2.35
then 

type in terminal:

~~~
cd ..

patch -p0 < phr.patch
~~~

11. Now you just need to recompile ns2,
type in terminal the following commands:

~~~
cd  ns-allinone-2.35/ns-2.35 

./configure --with-tcl-ver=8.5

export CC=gcc-4.8 CXX=g++-4.8 && make
~~~

NOW ns2 with PHR is ready to run .tcl files.

You can find a simple scienaro with PHR in the git, to run it, type in terminal:

~~~
ns phr_scenario.tcl 
~~~
*In order to get results out from .tr simulation file, python scripts are provided.*

+ To get Packet Delivery Ratio (PDR), all you need is to run pdr.py as follows:
~~~
python pdr.py ns2_tracefilename.tr
~~~
ns2_tracefilename.tr is trace filename that you've got after run simulation
+ To calculate CDF delay, all you need is to pass four different .tr files as parameters to delaypdf.py script to be ploated.
~~~
python delaypdf.py aodv.tr olsr.tr gpsr.tr phr.tr  
~~~
Note: do not change the .tr files order. if you need to plot two or more CDF delay protocols, please, change the script accordingly. 

+ To calculate Probability of Drop Burst Length(DBL), (A new metric proposed by Awos Kh. Ali & Iain Phillips), you need to run the following two python scripts as follows:
~~~
python pre_dbl.py ns2_tracefilename.tr > new_file.txt
~~~
Then the pass the new_file.txt to the second script as follows:

~~~
python calc_dbl new_file.txt > ready_to_plot.txt
~~~
Now ready_to_plot.txt contains probability of DBL for a certain simulation, all you need now is to plot it.
