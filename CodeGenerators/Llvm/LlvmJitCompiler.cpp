
#include "LlvmCodeGenerator.h"
#include "avmplus.h"	// Only to link against VM runtime functions
#include "VMCPPWrapperDefinitions.h"

using namespace avmplus;

/***
 * Assertions in this namespace are defined by LLVM
 */

namespace LlvmCodeGenerator {
	LlvmJitCompiler::LlvmJitCompiler(AvmCore* core, MMgc::GC* gc) :
		MMgc::GCRoot(gc)
	{
		this->core = core;
		this->gc = gc;
		this->addedMappings = false;
	}

	LlvmJitCompiler::~LlvmJitCompiler() {
		delete vmMethodModule;
		delete executionEngine;
	}

	void LlvmJitCompiler::initializeModule() {
		InitializeNativeTarget();
		module = new Module("test", context);

		// Have to do this before execution engine is created
		/*
		if (core->config.tessaVerbose) {
			llvm::PrintMachineCode = true;
		}
		*/

		EngineBuilder engineBuilder(module);
		/*** 
		 * This is really weird. Preliminary results show that having Codegen opt less is actually the fastest.
		 */
		engineBuilder.setOptLevel(CodeGenOpt::Less);
		executionEngine = engineBuilder.create();
		executionEngine->RegisterJITEventListener(&jitEventListener);

		atomType = Type::getInt32Ty(context);
		intType = Type::getInt32Ty(context);
		pointerType = Type::getInt32PtrTy(context);
	}

	/***
	 * Adds a method to the llvm module with the Method Signature Atom MethodInfo::Invoke(MethodEnv* env, int argCount, Atom* arguments);
	 */
	llvm::Function* LlvmJitCompiler::addLlvmInvokeFunction(std::string methodName) {
		Function* function = module->getFunction(methodName);
		assert(function == NULL);

		std::vector<const llvm::Type*> paramTypes;
		paramTypes.push_back(Type::getInt32PtrTy(context));	// MethodEnv*
		paramTypes.push_back(Type::getInt32Ty(context));
		paramTypes.push_back(Type::getInt32PtrTy(context));	// Atom*

		// Create the generic function entry and insert this entry into module.  The
		// function will have a return type of void and take no arguments
		// The '0' terminates the list of argument types.
		bool isVarArg = false;
		FunctionType* functionSignature = FunctionType::get(Type::getInt32Ty(context), paramTypes, isVarArg);
		return Function::Create(functionSignature, Function::InternalLinkage, methodName, module);
	}

	void LlvmJitCompiler::readDeclarationsModule() {
		std::string fileName = "../../CodeGenerators/Llvm/VMFunctionDeclarations.o";
		MemoryBuffer *bitcode = MemoryBuffer::getFileOrSTDIN(fileName);
		AvmAssert(bitcode != NULL);
		std::string error;
		vmMethodModule = llvm::ParseBitcodeFile(bitcode, context, &error);
		AvmAssert(vmMethodModule != NULL);
	}

	void LlvmJitCompiler::addMethodMapping(std::string mangledName, std::string prettyName, uintptr functionAddress, bool isPureMethod) {
		AvmAssert(vmMethodModule != NULL);
		AvmAssert(module != NULL);

		char errorMessage[512];
		VMPI_snprintf(errorMessage, sizeof(errorMessage), "No llvm function declaration exists for Method %s with mangled name %s\n", prettyName.data(), mangledName.data());

		llvm::Function* declaredMethod = vmMethodModule->getFunction(mangledName);
		AvmAssertMsg(declaredMethod != NULL, errorMessage);

		llvm::Function* realMethod = llvm::Function::Create(declaredMethod->getFunctionType(), Function::ExternalLinkage, prettyName, module);
		if (isPureMethod) {
			realMethod->addAttribute(0, Attribute::ReadOnly);
		} 

		executionEngine->addGlobalMapping(realMethod, (void*) functionAddress);
	}

	void LlvmJitCompiler::addMappings() {
		int numberOfMappings = SizeOfLlvmCallInfoArray / sizeof(LlvmCallInfo*);
		for (int i = 0; i < numberOfMappings; i++) {
			const LlvmCallInfo* callInfo = llvmCallInfoArray[i];
			addMethodMapping(callInfo->mangledMethodName, callInfo->methodName, callInfo->methodAddress, callInfo->isPure);
		}
	}

	void LlvmJitCompiler::addGlobalMappings() {
		AvmAssert(addedMappings == false);
		addedMappings = true;
		readDeclarationsModule();
		addMappings();
	}

	void* LlvmJitCompiler::getFunctionPointer(std::string methodName, int methodId) {
		Function* function = module->getFunction(methodName);
		assert(function != NULL);

#ifdef DEBUG
		if (core->config.tessaVerbose) {
			function->dump();
		}
#endif

		void* address = executionEngine->getPointerToFunction(function);

#ifdef INTEL_VTUNE
		VTune_NotifyLoad((char*)methodName.data(), methodId, address, jitEventListener.sizeOfCompiledCode);
#endif

		return address;
	}

	Module* LlvmJitCompiler::getModule() {
		return module;
	}

	LlvmJitCompiler* LlvmJitCompiler::singleton;	// static initializer

	LlvmJitCompiler* LlvmJitCompiler::getInstance(AvmCore* core, MMgc::GC* gc) {
		if (singleton == NULL) {
			assert(core != NULL);
			assert(gc != NULL);
			singleton = new LlvmJitCompiler(core, gc);
			singleton->initializeModule();
			singleton->addGlobalMappings();


#ifdef DEBUG
			gc->incrementalValidation = true;
			gc->incrementalValidationPedantic = true;
#endif
		}

		return singleton;
	}

	void LlvmJitCompiler::deleteInstance() {
		if (singleton != NULL) {
			delete singleton;
		}
	}
}