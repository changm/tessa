import sys
import os
import string
import subprocess
import time
import glob
import re

iterations = 1
resultsFile = "benchmarkResults.csv"

nanojitTypedResults = {}
nanojitResults = {}
tessaResults= {}
tessaTPResults = {}
tessaInlineResults = {}
tessaTPInlineResults = {}

def usage():
    print("Usage")
    print("runtests.py avmshell asc.jar builtinAbc testFile")
    print

def runShell(shellExec, testFile, vmArgs):
	total = 0
	for i in range(0, iterations):
    		avmshellCommand = os.path.abspath(shellExec) + " " + vmArgs + " " + testFile
		start = time.clock() 
		avmshellProcess = subprocess.call(avmshellCommand)
        	end = time.clock()
		total += end - start
		
	average = total / iterations
	return average

def compileAbc(ascJarFile, builtinAbc, testFile):
    ascJarFile = os.path.abspath(ascJarFile)
    builtinAbc = os.path.abspath(builtinAbc)
    ascCommand = "java -jar " + ascJarFile + " -import " + builtinAbc + " " + testFile
    compileToAbcProcess = os.popen(ascCommand)
    compiledFileName = testFile.replace(".as", ".abc")
    return compiledFileName

def writeResultsToFile(testFile):
    print("looking at file: " + testFile)
    avmshellResult = nanojitResults[testFile]
    tessaResult = tessaResults[testFile]
    tessaInlineResult = tessaInlineResults[testFile]
    tessaType = tessaTPResults[testFile]
    tessaTypeInline = tessaTPInlineResults[testFile]
    
    tessaComparison = str(avmshellResult / tessaResult) 
    tessaInlineComparison = str(avmshellResult / tessaInlineResult) 
    tessaTypeComparison = str(avmshellResult / tessaType)
    tessaTypeInlineComparison = str(avmshellResult / tessaTypeInline) 


    typedFileName = testFile
    if (re.search('\\\\untyped\\\\', testFile)):
        typedFileName = re.sub('\\\\untyped\\\\', '\\\\typed\\\\', testFile)
    elif (re.search('\\\\partialTypedFieldsAndParam\\\\', testFile)):
        typedFileName = re.sub('\\\\partialTypedFieldsAndParam\\\\', '\\\\typed\\\\', testFile)

    nanojitTyped = nanojitTypedResults[typedFileName]
    nanojitComparedToNanojitTyped = str (nanojitTyped / avmshellResult)
    tessaComparedToNanojitTyped = str(nanojitTyped / tessaResult)
    tessaTypedComparedToNanojitTyped = str(nanojitTyped / tessaType)
    tessaInlineComparedToNanojitTyped = str(nanojitTyped / tessaInlineResult)
    tessaInlineAndTypeComparedToNanojitTyped = str(nanojitTyped / tessaTypeInline)

    csvFile.write(testFile + "," + str(avmshellResult) + "," + str(tessaResult) + "," + str(tessaType) + "," + str(tessaInlineResult) + "," + str(tessaTypeInline) + "," + tessaComparison + "," + tessaTypeComparison + ","  + tessaInlineComparison + "," + tessaTypeInlineComparison + "," + nanojitComparedToNanojitTyped + "," + tessaComparedToNanojitTyped + "," + tessaTypedComparedToNanojitTyped + " ," + tessaInlineComparedToNanojitTyped + "," + tessaInlineAndTypeComparedToNanojitTyped + "\n")


def runTest(shellExec, ascJarFile, builtinAbc, testFile):
    print("Running test: " + testFile);
    compiledFileName = compileAbc(ascJarFile, builtinAbc, testFile)
    shellResult = runShell(shellExec, compiledFileName, "")
    tessaResult = runShell(shellExec, compiledFileName, "-Dtessa")
    tessaType = runShell(shellExec, compiledFileName, "-Dtessa -Dtessa_tp")
    tessaInlineResult = runShell(shellExec, compiledFileName, "-Dtessa -Dtessa_inline")
    tessaTypeInline = runShell(shellExec, compiledFileName, "-Dtessa -Dtessa_inline -Dtessa_tp")

    nanojitResults[testFile] = shellResult
    tessaResults[testFile] = tessaResult
    tessaTPResults[testFile] = tessaType
    tessaInlineResults[testFile] = tessaInlineResult
    tessaTPInlineResults[testFile] = tessaTypeInline

    if (re.search('\\\\typed\\\\', testFile)):
        nanojitTypedResults[testFile] = shellResult


def writeFileHeader(openedFile):
	openedFile.write("Benchmark, NanoJIT, TESSA, TESSA TYPE, TESSA INLINE, TESSA TYPE INLINE, TESSA DIFF, TESSA TYPE DIFF, TESSA INLINE DIFF, TESSA TYPE INLINE DIFF, NanoJIT V NJ TYPED, TESSA V NJ TYPED, TESSA TP V NJ TYPED, TESSA INLINED V NJ TYPED, TESSA INLINE + TP V NJ TYPED \n")


numberOfArgs = len(sys.argv)
if (numberOfArgs != 5):
    usage()
    exit(0)

csvFile = None 
if (os.path.exists(resultsFile)):
	csvFile = open(resultsFile, "a")
else:
	csvFile = open(resultsFile, "w")
	writeFileHeader(csvFile)

def runSuite(shellExec, ascJarFile, builtinAbc, testDirectory):
    for file in os.listdir(testDirectory):
        fullPath = os.path.normpath(os.path.join(testDirectory, file))
        if (os.path.isdir(fullPath)):
            runSuite(shellExec, ascJarFile, builtinAbc, fullPath)
        elif (re.search('\.as', fullPath)):
            runTest(shellExec, ascJarFile, builtinAbc, fullPath)


def writeResults(testDirectory):
    for file in os.listdir(testDirectory):
        fullPath = os.path.normpath(os.path.join(testDirectory, file))
        if (os.path.isdir(fullPath)):
            writeResults(fullPath)
        # don't want .abc files
        elif (re.search('\.as', fullPath)):
            writeResultsToFile(fullPath)

runSuite(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
writeResults(sys.argv[4])
csvFile.close()
