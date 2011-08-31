#!/bin/bash

rm benchmarkResults.csv

for file in `find sanity -name '*.abc'`
do
    rm $file
done

python timeTestSuite.py ../..//platform/win32/obj_9/shell/Release/avmplus.exe ../../utils/asc.jar ../../core/builtin.abc sanity/

for file in `find sanity -name '*.abc'`
do
    rm $file
done

