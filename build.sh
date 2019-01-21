#!/bin/bash
mkdir Release
cd Release
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLIBTYPE=STATIC
cmake --build . # make
