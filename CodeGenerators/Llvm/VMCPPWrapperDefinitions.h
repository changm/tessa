/***
 * These are C functions that are wrappers to VM C++ class member methods, and ONLY C++ class member methods.
 * Why do we need these?
 *
 * LLVM JIT compiled code needs to call back into the VM.
 * In order to do this, an LLVM call Instruction needs a llvm function object
 * These function objects are function declarations, which are later linked to an actual address.
 *
 * However, LLVM 2.7 out of the box does not support the MSVC "thiscall" calling convention.
 * where the "this" parameter is passed into the ECX register. We have to modify LLVM to support this later.
 * http://lists.cs.uiuc.edu/pipermail/llvmdev/2009-July/024530.html - As of July 2009
 * This probably doesn't apply to windows/unix where "This" is always the first parameter at which point we can bypass the wrappers.
 * Probably have to do some Macro platform specific magic to choose which address to call
 *
 * So these are C wrappers which call C++ member functions.
 * LLVM then calls these wrappers.
 * These ONLY apply to C++ class member methods. For standard C methods, and static C++ methods, we don't need a wrapper.
 * 
 * Whatever methods you need, please define them only in VMCPPWrapperDefinitoins.cpp 
 */

#include "avmplus.h"	// Only to link against VM runtime functions
using namespace avmplus;

/***
 * Maps function names to their respective addresses.
 * The overall process is:
 * 1) Add a function declaration in VMFunctionDeclarations.cpp.
 * 2) Build the file with llvm.
 * 3) Add the definition in VMCPPWrapperDefintions.cpp
 * 4) Add the call info definition in LlvmJitCalls.h
 * 5) Reference the llvm function object in LlvmJitCompiler.h by the name here
 */

namespace LlvmCodeGenerator {

#ifndef TESSA_ABIKIND
enum AbiKind {
    ABI_FASTCALL,
    ABI_THISCALL,
    ABI_STDCALL,
    ABI_CDECL
};
#endif

	class LlvmCallInfo {
	public:
		std::string methodName;
		std::string mangledMethodName;
		uintptr_t	methodAddress;
		AbiKind		callingConvention;
		bool		isPure;
	};
		
	// Typedefs for templatized methods
	typedef VTable* (*toVTable_Toplevel)(Toplevel*, Atom);
	typedef Atom (*op_construct_Toplevel)(Toplevel*, Atom, int, Atom*);
	typedef Atom (*constructprop_MethodEnv)(MethodEnv*, const Multiname*, int, Atom*);
	typedef Atom (*op_call_Toplevel)(Toplevel*, Atom, int, Atom*);
	typedef Atom (*op_call_instr)(MethodEnv*, Atom, int, Atom*);
	typedef Atom (*op_applytype_MethodEnv)(MethodEnv*, Atom, int, Atom*);
	typedef ArrayObject* (*op_newarray_Toplevel)(Toplevel*, int, Atom*);

	/***
	 * ONE UGLY MASSIVE HACK. This section only DECLARES the call infos as externs
	 */

#define PURE_FUNCTION_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName; 

#define FUNCTION_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName; 

#define PURE_METHOD_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName; 

#define METHOD_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName; 

#define FASTCALL_LLVM_CALLINFO(prettyName, mangledName, address) \
	extern const LlvmCallInfo llvmCallInfo_##prettyName; 

	#include "LlvmJitCalls.h"

	// Keep these as externs
	extern const LlvmCallInfo* llvmCallInfoArray[];
	extern const int SizeOfLlvmCallInfoArray;	// We need this in case anyone wants to get the size of. Because of build issues, we can't do a sizeof(llvmCallInfoArray) anywhere else

#undef FUNCTION_LLVM_CALLINFO
#undef METHOD_LLVM_CALLINFO
#undef FASTCALL_LLVM_CALLINFO
#undef PURE_FUNCTION_LLVM_CALLINFO
#undef PURE_METHOD_LLVM_CALLINFO


#define GET_LLVM_CALLINFO(prettyName) &llvmCallInfo_##prettyName

}	// End namespaec