
/***
 * This namespaec takes in LLVM IR and invokes LLVM's JIT compiler.
 * This namespace should not rely on ANYTHING other than LLVM.
 * To do this, Code Generators must therefore be allocated on the stack 
 */

#ifndef __LLVMCODEGENERATOR__LLVMCODEGENERATOR__
#define __LLVMCODEGENERATOR__LLVMCODEGENERATOR__

#define __USING_LLVM__

#include "llvm/LLVMContext.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Module.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace LlvmCodeGenerator {
	class LlvmJitCompiler;
	class LlvmJitEventListener;
}

#include "LlvmJitEventListener.h"
#include "LlvmJitCompiler.h"	// Relies on LlvmJitEventListener


#endif