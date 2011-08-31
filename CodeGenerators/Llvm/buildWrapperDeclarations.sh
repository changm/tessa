../../../../llvm-gcc-2.8/bin/llvm-g++.exe -emit-llvm -O3 VMFunctionDeclarations.cpp -c -o VMFunctionDeclarations.o
rm VMFunctionDeclarations.o.ll
../../../../llvm-2.8/bin/Release/llvm-dis.exe VMFunctionDeclarations.o
