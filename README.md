ReHitman Tools
--------------

This repository contains utilities for work with files of games who was built on Glacier Engine

License: **GNU GPLv2**

Build
-----

```
git clone https://github.com/ReGlacier/ReHitmanTools.git
cd ReHitmanTools
git submodule update --init --recursive
```

For build all projects: 
```
mkdir build && cd build
cmake -A Win32 ..
cmake --build . --config Release
```