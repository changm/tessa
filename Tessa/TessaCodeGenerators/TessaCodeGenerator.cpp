
/***
 * We have to include an architecture change to enable building LLVM with Tamarin.
 * Tamarin and LLVM namespaces conflict with a datatype.
 * To resolve the issue, we have to undefine some stuff in tamarin/platform/win32-platform.
 * This also forces us to include LLVM include files BEFORE avmplus files
 *
 * Because of this, we cannot include LLVM stuff WITHIN MethodInfo.cpp
 * So we need a separate project OUTSIDE of avmplus where we include LLVM THEN avmplus.
 * Therefore, MethodInfo creates a code generator, which then instatiates a LIR creator
 * And takes that LIR and sends it off to the real backend
 */

#include "TessaCodeGenerators.h"
#include "llvm/LlvmCodeGenerator.h"
#include "LirGenerators.h"

#include <stdio.h>

namespace TessaCodeGenerators {
	using namespace LlvmCodeGenerator;
	using namespace llvm;
	using namespace LirGenerators;

	TessaCodeGenerator::TessaCodeGenerator() {

	}

	std::string TessaCodeGenerator::createMethodName(MethodInfo* methodInfo) {
		/***
		 * Have to use the iid() instead of the id() because multiple methods such as VM thunkers can have the same method id.
		 * MethodInfos* addresses are the only unique identifiers
		 */
		int methodId = methodInfo->iid();
		char methodName[128];

#if defined(DEBUG) || defined(INTEL_VTUNE)
		avmplus::Stringp methodNameStringP = methodInfo->getMethodName();
		StUTF8String cString(methodNameStringP);
		VMPI_snprintf(methodName, sizeof(methodName), "Method%d::%s", methodId, cString.c_str());
#else 
		VMPI_snprintf(methodName, sizeof(methodName), "Method%d", methodId);
#endif
	
		return methodName;
	}

	void TessaCodeGenerator::compileCode(ASFunction* function, MethodInfo* methodInfo, AvmCore* core) {
		LlvmJitCompiler* llvmJitCompiler = LlvmJitCompiler::getInstance(core, core->gc);
		std::string methodName = createMethodName(methodInfo);
		llvm::Function* llvmFunction = llvmJitCompiler->addLlvmInvokeFunction(methodName);

		LirGenerators::LlvmIRGenerator* llvmIrGenerator = new (core->gc) LirGenerators::LlvmIRGenerator(core, llvmJitCompiler->getModule(), llvmFunction);
		llvmIrGenerator->createLIR(methodInfo, function);

		void* functionPointer = llvmJitCompiler->getFunctionPointer(methodName, methodInfo->iid());
		_nativeCodeLocation = (MethodInvoke)(functionPointer);
	}

	/***
	 * In order to build tamarin with llvm, we have to separate the tessa code generator out from the avmplus namespace.
	 * Since LlvmJitCompiler is a GCRoot, we have to delete the root manually when the VM Dies down
	 * So AvmCore creates an instance of the code generator to delete the llvm jit compiler singleton.
	 * LlvmJitCompiler has to be a GCRoot otherwise the GC will sweep LLVM objects
	 */
	void TessaCodeGenerator::deleteInstance() {
		LlvmJitCompiler::deleteInstance();
	}

	MethodInvoke TessaCodeGenerator::getNativeCodeLocation() {
		TessaAssert(_nativeCodeLocation != NULL);
		return _nativeCodeLocation;
	}
}
