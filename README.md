Metrics-Calculator


Requirements
------------

On Debian/Ubuntu, install the following packages:
- clang 3.6
- libclang-3.6-dev

It normally also installs the LLVM dependencies.


Compilation
-----------

mkdir build
cd build
cmake ../cmake


Running the tool
----------------

./metrics-calculator yourcppfile.cpp -- yourcompiloptions

Examples:
./metrics-calculator main.cpp -- -isystem /usr/include/clang/3.6/include

If you need C++11 support just add -std=c++11 after --.

Note that the -isystem option is here to avoid conflicts with gcc installations.


Issues
------

On Debian/Ubuntu systems it seems that some files are not placed in the correct directory, see:
https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=785931
https://llvm.org/bugs/show_bug.cgi?id=23352

Debian simple bug workaround :
ln -s /usr/share/llvm-3.6 /usr/lib/llvm-3.6/share/llvm
