README
======

Sayuri is ...
-------------

* A chess engine for UCI.
* MIT License software.
* Written in C++11 and only use the standard libraries.
* Easy to build with CMake.
* Equipped Lisp interpreter that is named *Sayulisp*.
    + Sayulisp can generate and operate Chess Engine.
    + Sayulisp can customize algorithm of Search Function.
    + Sayulisp can customize values of Evaluation Function.



Files and Directories
---------------------

### Directories ###

* src : Source files.
* SayuriCompiled : Binary files.
    + Linux : Binary files for Linux.
        - For64Bit : For 64 bit machines.
    + Windows : Binary files for Windows. (Created by Linux's MinGW.)
        - For64Bit : For 64 bit machines.
    + Android : Binary file for Android.
* SayuriLogo : Logo image files.
* SampleGames : Sample game files of Sayuri vs other chess engines.
* DocumentsHTML : Documents.
* Tools : Convenient tools.

### Files ###

* README.md : README file.
* CMakeLists.txt : A configuration file for CMake.
* LICENSE : The software license.



How To Build with CMake
-----------------------

Using CMake, you can build Sayuri easily.

It's default compiler is *"clang++"*.  

### Build for Release ###

1. Change directory to where "CMakeLists.txt" is placed in.
2. Run the following commands.
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake ..`
        - If you want to change a compiler into `<cc>`, you should call...  
          `$ cmake -DCMAKE_CXX_COMPILER=<cc> ..`
    4. `$ make`
3. *"sayuri"* is built.

### Build for Debug ###

"for Debug" means *"No Optimize"* and *"Embedded Debug Information"*.

1. Change directory to where "CMakeLists.txt" is placed in.
2. Run the following commands.
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake -DCMAKE_BUILD_TYPE=Debug ..`
        - If you want to change a compiler into `<cc>`, you should call...  
          `$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=<cc> ..`
    4. `$ make`
3. *"sayuri"* is built.



How To Build without CMake
--------------------------

If you don't have CMake, you can build "sayuri" by the following commands
in "src" directory.

If you have "clang" :

    clang++ -std=c++11 -fno-rtti -Ofast -march=native -pthread -o sayuri *.cpp

If you have "gcc" :

    g++ -std=c++11 -fno-rtti -Ofast -march=native -pthread -o sayuri *.cpp



How To Make Distributable Packages
----------------------------------

After building Sayuri with CMake, run "`$ make dist`" command.

The following packages will be created.

* sayuri-xxxx.xx.xx.tar.Z
* sayuri-xxxx.xx.xx.tar.bz2
* sayuri-xxxx.xx.xx.tar.gz

(Note: "xxxx.xx.xx" is the version number.)



Use as Library
--------------

Sayuri's source code can be used as library.

The Library has only one function.

    extern "C"
    const char* ExecuteSayulisp(const char* code)

<h6> Usage </h6>

1. Delete `main.cpp`, because it has `main()` function.

2. Include `sayuri.h` with `#include "sayuri.h"`.

3. Call `const char* ExecuteSayulisp(const char* code)`.
    + `code` is Sayulisp.
    + Returns S-Expression.

<h6> Example </h6>

If you want to call Sayuri from Python on Linux, then...

1. Delete `main.cpp`, because it has `main()` function.

2. Build "`libsayuri.so`" by...

~~~~
$ <g++ | clang++> -std=c++11 -Ofast -pthread -march=native -fno-rtti -shared -fPIC -o libsayuri.so *.cpp
~~~~

3. Use from Python.

~~~~python
from ctypes import*

# Load.
lib = cdll.LoadLibrary("/path/to/libsayuri.so")

# Get ready.
sayulisp = lib.ExecuteSayulisp
sayulisp.restype = c_char_p
sayulisp.argtype = (c_cahr_p)

# Go.
print(sayulisp(b"(+ 1 2 3)"))
~~~~


UCI Options
-----------

* To change size of the hash table. (Default: 32 MB, Max: 8192 MB, Min: 8 MB)
    + `setoption name Hash value <Size(MB)>`  
      (Note!!) `<Size(MB)>` must be 2^n. (e.g. 128, 256, 512, 1024,...)

* To initialize the hash table.
    + `setoption name Clear Hash`

* To enable Ponder. (Default: true)
    + `setoption name Ponder value <true or false>`

* To change the number of threads. (Default: 1, Max: 64, Min: 1)
    + `setoption name Threads value <Number of threads>`

* To enable analyse mode. (Default: false)
    + `setoption name UCI_AnalyseMode value <true or false>`
