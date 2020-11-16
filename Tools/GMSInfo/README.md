GMSInfo
-------

This tool was created to parse GMS file format

Required
--------
 * Visual Studio 2019 (or other compiled with C++20)
 * CMake 3.16
 * Python 3.9.0

Build
-----
```
mkdir build && cd build
cmake -A Win32 ..
cmake --build . --config Release --target HBM_GMSTool
```

Usage
-----
`./HBM_GMSTool.exe [path to GMS file] [path to PRM file] >> report.txt`

all information about the GMS file will be written into `report.txt` file

Supported games
---------------

 * Hitman Blood Money (primary support)
 * Hitman Contracts (in theory with possible bugs)
 * Hitman 2 Silent Assassin (not tested)
 * Hitman Agent 47 (not tested)

FAQ
----

 * **Q:** My report contains `NOT FOUND` strings, what's wrong?
 * **A:** Your type not declared inside `typeids.json` file or you forgot to copy this file to the folder with HBM_GMSTool.exe