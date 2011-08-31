import sys
import os
import string
import subprocess

def usage():
    print("Usage")
    print("runtests.py avmshell asc.jar builtinAbc testFile")
    print

def runShell(shellExec, testFile):
    avmshellCommand = os.path.abspath(shellExec) + " " + testFile
    avmshellProcess = os.popen(avmshellCommand)
    return avmshellProcess.read()

def compileAbc(ascJarFile, builtinAbc, testFile):
    ascJarFile = os.path.abspath(ascJarFile)
    builtinAbc = os.path.abspath(builtinAbc)
    ascCommand = "java -jar " + ascJarFile + " -import " + builtinAbc + " " + testFile
    compileToAbcProcess = os.popen(ascCommand)
    compiledFileName = testFile.replace(".as", ".abc")
    return compiledFileName

def runShellTessa(shellExec, testFile):
    avmshellCommand = os.path.abspath(shellExec) + " -Dtessa " + testFile
    avmshellProcess = os.popen(avmshellCommand)
    return avmshellProcess.read()

def runShellTessaInline(shellExec, testFile):
    avmshellCommand = os.path.abspath(shellExec) + " -Dtessa -Dtessa_inline " + testFile
    avmshellProcess = os.popen(avmshellCommand)
    return avmshellProcess.read()

def runShellTessaTP(shellExec, testFile):
    avmshellCommand = os.path.abspath(shellExec) + " -Dtessa -Dtessa_tp " + testFile
    avmshellProcess = os.popen(avmshellCommand)
    return avmshellProcess.read()


def checkResults(avmshellResult, tessaResult, testFile):
    if (avmshellResult != tessaResult):
        print("Tessa does not equal Shell for " + testFile + ".")
        print("Shell: " + avmshellResult)
        print("Tessa: " + tessaResult)
    else:
        print(testFile + " Tessa passed")
    
def checkResultsWithInline(avmshellResult, tessaResult, tessaInlineResult, testFile):
    if (avmshellResult != tessaResult):
        print("Tessa does not equal Shell for " + testFile + ".")
        print("Shell: " + avmshellResult)
        print("Tessa: " + tessaResult)
    else:
        print(testFile + " Tessa passed")

	if (avmshellResult != tessaInlineResult):
		print("Tessa Inline does not equal Shell for " + testFile + ".")
		print("Shell: " + avmshellResult)
		print("Tessa Inline: " + tessaInlineResult)
	else:
		print(testFile + " inline passed")


def runTest(shellExec, ascJarFile, builtinAbc, testFile):
    compiledFileName = compileAbc(ascJarFile, builtinAbc, testFile)
    shellResult = runShell(shellExec, compiledFileName).strip(string.whitespace)
    tessaResult = runShellTessa(shellExec, compiledFileName).strip(string.whitespace)
    tessaTypeResult = runShellTessaTP(shellExec, compiledFileName).strip(string.whitespace)
    tessaInlineResult = runShellTessaInline(shellExec, compiledFileName).strip(string.whitespace)
    checkResultsWithInline(shellResult, tessaResult, tessaInlineResult, testFile)
    checkResults(shellResult, tessaTypeResult, testFile)

numberOfArgs = len(sys.argv)
if (numberOfArgs != 5):
    usage()
    exit(0)

runTest(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
