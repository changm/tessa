#!/bin/bash
AVM=../../platform/win32/obj_9/shell/Debug/avmplus_d.exe
#AVM=../../platform/win32/obj_9/shell/Release/avmplus.exe
ASC=../../utils/asc.jar
BUILTINABC=../../core/builtin.abc
SHELLABC=../../shell/shell_toplevel.abc

export AVM
export ASC
export BUILTINABC
export SHELLABC

rm *.html


function runSunspider()
{
	echo "Running Sunspider no inline"
	python runtests.py -i 1 sunspider
}

function runJsBench()
{
	echo "Running jsbench no inline"
	python runtests.py -i 1 jsbench

}

function runScimark()
{
	echo "Running scimark no inline"
	python runtests.py -i 1 scimark
}

function runV8() {
    echo "Running v8"
    python runtests.py -i 1 v8.5
}

runSunspider
#runScimark
#runJsBench
#runV8
