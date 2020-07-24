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


9- patch the PHR protocol by to place  phr.patch in the same directory of ns-allinone-2.35
then 

type in terminal:  patch -p0 < phr.patch


Know you just need to recompile ns2,

cd  ns-allinone-2.35/ns-2.35 

then type in terminal:  export CC=gcc-4.8 CXX=g++-4.8 && make

