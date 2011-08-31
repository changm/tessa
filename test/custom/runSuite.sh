#!/bin/bash

for file in `find sanity -name '*.abc'`
do
    rm $file
done

for file in `find sanity -name '*.as'`
do
    python runTestSuite.py ../..//platform/win32/obj_9/shell/Debug/avmplus_d.exe ../../utils/asc.jar ../../core/builtin.abc $file
#    python runTestSuite.py ../..//platform/win32/obj_9/shell/Release/avmplus.exe ../../utils/asc.jar ../../core/builtin.abc $file

done

for file in `find sanity -name '*.abc'`
do
    rm $file
done

#python runTestSuite.py ../../../../tamarin-redux/platform/win32/obj_9/shell/Debug/avmplus_d.exe ../../lib/asc.jar ../../../../tamarin-redux/core/builtin.abc $1
