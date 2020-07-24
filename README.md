Previous Hop Routing:
A new routing protocol for VANETs.
A tutorial on how to use the PHR code in NS2 is provided as follows: directory.

Unlike earlier version of Ubuntu, the 18.04 version does not support the direct installation of ns2, because of the latest C/C++ compiler (7.x), but the latest C/C++ compiler supported by ns2 is gcc-4.8.  Here, we’re going to install ns2.35 on Ubuntu version 18.04:

1- Download ns2.35 package.
https://sourceforge.net/projects/nsnam/files/latest/download/


2- tar zxvf ns-allinone-2.35.tar.gz

3- sudo apt install gcc-4.8 g++-4.8

4- sudo apt update

5- sudo apt install build-essential autoconf automake libxmu-dev

6- gedit  ns-allinone-2.35/ns-2.35/linkstate/ls.h
    in line number 137, change erase to this->erase 

7- cd   ns-allinone-2.35/ns-2.35/

8- export CC=gcc-4.8 CXX=g++-4.8 && ./install

9- sudo gedit ~/.bashrc


add the following lines and change "yourname" accordingly. 

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


10- patch the PHR protocol by to place  phr.patch in the same directory of ns-allinone-2.35
then 

type in terminal:

patch -p0 < phr.patch


11- Know you just need to recompile ns2,

cd  ns-allinone-2.35/ns-2.35 

then type in terminal:  export CC=gcc-4.8 CXX=g++-4.8 && make

