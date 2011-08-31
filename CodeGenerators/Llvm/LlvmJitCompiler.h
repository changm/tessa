
namespace avmplus {
	class AvmCore;
}

namespace MMgc {
	class GC;
	class GCObject;
}
#include "MMgc.h"

/***
 * Represents the LLVM jit code generator. 
 * This is a singleton because initializing the LLVM code is expensive and we only want to do it once.
 * This also maps llvm functions to their actual VM Address at runtime. This is a one time cost.
 *
 */
namespace LlvmCodeGenerator {
	class LlvmJitCompiler : public MMgc::GCRoot {
	private:
		LlvmJitEventListener jitEventListener;
		bool addedMappings;

		Module* module;
		Module* vmMethodModule;
		avmplus::AvmCore* core;
		MMgc::GC* gc;
		llvm::LLVMContext context;
		ExecutionEngine*	executionEngine;
		static LlvmJitCompiler*	singleton;

		// Try to mimic those in core/jit-calls.h
		const llvm::IntegerType* atomType;
		const llvm::PointerType* pointerType; 
		const llvm::IntegerType* intType;

		// Setup known global mappings
		void addMethodMapping(std::string mangledName, std::string prettyName, unsigned int functionAddress, bool isPureMethod);
		void readDeclarationsModule();
		void addMappings();

		llvm::ConstantInt*	createConstantInt(int integerValue); 
		llvm::IntToPtrInst* createConstantPtr(intptr_t pointer); 
		llvm::IntToPtrInst* castIntToPtr(llvm::Value* integer, std::string castName = ""); 

		
	public:
		llvm::Function* addLlvmInvokeFunction(std::string methodName);
		Module* getModule();
		void*	getFunctionPointer(std::string methodName, int methodId);
		static void deleteInstance();
		static LlvmJitCompiler* getInstance(avmplus::AvmCore* core, MMgc::GC* gc); 

		LlvmJitCompiler(avmplus::AvmCore* core, MMgc::GC* gc);
		~LlvmJitCompiler();
		void	addGlobalMappings();
		void	initializeModule();
		
	};
}