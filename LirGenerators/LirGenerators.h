
#ifndef __CODEGENERATORS__CODEGENERATORS__
#define __CODEGENERATORS__CODEGENERATORS__

#pragma warning(disable:4291) // Disable matching delete operator. OCcurs due to enabling C++ exceptions

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"

#define __USING_LLVM__

#include "TessaVM.h"

namespace LirGenerators {
	class LlvmIrGenerator;
}

#ifndef TESSA_ABIKIND
#define TESSA_ABIKIND
enum AbiKind {
    ABI_FASTCALL,
    ABI_THISCALL,
    ABI_STDCALL,
    ABI_CDECL
};
#endif

#include "LlvmIRGenerator.h"

#endif