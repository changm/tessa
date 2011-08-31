
/***
 * Go through each TESSA instruction and lazily create LLVM IR Instructions
 * We also lazily create LLVM basic Blocks, instructions, etc
 * This is designed to compile only one method at a time
 */

#define __USING_LLVM__	// Need this since LLVM redefines int8_t

#include "LirGenerators.h"	// Include LLVM stuff BEFORE defining Tamarin stuff because of datatype conflicts
#include "VMCPPWrapperDefinitions.h"

/***
 * NOTE NOTE NOTE:
 * Some of the object names in Tessa collide with LLVM (e.g. BasicBlock).
 * In these cases, we ALWAYS ALWAYS ALWAYS with using namespace llvm
 * and fully qualify TessaVM objects
 */

/***
 * Rules of the game:
 * We assume that the following rules apply between EACH visit function:
 * All types are coerced back to the native LLVM types at each visit() method. It is the responsibility of the
 * visit() to coerce the types back to their native types.
 * ints -> intType
 * uint -> uintType
 * double -> doubleType
 * Any pointer -> pointerType
 * eg if the visit() calls out to the VM and returns a ScriptObject*, the visit() should recast
 * the ScriptObject* -> pointerType.
 *
 * Biggest trick is that Tamarin assumes booleans are 32 bit integers. Llvm wants booleans to be 1 bit integers.
 * So we have to cast between 32 bit and 1 bit integers for boolean values when loading/storing to/from arguments arrays
 * and return values
 */

/***
 * Performance penalty notes:
 * Be wary of allocating anything on the stack. If you over allocate, the GC thinks some value on the stack may be alive,
 * and you get massive performance penalties during the GC phases.
 */
namespace LirGenerators {
	using namespace llvm;
	using namespace TessaVM;
	using namespace TessaInstructions;
	using namespace avmplus;

	LlvmIRGenerator::LlvmIRGenerator(avmplus::AvmCore* core, Module* module, llvm::Function* llvmFunction) {
		this->core = core;
		this->gc = core->gc;
		_tessaValueToLlvmValueMap = new (gc) avmplus::GCSortedMap<TessaValue*, llvm::Value*, avmplus::LIST_NonGCObjects> (gc);
		_tessaToLlvmBasicBlockMap = new (gc) GCSortedMap<TessaVM::BasicBlock*, llvm::BasicBlock*, avmplus::LIST_NonGCObjects>(gc);
		_tessaToEndLlvmBasicBlockMap = new (gc) GCSortedMap<TessaVM::BasicBlock*, llvm::BasicBlock*, avmplus::LIST_NonGCObjects>(gc);

		_methodEnvCallStack = new (gc) List<llvm::Value*, avmplus::LIST_NonGCObjects>(gc);
		_toplevelPointerStack = new (gc) avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>(gc);
		_methodEnvScopeChainStack = new (gc) avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>(gc);
		_methodEnvVTableStack = new (gc) avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>(gc);
		_scopeStackCallStack = new (gc) avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>(gc);
		_scopeStackCountStack = new (gc) avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>(gc);
		_methodInfoStack = new (gc) avmplus::List<MethodInfo*, avmplus::LIST_GCObjects>(gc);

		_tessaTypeFactory = TypeFactory::getInstance();

		TessaAssert(llvmFunction != NULL);
		TessaAssert(module != NULL);

		this->_llvmFunction = llvmFunction;
		this->_module = module;
		context = &module->getContext();

		// Use known types
		_atomType = llvm::Type::getInt32Ty(*context);
		_intType = llvm::Type::getInt32Ty(*context);
		_pointerType = llvm::Type::getInt32PtrTy(*context);
		_doublePointerType = llvm::Type::getDoublePtrTy(*context);
		_doubleType = llvm::Type::getDoubleTy(*context);
		_uintType = llvm::Type::getInt32Ty(*context);
		_booleanType = llvm::Type::getInt1Ty(*context);
		_booleanPointerType = llvm::Type::getInt1PtrTy(*context);
		_voidType = llvm::Type::getVoidTy(*context);
		_returnCount = 0;
	}

	LlvmIRGenerator::~LlvmIRGenerator() {

	}

	void LlvmIRGenerator::putValue(TessaValue* tessaValue, llvm::Value* llvmValue) {
		TessaAssert(tessaValue != NULL);
		TessaAssert(llvmValue != NULL);

		// We can overwrite the beginning parameter instructions multiple times due to the way the arguments work. Other than that, no
		if (tessaValue->isInstruction()) {
			TessaInstruction* instruction = static_cast<TessaInstruction*>(tessaValue);
			if (!(((currentBasicBlock->getBasicBlockId() == 0) && instruction->isParameter()))) {
				TessaAssert(!_tessaValueToLlvmValueMap->containsKey(instruction));
			}
		}

		_tessaValueToLlvmValueMap->put(tessaValue, llvmValue);
	}

	void LlvmIRGenerator::debugMessage(std::string message) {
#ifdef DEBUG
		if (core->config.tessaVerbose) {
			printf("%s\n", message.data());
		}
#endif
	}

	llvm::Value* LlvmIRGenerator::getValue(TessaValue* value) {
		if (value->isConstantValue()) {
			llvm::Value* llvmConstantValue;
			ConstantValue* constantValue = static_cast<ConstantValue*>(value);
			if (constantValue->isPointer()) {
				TessaTypes::ConstantPtr* constPtr = static_cast<ConstantPtr*>(constantValue);
				llvmConstantValue = createConstantPtr(constPtr->getValue());
				_tessaValueToLlvmValueMap->put(value, llvmConstantValue);
			}
		}

		TessaAssert(value != NULL);
		TessaAssert(containsLlvmValue(value));
		return _tessaValueToLlvmValueMap->get(value);
	}

	bool LlvmIRGenerator::containsLlvmValue(TessaValue* tessaValue) {
		return _tessaValueToLlvmValueMap->containsKey(tessaValue);
	}

	llvm::Function* LlvmIRGenerator::getLlvmFunction(const LlvmCallInfo* llvmCallInfo) {
		llvm::Function* functionToGet = this->_module->getFunction(llvmCallInfo->methodName);
		TessaAssert(functionToGet != NULL);
		return functionToGet;
	}

	llvm::ConstantInt* LlvmIRGenerator::createConstantInt(int integerValue) {
		bool isSigned = true;
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), integerValue, isSigned);
	}

	llvm::ConstantInt* LlvmIRGenerator::createConstantUnsignedInt(uint32_t integerValue) {
		bool isSigned = false;
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), integerValue, isSigned);
	}

	llvm::ConstantInt* LlvmIRGenerator::createConstantBoolean(bool value) {
		bool isSigned = false;
		return llvm::ConstantInt::get(_booleanType, value, isSigned);
	}

	llvm::IntToPtrInst* LlvmIRGenerator::createConstantPtr(intptr_t pointer) {
		llvm::ConstantInt* address = createConstantInt(pointer);
		return castIntToPtr(address, "");
	}

	llvm::Constant* LlvmIRGenerator::createConstantDouble(double doubleValue) {
		return llvm::ConstantFP::get(_doubleType, doubleValue);
	}

	llvm::IntToPtrInst* LlvmIRGenerator::castIntToPtr(llvm::Value* integer, std::string castName = "") {
		TessaAssert(llvmBasicBlock != NULL);
		TessaAssert(integer->getType()->isIntegerTy());
		return new llvm::IntToPtrInst(integer, _pointerType, castName, llvmBasicBlock);
	}

	llvm::PtrToIntInst* LlvmIRGenerator::castPtrToInt(llvm::Value* pointer, std::string castName = "") {
		TessaAssert(pointer->getType()->isPointerTy());
		return new llvm::PtrToIntInst(pointer, _atomType, castName, llvmBasicBlock);
	}

	/***
	 * Returns the loaded value as whatever type the pointer points to
	 */
	llvm::Instruction* LlvmIRGenerator::createLlvmLoad(llvm::Value* pointer, int offset, llvm::BasicBlock* basicBlockToInsert) {
		llvm::Value* llvmOffset = createConstantInt(offset);
		llvm::GetElementPtrInst* addressCalculation = llvm::GetElementPtrInst::Create(pointer, llvmOffset, "", basicBlockToInsert);
		return new llvm::LoadInst(addressCalculation, "", basicBlockToInsert);
	}

	// Returns loaded value as an int
	llvm::Instruction* LlvmIRGenerator::createLlvmLoad(llvm::Value* pointer, int offset) {
		llvm::Value* llvmPointer = pointer;
		TessaAssert(pointer->getType()->isPointerTy());
		if (pointer->getType() != _pointerType) {
			llvmPointer = createBitCast(pointer, _pointerType);
		} 

		return createLlvmLoad(llvmPointer, offset, llvmBasicBlock);
	}

	// Returns loaded value as a double
	llvm::Instruction* LlvmIRGenerator::createLlvmLoadDouble(llvm::Value* pointer, int offset) {
		llvm::Value* llvmPointer = createBitCast(pointer, _doublePointerType);
		return createLlvmLoad(llvmPointer, offset, llvmBasicBlock);
	}

	// Returns loaded value as a boolean 
	llvm::Instruction* LlvmIRGenerator::createLlvmLoadBoolean(llvm::Value* pointer, int offset) {
		llvm::Value* llvmPointer = createBitCast(pointer, _booleanPointerType);
		return createLlvmLoad(llvmPointer, offset, llvmBasicBlock);
	}

	// Returns loaded value as a pointer
	llvm::Instruction* LlvmIRGenerator::createLlvmLoadPtr(llvm::Value* pointer, int offset) {
		return castIntToPtr(createLlvmLoad(pointer, offset));
	}

	// Returns the calculated address as a pointer value
	llvm::Instruction* LlvmIRGenerator::doPointerArithmetic(llvm::Value* pointer, int offset) {
		llvm::Value* base = pointer;
		if (pointer->getType()->isPointerTy()) {
			base = castPtrToInt(pointer);
		}

		TessaAssert(base->getType()->isIntegerTy());
		llvm::Value* addedAddress = llvm::BinaryOperator::Create(Instruction::Add, base, createConstantInt(offset), "", llvmBasicBlock);
		return castIntToPtr(addedAddress);
	}

	/***
	 * Stores the valueToStore into the pointer[offset].
	 * Here offset is defined as the offset that the llvm GEP instruction will use in address calculation
	 * This assumes that pointer and value to store are of the correct type already
	 */
	void LlvmIRGenerator::createLlvmStore(llvm::Value* pointer, llvm::Value* valueToStore, int offset) {
		TessaAssert(pointer->getType()->isPointerTy());
		TessaAssertMessage(valueToStore->getType() == pointer->getType()->getContainedType(0), "Probably didn't cast value to the llvm pointer type");

		llvm::Value* llvmOffset = createConstantInt(offset);
		llvm::GetElementPtrInst* addressCalculation = llvm::GetElementPtrInst::Create(pointer, llvmOffset, "", llvmBasicBlock);
		new llvm::StoreInst(valueToStore, addressCalculation, llvmBasicBlock);
	}

	void LlvmIRGenerator::createLlvmStoreInt(llvm::Value* pointer, llvm::Value* valueToStore, int offset) {
		llvm::Value* llvmPointer = pointer;
		const llvm::Type* pointsToType = pointer->getType()->getContainedType(0);
		TessaAssert(valueToStore->getType()->isIntegerTy());

		if (pointsToType != _intType) {
			llvmPointer = createBitCast(pointer, _pointerType);
		}

		//TessaAssert(pointsToType->isPointerTy());
		createLlvmStore(llvmPointer, valueToStore, offset);
	}

	void LlvmIRGenerator::createLlvmStoreDouble(llvm::Value* pointer, llvm::Value* valueToStore, int offset) {
		llvm::Value* llvmPointer = pointer;
		TessaAssert(valueToStore->getType()->isDoubleTy());

		const llvm::Type* pointsToType = pointer->getType()->getContainedType(0);
		if (pointsToType != _doublePointerType) {
			llvmPointer = createBitCast(pointer, _doublePointerType);
		}

		createLlvmStore(llvmPointer, valueToStore, offset);
	}

	void LlvmIRGenerator::createLlvmStorePointer(llvm::Value* pointer, llvm::Value* valueToStore, int offset) {
		llvm::Value* llvmPointer = pointer;
		const llvm::Type* pointsToType = pointer->getType()->getContainedType(0);
		TessaAssert(pointsToType->isPointerTy());
		TessaAssert(valueToStore->getType()->isPointerTy());

		if (pointsToType != _pointerType) {
			llvmPointer = createBitCast(pointer, _pointerType);
		}

		createLlvmStore(llvmPointer, valueToStore, offset);
	}

	llvm::CallInst* LlvmIRGenerator::createLlvmCallInstruction(const LlvmCallInfo* llvmCallInfo, int argCount, ...) {
		llvm::Function* functionToCall = getLlvmFunction(llvmCallInfo);
		std::string functionName = functionToCall->getName();
		std::vector<llvm::Value*> arguments;

		va_list ap;
		va_start(ap, argCount);
		for (int i = 0; i < argCount; i++) {
			arguments.push_back( va_arg(ap, llvm::Value*) );
		}
		va_end(ap);

		return createLlvmCallInstruction(functionToCall, functionToCall->getFunctionType(), functionName, &arguments, llvmCallInfo->callingConvention);
	}

	/***
	 * This creates a call to an indirect address which is supplied as the first argument.
	 * This also assumes that the calling convention is CDECL
	 *
	 * IndirectAddress parameter must be a function pointer.
	 */
	llvm::CallInst* LlvmIRGenerator::createLlvmIndirectCallInstruction(llvm::Value* indirectAddress, const llvm::FunctionType* functionType, std::string functionName, int argCount, ...) {
		std::vector<llvm::Value*> arguments;
		va_list ap;
		va_start(ap, argCount);
		for (int i = 0; i < argCount; i++) {
			arguments.push_back( va_arg(ap, llvm::Value*) );
		}
		va_end(ap);

		TessaAssertMessage(indirectAddress->getType()->isPointerTy(), "Indirect address must be a pointer type");
		llvm::PointerType* functionPointerType = llvm::PointerType::get(functionType, 0);
		llvm::Value* functionPointerInstance = createBitCast(indirectAddress, functionPointerType);
		return createLlvmCallInstruction(functionPointerInstance, functionType, functionName, &arguments, ABI_CDECL);
	}

	/***
	 * As a side affect, this casts the arguments to the type required by the llvm::function signature
	 */
	llvm::CallInst*	LlvmIRGenerator::createLlvmCallInstruction(llvm::Value* valueToCall, const llvm::FunctionType* functionType, std::string functionName, std::vector<llvm::Value*>* arguments, AbiKind abiKind) {
		TessaAssertMessage(arguments->size() == functionType->getNumParams(), "Incorrect number of parameters");
		std::vector<llvm::Value*> castedArguments; 
		castCallArgumentsToDeclaredTypes(&castedArguments, functionType, arguments);

		if (functionType->getReturnType()->isVoidTy()) {
			functionName = "";	// Void types can't have function names
		}
		llvm::CallInst* callInstruction = CallInst::Create(valueToCall, castedArguments.begin(), castedArguments.end(), functionName, llvmBasicBlock);
		setLlvmCallingConvention(callInstruction, abiKind);
		return callInstruction;
	}

	void LlvmIRGenerator::castCallArgumentsToDeclaredTypes(std::vector<llvm::Value*>* castedArgumentsLocation, const llvm::FunctionType* functionType, std::vector<llvm::Value*>* arguments) {
		for (uint32_t i = 0; i < functionType->getNumParams(); i++) {
			const llvm::Type* functionParamType = functionType->getParamType(i);
			llvm::Value* argumentValue = arguments->at(i);
			const llvm::Type* argumentParamType = argumentValue->getType();

			if (functionParamType != argumentParamType) {
				// Zero extend booleans to the proper integer value
				if (functionParamType->isIntegerTy() && (argumentParamType == llvm::Type::getInt1Ty(*context))) {
					castedArgumentsLocation->push_back(llvm::ZExtInst::Create(llvm::Instruction::ZExt, argumentValue, functionParamType, "", llvmBasicBlock));
				} else {
					castedArgumentsLocation->push_back(new llvm::BitCastInst(argumentValue, functionParamType, "", this->llvmBasicBlock));
				}
			} else {
				castedArgumentsLocation->push_back(argumentValue);
			}
		}
	}

	void LlvmIRGenerator::setLlvmCallingConvention(llvm::CallInst* callInstruction, AbiKind callingConvention) {
		switch (callingConvention) {
			case ABI_THISCALL:
				callInstruction->setCallingConv(llvm::CallingConv::X86_ThisCall);
				break;
			case ABI_FASTCALL:
				callInstruction->setCallingConv(llvm::CallingConv::X86_FastCall);			
				break;
			default:
				callInstruction->setCallingConv(llvm::CallingConv::C);
				break;
		}
	}

	// emit code to create a stack-allocated copy of the given multiname.
    // this helper only initializes Multiname.flags and Multiname.next_index
	llvm::Value* LlvmIRGenerator::copyMultiname(const Multiname* multiname) {
		llvm::Value* stackAllocatedMultiname = new llvm::AllocaInst(_intType, createConstantInt(sizeof(Multiname)), "MultinameCopy", llvmBasicBlock);
        createLlvmStore(stackAllocatedMultiname, createConstantInt(multiname->ctFlags()), offsetof(Multiname, flags) / sizeof(int32_t));
        createLlvmStore(stackAllocatedMultiname, createConstantInt(multiname->next_index), offsetof(Multiname, next_index) / sizeof(int32_t));
        return stackAllocatedMultiname;
    }

	llvm::Value* LlvmIRGenerator::initMultiname(const Multiname* multiname, TessaInstruction* namespaceInstruction, bool isDeleteProp = false) {
		if (!multiname->isRuntime()) {
            // use the precomputed multiname
            return createConstantPtr((intptr_t)multiname);
        }

        // create an initialize a copy of the given multiname
		llvm::Value* stackMultinameCopy = copyMultiname(multiname);

        // then initialize its name and ns|nsset fields.
		llvm::Value* nameAtom = NULL;
		if (multiname->isRtname()) {
			nameAtom = getValue(namespaceInstruction);
		} else {
            // copy the compile-time name to the temp name
			llvm::Value* mName = createConstantPtr((intptr_t)multiname->name);
            createLlvmStore(stackMultinameCopy, mName, offsetof(Multiname,name) / sizeof(int32_t));
        }

		if (multiname->isRtns()) {
            // intern the runtime namespace and copy to the temp multiname
			TessaAssert(false);
			/*
			llvm::Value* namespaceAtom = loadAtomRep(csp--);
            LIns* internNs = callIns(FUNCTIONID(internRtns), 2, env_param, nsAtom);
            stp(internNs, _tempname, offsetof(Multiname,ns), ACC_OTHER);
			*/
		} else {
            // copy the compile-time namespace to the temp multiname
			//llvm::Value* namespaceValue = createConstantPtr((intptr_t)multiname->ns);
			llvm::Value* namespaceValue = createConstantInt((intptr_t)multiname->ns);
            createLlvmStore(stackMultinameCopy, namespaceValue, offsetof(Multiname, ns) / sizeof(int32_t));
        }

        // Call initMultinameLate as the last step, since if a runtime
        // namespace is present, initMultinameLate may clobber it if a
        // QName is provided as index.
		if (nameAtom) {
			if (isDeleteProp) {
                //callIns(FUNCTIONID(initMultinameLateForDelete), 3, env_param, _tempname, nameAtom);
				TessaAssert(false);
			} else {
				createLlvmCallInstruction(GET_LLVM_CALLINFO(InitMultinameLate), 3, _avmcorePointer, stackMultinameCopy, nameAtom);
            }
        }

        return stackMultinameCopy;
    }

	/***
	 * LLVM's alloc instruction works a bit differently than nanojits. 
	 * NanoJIT allocs space on the stack only ONCE per alloc.
	 * LLvm will alloc per ALLOC instruction. So if there is a loop, with an alloc in it, nanojit
	 * will use the same alloced space over and over again. LLVM will keep allocing the requested space.
	 * This blows up the stack space in LLVM. So go through the function, and allocate one block for ALL
	 * the arguments pointers ever to be used at the jit code prologue.
	 */
	llvm::Value* LlvmIRGenerator::allocateArgumentsPointer(List<TessaVM::BasicBlock*, LIST_GCObjects>* basicBlocks) {
		maxArgPointerSize = 0;

		for	(uint32_t i = 0; i < basicBlocks->size(); i++) {
			TessaVM::BasicBlock* currentBasicBlock = basicBlocks->get(i);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = currentBasicBlock->getInstructions();

			for (uint32_t j = 0; j < instructions->size(); j++) {
				TessaInstruction* currentInstruction = instructions->get(j);
				if (currentInstruction->isArrayOfInstructions()) {
					ArrayOfInstructions* arrayOfInstructions = (ArrayOfInstructions*) currentInstruction;
					int size = arrayOfInstructions->size();
					if (size > (int)maxArgPointerSize) {
						maxArgPointerSize = size;
					}
				}
				else if (currentInstruction->isNewArray()) {
					NewArrayInstruction* newArray = (NewArrayInstruction*) currentInstruction;
					int arraySize = newArray->numberOfElements();
					if (arraySize > (int) maxArgPointerSize) {
						maxArgPointerSize = arraySize;
					}
				}
			}
		}	

		/***
		 * We only count for the number of arguments here, which may all be doubles.
		 * However, LLVM alloc here thinks we are a pointer size. So double the allocation size
		 * to potentially accomodate all native doubles;
		 */
		TessaAssert((sizeof(double) / 2) == sizeof(int));
		maxArgPointerSize = maxArgPointerSize * 2;
		llvm::Value* allocSize = createConstantInt(maxArgPointerSize);
		return new llvm::AllocaInst(_pointerType, allocSize, "Args Pointer", llvmBasicBlock);
	}

	void LlvmIRGenerator::setMethodEnvPointers() {
		_methodEnvScopeChain = createLlvmLoadPtr(_envPointer, offsetof(MethodEnv, _scope) / sizeof(int32_t));
		_methodEnvVTable = createLlvmLoadPtr(_methodEnvScopeChain, offsetof(ScopeChain, _vtable) / sizeof(int32_t));
		_toplevelPointer = createLlvmLoadPtr(_methodEnvVTable, offsetof(VTable, _toplevel) / sizeof(int32_t));

		_methodEnvCallStack->add(_envPointer);
		_methodEnvVTableStack->add(_methodEnvVTable);
		_methodEnvScopeChainStack->add(_methodEnvScopeChain);
		_toplevelPointerStack->add(_toplevelPointer);
	};

	void LlvmIRGenerator::insertConstantAtoms(TessaVM::ASFunction* asFunction) {
		TessaAssert(llvmBasicBlock != NULL);
		_envPointer = _llvmFunction->arg_begin();
		setMethodEnvPointers();
		_avmcorePointer = createConstantPtr((intptr_t)this->core);

		_falseAtom = createConstantInt(AtomConstants::falseAtom);
		_trueAtom = createConstantInt(AtomConstants::trueAtom);
		_undefinedAtom = createConstantInt(AtomConstants::undefinedAtom);
		_nullObjectAtom = createConstantInt(AtomConstants::nullObjectAtom);

		// Allocing 0 is undefined
		if (asFunction->getScopeStackSize() != 0) {
			llvm::Value* maxScopeStack = createConstantInt(asFunction->getScopeStackSize());
			_scopeStack = new llvm::AllocaInst(_pointerType, maxScopeStack, "Scope Stack", llvmBasicBlock);
		} else {
			_scopeStack = new llvm::AllocaInst(_pointerType, createConstantInt(1), "Scope Stack", llvmBasicBlock);
		}

		_currentScopeDepth = createConstantInt(0);
	}

	const llvm::Type* LlvmIRGenerator::getLlvmType(TessaTypes::Type* tessaType) {
		if (tessaType->isNumber()) {
			return _doubleType;
		} else if (tessaType->isUnsignedInt()) {
			return _uintType;
		} else if (tessaType->isInteger()) {
			return _intType;
		} else if (tessaType->isPointer()) {
			return _pointerType;
		} else if (tessaType->isObject()) {
			// This seems like a bug. Shouldn't objects be Atoms?
			//TessaAssert(false);
			return _atomType;
		} else if (tessaType->isBoolean()) {
			return _booleanType;
		} else if (tessaType->isAnyType()) {
			return _atomType;
		} else {
			return _atomType;
		}
	}

	/***
	 *  Fetches the appropriate pointer type for the given tessa type used in GEP/Load/STore llvm instructions
	 */
	const llvm::Type* LlvmIRGenerator::getPointerType(TessaTypes::Type* tessaType) {
		if (tessaType->isPointer()) {
			return _pointerType;
		} else if (tessaType->isNumber()) {
			return _doubleType;
		} else {
			return _atomType;
		}
	}

	void LlvmIRGenerator::buildJitTimeOptionalParameter(TessaInstruction* parameter, Atom defaultValue, int parameterIndex, int optionalIndex, int requiredCount, int displacement, TessaVM::BasicBlock* preOptionalBlock) {
		char paramName[32];
		VMPI_snprintf(paramName, sizeof(paramName), "Opt Param %d", parameterIndex);
		llvm::Argument* argCount = getArgCountParameter();
		llvm::Argument* argParameters = getAtomArgumentsParameter();

		/***
		 * Each optional parameter is an allocated stack value. Store the default value at stack location.
		 * Check if arg count > optional count, if so load the arg and store it back into the allocated stack slot.
		 * Then reload the stack slot.
		 * 
		 * Build the C equivalent of:
		 * if (argCount > requiredParameters) {
		 *		locals[optionalIndex] = arguments[required_index + optionalIndex];
		 * }
		 * localVar = locals[optionalIndex];
		 *
		 */
		TessaTypes::Type* nativeType = parameter->getType();
		const llvm::Type* llvmPointerType = getPointerType(nativeType);
		
		// Store locals[argument] = defaultValue
		// May have to store a double type
		int paramSize = sizeof(double) / sizeof(int32_t);
		TessaAssert(paramSize != 0);
		llvm::Value* paramLocation = new llvm::AllocaInst(llvmPointerType, createConstantInt(paramSize), paramName, llvmBasicBlock);
		llvm::Value* llvmDefaultValue = createConstantInt(defaultValue);
		llvmDefaultValue = atomToNative(llvmDefaultValue, nativeType);
		if (parameter->isBoolean()) {
			llvmDefaultValue = castBooleanToNative(llvmDefaultValue, _tessaTypeFactory->integerType()); 
		}
		createLlvmStore(paramLocation, llvmDefaultValue, 0);

		// then generate: if (argc > p) local[p+1] = arg[p+1], so overwrite only if it exists
		llvm::BasicBlock* optionalBasicBlock = llvm::BasicBlock::Create(*context, "", _llvmFunction);
		llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "", _llvmFunction);

		// if (argCount > requiredParameterCount)
		llvm::Value* ifCondition = new ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_SGT, argCount, createConstantInt(optionalIndex + requiredCount), "");
		llvm::Value* branchInstruction = llvm::BranchInst::Create(optionalBasicBlock, mergeBlock, ifCondition, llvmBasicBlock);
		
		// We are now in the if arguments
		// unboxedValue = arguments[paramCount + 1]
		// This is really pointer arithmetic, [stackAddress + displacement]
		llvm::Value* argParametersInt = new llvm::PtrToIntInst(argParameters, _intType, paramName, optionalBasicBlock);
		llvm::Value* loadLocationInteger = llvm::BinaryOperator::Create(Instruction::Add, argParametersInt, createConstantInt(displacement), "", optionalBasicBlock);

		llvm::Value* loadLocationPointer;
		if (parameter->isPointer()) { 
			loadLocationPointer = new llvm::IntToPtrInst(loadLocationInteger, llvmPointerType, "", optionalBasicBlock);
		} else if (parameter->isNumber()) {
			loadLocationPointer = new llvm::IntToPtrInst(loadLocationInteger, _pointerType, "", optionalBasicBlock);
			loadLocationPointer = new llvm::BitCastInst(loadLocationPointer, _doublePointerType, "", optionalBasicBlock);
		} else {
			// Integers and uints aren't pointers, but we have to trick llvm into thinking they are so we can actually do a load
			loadLocationPointer = new llvm::IntToPtrInst(loadLocationInteger, _pointerType, "", optionalBasicBlock);
		}

		llvm::Value* loadedNativeValue = new llvm::LoadInst(loadLocationPointer, "", optionalBasicBlock);
		if (parameter->isPointer()) {
			// Load returns an integer :(
			loadedNativeValue = new llvm::IntToPtrInst(loadedNativeValue, _pointerType, paramName, optionalBasicBlock);
		} else if (parameter->isNumber()) {
			TessaAssert(loadedNativeValue->getType()->getTypeID() == 2);
		}

		// local[paramCount + 1] = realValue
		llvm::GetElementPtrInst* addressCalculation = llvm::GetElementPtrInst::Create(paramLocation, createConstantInt(0), "", optionalBasicBlock);
		new llvm::StoreInst(loadedNativeValue, addressCalculation, optionalBasicBlock);
		llvm::Instruction* branchToMerge = llvm::BranchInst::Create(mergeBlock, optionalBasicBlock);

		// Force the first tessa basic block to be the merge block
		llvmBasicBlock = mergeBlock;
		_tessaToLlvmBasicBlockMap->put(preOptionalBlock, llvmBasicBlock);

		// Reload the real value of the parameter. It's either the default value or real value 
		llvm::Value* paramValue;
		if (nativeType->isPointer()) {
			paramValue = createLlvmLoadPtr(paramLocation, 0);
		} else if (nativeType->isNumber()) {
			paramValue = createLlvmLoadDouble(paramLocation, 0);
		} else if (nativeType->isBoolean()) {
			paramValue = createLlvmLoad(paramLocation, 0);
			paramValue = castToNative(paramValue, _tessaTypeFactory->integerType(), _tessaTypeFactory->boolType());
		} else {
			paramValue = createLlvmLoad(paramLocation, 0);
		}

		putValue(parameter, paramValue);
	}

	void LlvmIRGenerator::initOptionalParameters(TessaVM::BasicBlock* firstBasicBlock) {
		TessaAssert(firstBasicBlock->getBasicBlockId() == 0);
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = firstBasicBlock->getInstructions();

		if (methodInfo->hasOptional()) {
			int32_t optionalCount = methodSignature->optional_count();
			int32_t requiredCount = methodSignature->param_count() - optionalCount;
			TessaAssert(requiredCount == methodSignature->requiredParamCount());
			checkPointerSizes();

			int displacement = 0;
			// <= because the 0th argument is the "this" parameter
			for (int i = 0; i <= requiredCount; i++) {
				TessaTypes::Type* tessaType = getTessaType(methodSignature->paramTraits(i));
				if (tessaType == _tessaTypeFactory->numberType()) {
					displacement += sizeof(double);
				} else {
					displacement += sizeof(int);
				}
			}
			
			/***
			 * Assume function:
			 * function testFunction(firstParameter, otherParameter = default)
			 * Arguments are laid out like this:
			 *   0     1               2       
			 * "this"  firstParameter  otherParameter
			 *
			 * Required count will be 1, optional count will be 1
			 * To actually get the default value, need to offset optionalCount + requiredCount which will get us to the optinal arguments
			 */
			for (int optionalIndex = 0; optionalIndex < optionalCount; optionalIndex++) {
				int parameterIndex = optionalIndex + requiredCount + 1;	// + 1. Optional starts at 0, but we need to get the param AFTER all the required parameters
				TessaInstruction* parameter = instructions->get(parameterIndex);
				TessaAssert(parameter->isParameter());
				Atom defaultValue = methodSignature->getDefaultValue(optionalIndex);	// MethodSignature auto corrects the index

				buildJitTimeOptionalParameter(parameter, defaultValue, parameterIndex, optionalIndex, requiredCount, displacement, firstBasicBlock);

				TessaTypes::Type* paramType = parameter->getType();
				if (paramType->isNumber()) {
					displacement += sizeof(double);
				} else {
					displacement += sizeof(int);
				}
			}
		}
	}

	llvm::Argument* LlvmIRGenerator::getArgCountParameter() {
		// Really need to find a better way to get the arguments
		llvm::Function::arg_iterator argIter = _llvmFunction->arg_begin();
		for (int i = 0; i < 1; i++) {
			argIter++;
		}

		return argIter;
	}

	llvm::Argument* LlvmIRGenerator::getAtomArgumentsParameter() {
		// Really need to find a better way to get the third argument :(. llvm->functionarg_end() already passes the arguments
		llvm::Function::arg_iterator argIter = _llvmFunction->arg_begin();
		for (int i = 0; i < 2; i++) {
			argIter++;
		}

		return argIter;
	}

	/***
	 * We assume the method signature of a method call is invoke(MethodEnv* env, int argCount, Atom* arguments);
	 * However, Arguments isn't necessarily an array of Atoms. Everything should be unboxed and coerced
	 * to the appropriate declared types by the caller method. 
	 */
	void LlvmIRGenerator::copyRequiredParameters(TessaVM::BasicBlock* firstBasicBlock) {
		llvm::Argument* atomArguments = getAtomArgumentsParameter();
		TessaAssert(atomArguments->getType()->isPointerTy());
		checkPointerSizes();

		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		int requiredArgCount = methodSignature->requiredParamCount(); 
		List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = firstBasicBlock->getInstructions();

		int displacement = 0;
		for (int i = 0; i <= requiredArgCount; i++) {
			TessaInstruction* parameter = instructions->get(i);
			TessaAssert(parameter->isParameter());

			llvm::GetElementPtrInst* atomAddress = llvm::GetElementPtrInst::Create(atomArguments, createConstantInt(displacement / sizeof(int32_t)), "", llvmBasicBlock);
			llvm::Value* llvmParameterValue;
			if (parameter->isNumber()) {
				/***
				 * We can pass number arguments in non-aligned sizes eg argsPointer[4] = someDouble. However, GEP won't let us address
				 * argsPointer[4] since the base address is based on a word size of 4.
				 * so manually increment the pointer to argPointer + 4. Set that value to a double pointer type, and load from offset 0. Ugly ugly ugly
				 */
				llvm::Value* doubleLocationPtr = doPointerArithmetic(atomArguments, displacement); 
				llvmParameterValue = createLlvmLoadDouble(doubleLocationPtr, 0);
				displacement += sizeof(double);
			} else {
				llvm::Value* loadInstruction = new llvm::LoadInst(atomAddress, "", llvmBasicBlock);
				llvmParameterValue = loadInstruction;

				if (parameter->isPointer()) {
					llvmParameterValue = castIntToPtr(llvmParameterValue);
				} else if (parameter->isBoolean()) {
					// We loaded an int32ty, have to cast to native llvm's int1ty
					llvmParameterValue = castToNative(llvmParameterValue, _tessaTypeFactory->integerType(), _tessaTypeFactory->boolType());
				}

				displacement += sizeof(int32_t);
			}
				
			putValue(parameter, llvmParameterValue);
		}
	}

	void LlvmIRGenerator::createRestParameters(TessaVM::BasicBlock* firstBasicBlock) {
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		llvm::Argument* atomArguments = getAtomArgumentsParameter();
		llvm::Argument* argCountArgument = getArgCountParameter();
		List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = firstBasicBlock->getInstructions();

		/**
		 * Fill out the "REST" arguments AFTER the normal parameter instructions
		 * Also, we have to explicitly remap some the REST parameter AFTER we put in the "real" passed in arguments
		 * Ask the semantics of the interpreter why this is
		 */
		if (methodInfo->needRest()) {
			int parameterCount = methodSignature->param_count();
			TessaInstruction* parameter = instructions->get(parameterCount + 1);
			TessaAssert(parameter->isParameter());
			llvm::Value* restParameters = createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvCreateRest), 3, _envPointer, atomArguments, argCountArgument);
			TessaAssert(parameter->isArray());
			restParameters = createBitCast(restParameters, _pointerType);
			putValue(parameter, restParameters);
        } else if (methodInfo->needArguments()) {
			TessaAssert(false);
            // create arguments using atomv[1..argc].
            // Even tho E3 says create an Object, E4 says create an Array so thats what we will do.
            //framep[param_count+1] = env->createArguments(_atomv, arguments_argc)->atom();
        }
	}

	void LlvmIRGenerator::unboxParametersToNativeTypes(TessaVM::BasicBlock* firstBasicBlock) {
		TessaAssert(false);
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		List<ParameterInstruction*, LIST_GCObjects>* parameters = firstBasicBlock->getParameterInstructions();

		for (int i = 0; i < methodSignature->param_count(); i++) {
			TessaInstruction* tessaParam = parameters->get(i + 1);	// + 1 since the 0th atom arg is the "this" param
			llvm::Value* parameter = getValue(tessaParam);
			parameter = atomToNative(parameter, tessaParam->getType());
			putValue(tessaParam, parameter);
		}

		TessaInstruction* tessaParam = parameters->get(0);	// Coerce the "this" object
		llvm::Value* thisParameter= getValue(tessaParam);

		thisParameter = atomToNative(thisParameter, tessaParam->getType());
		putValue(tessaParam, thisParameter);
	}

	/***
	 * Argument data structures are kind of weird. Even if there is no explicit default value in AS source,
	 * the method signature assumes that the default value for a variable is undefined.
	 * Therefore, we have to initialize all arguments to undefined, THEN overwrite them
	 * with the real passed in values.
	 */
	void LlvmIRGenerator::setParametersToUndefined(TessaVM::BasicBlock* firstBasicBlock) {
		TessaAssert(firstBasicBlock->getBasicBlockId() == 0);
		List<ParameterInstruction*, LIST_GCObjects>* parameters = firstBasicBlock->getParameterInstructions();
		for (uint32_t i = 0; i < parameters->size(); i++) {
			putValue(parameters->get(i), _undefinedAtom);
		}
	}

	void LlvmIRGenerator::mapParametersToValues(TessaVM::BasicBlock* firstBasicBlock) {
		bool isSigned = false;
		TessaAssert(firstBasicBlock->getBasicBlockId() == 0);
		currentBasicBlock = firstBasicBlock;
		
		TessaAssert(_llvmFunction->arg_size() == 3);	// Method*, argc, Atom*

		setParametersToUndefined(firstBasicBlock);
		initOptionalParameters(firstBasicBlock);
		copyRequiredParameters(firstBasicBlock);
		createRestParameters(firstBasicBlock);
	}

	llvm::BasicBlock* LlvmIRGenerator::getLlvmBasicBlock(TessaVM::BasicBlock* tessaBasicBlock) {
		llvm::BasicBlock* foundBasicBlock = _tessaToLlvmBasicBlockMap->get(tessaBasicBlock);
		TessaAssert(foundBasicBlock!= NULL);
		return foundBasicBlock;
	}
	
	void LlvmIRGenerator::createLlvmBasicBlocks(ASFunction* function) {
		char basicBlockName[32];
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> *basicBlocks = function->getBasicBlocksInReversePostOrder();

		for (uint32_t basicBlockIndex = 0; basicBlockIndex < basicBlocks->size(); basicBlockIndex++) {
			TessaVM::BasicBlock* tessaBasicBlock = basicBlocks->get(basicBlockIndex);
			sprintf(basicBlockName, "BB%d", tessaBasicBlock->getBasicBlockId());

			llvm::BasicBlock* newLlvmBasicBlock = llvm::BasicBlock::Create(*context, basicBlockName, _llvmFunction);
			TessaAssert(newLlvmBasicBlock != NULL);
			_tessaToLlvmBasicBlockMap->put(tessaBasicBlock, newLlvmBasicBlock);
		}
	}

	void LlvmIRGenerator::visitBlocksInReversePostOrder(ASFunction* function) {
		debugMessage("\n\n ======= Creating LLVM IR ========\n");
		TessaVM::BasicBlock* rootBlock = function->getEntryBlock();
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* reversePostOrderList = function->getBasicBlocksInReversePostOrder();

		for (uint32_t basicBlockIndex = 0; basicBlockIndex < reversePostOrderList->size(); basicBlockIndex++) {
			currentBasicBlock = reversePostOrderList->get(basicBlockIndex);
			#ifdef DEBUG
			if (core->config.tessaVerbose) {
				printf("Basic block: %d\n", currentBasicBlock->getBasicBlockId());
			}
			#endif

			llvmBasicBlock = getLlvmBasicBlock(currentBasicBlock);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = currentBasicBlock->getInstructions();

			for (uint32_t instructionIndex = 0; instructionIndex < instructions->size(); instructionIndex++) {
				TessaInstruction* currentInstruction = instructions->get(instructionIndex);

#ifdef DEBUG
				if (core->config.tessaVerbose) {
					currentInstruction->print();
				}
#endif

				currentInstruction->visit(this);
			}

			/***
			 *  We may create new basic blocks when we inline specialized cases during LLVM IR creation
			 * This maps the actual llvm END basic block. Thus a TESSA basic block may start at one llvm block
			 * and "end" at a different llvm basic block.
			 */
			this->_tessaToEndLlvmBasicBlockMap->put(currentBasicBlock, llvmBasicBlock);
		}
	}

	void LlvmIRGenerator::createLIR(MethodInfo* methodInfo, ASFunction* function) {
		this->methodInfo = methodInfo;
		createLlvmBasicBlocks(function);
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects> *basicBlocks = function->getBasicBlocks();
		llvmBasicBlock = getLlvmBasicBlock(function->getEntryBlock());

		insertConstantAtoms(function);
		_allocedArgumentsPointer = allocateArgumentsPointer(basicBlocks);
		mapParametersToValues(function->getEntryBlock());

		createPrologue(methodInfo);
		visitBlocksInReversePostOrder(function);
		addLlvmPhiValues(basicBlocks);
		//createEpilogue();	 Occurs right before the return
	}

	void LlvmIRGenerator::visit(TessaInstruction* tessaInstruction) {
		TessaAssertMessage(false, "Should not create llvm ir for a generic tessa instruction");
	}

	llvm::Value* LlvmIRGenerator::emitFindDefinition(FindPropertyInstruction* findPropertyInstruction, llvm::Value* multinamePointer) {
		if (findPropertyInstruction->useFindDefCache) {
			int cacheSlot = findPropertyInstruction->getCacheSlot();
			llvm::Value* scriptObjectCache = createLlvmCallInstruction(GET_LLVM_CALLINFO(finddef_cache), 3, _envPointer, multinamePointer, createConstantInt(cacheSlot));
			return createBitCast(scriptObjectCache, _pointerType);
		} else {
			TessaAssert(false);
			//return createLlvmCallInstruction(GET_LLVM_CALLINFO(finddef_slow), 2, envPointer, multinamePointer);
		}
	}

	void LlvmIRGenerator::visit(FindPropertyInstruction* findPropertyInstruction) {
		const Multiname* multiName = findPropertyInstruction->getMultiname();
		llvm::Value* multinamePointer = castIntToPtr(createConstantInt((intptr_t)multiName)); 
		llvm::Value* foundDefinition;
		
		if (findPropertyInstruction->isFindDefinition()) {
			foundDefinition = emitFindDefinition(findPropertyInstruction, multinamePointer);
		} else {
			/***
			 * Todo:
			 * Work in with scope, and the real scope object
			 * MethodEnvFindProp should return script object not Atom
			 */
			bool isStrict = findPropertyInstruction->isStrict();
			llvm::Value* isStrictBoolean = createConstantBoolean(isStrict);
			foundDefinition = createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvFindProperty), 3, _envPointer, multinamePointer, isStrictBoolean);
		}

		putValue(findPropertyInstruction, foundDefinition);
	}

	bool LlvmIRGenerator::hasType(TessaValue* tessaValue) {
		return !tessaValue->getType()->isAnyType();
	}

	bool LlvmIRGenerator::isLlvmPointerType(TessaTypes::Type* tessaType) {
		return tessaType->isPointer();
		/*
		switch (tessaType) {
			case VECTOR:
			case &_tessaTypeFactory->anyArrayType():
			case &TessaTypes::objectVectorType:
			case &TessaTypes::intVectorType:
			case &TessaTypes::uintVectorType:
			case &_tessaTypeFactory->numberVectorType():
			case _tessaTypeFactory->scriptObjectType():
			case _tessaTypeFactory->stringType():
				return true;
			default:
				return false;
				*				*/
	}

	bool LlvmIRGenerator::isLlvmIntegerType(TessaTypes::Type* tessaType) {
		TessaAssert(false);
		return false;
	}

	bool LlvmIRGenerator::isAtomType(llvm::Value* value) {
		return value->getType() == _atomType;
	}

	bool LlvmIRGenerator::isPointerType(llvm::Value* value) {
		return value->getType()->isPointerTy();
	}

	bool LlvmIRGenerator::isBoolType(llvm::Value* value) {
		return value->getType() == _booleanType;
	}

	bool LlvmIRGenerator::isInteger(TessaValue* tessaValue) {
		return tessaValue->getType()->isInteger();
	}

	bool LlvmIRGenerator::isVoidType(const llvm::Type* type) {
		return type == llvm::Type::getVoidTy(*context);
	}

	void LlvmIRGenerator::visit(ConstantValueInstruction* constantValueInstruction) {
		ConstantValue* constantValue = constantValueInstruction->getConstantValue();
		llvm::Value* llvmConstantValue;

		if (constantValue->isString()) {
			ConstantString* constantString = (ConstantString*) constantValue;
			Stringp stringValue = constantString->getValue();
			llvmConstantValue = createConstantPtr((intptr_t)stringValue);
		} else if (constantValue->isNull()) {
			llvmConstantValue = _nullObjectAtom;
		} else if (constantValue->isInteger()) {
			TessaTypes::ConstantInt* tessaConstantInteger = (TessaTypes::ConstantInt*) constantValue;
			llvmConstantValue = createConstantInt(tessaConstantInteger->getValue());
		} else if (constantValue->isNumber()) {
			ConstantFloat* constantDouble = (ConstantFloat*) constantValue;
			llvmConstantValue = createConstantDouble(constantDouble->getValue());
		} else if (constantValue->isUndefined()) {
			llvmConstantValue = _undefinedAtom;
		} else if (constantValue->isBoolean()) {
			ConstantBool* constantBool = (ConstantBool*) constantValue;
			if (constantBool->getValue()) {
				llvmConstantValue = createConstantBoolean(true);
			} else {
				llvmConstantValue = createConstantBoolean(false);
			}
		} else {
			TessaAssert(false);
		}

		putValue(constantValueInstruction, llvmConstantValue);
	}

	void LlvmIRGenerator::visit(ReturnInstruction* returnInstruction) {
		llvm::Value* returnValue;

		if (returnInstruction->getIsVoidReturn()) {
			returnValue = _undefinedAtom;
		} else {
			TessaInstruction* tessaReturnValue = returnInstruction->getValueToReturn();
			returnValue = castToNative(getValue(tessaReturnValue), tessaReturnValue->getType(), returnInstruction->getType());

			if (tessaReturnValue->isBoolean() && (returnInstruction->getType()->isBoolean())) { 
				/**
				 * Have to cast it to a full 32 bit integer because the rest of the VM expects it
				 */
				returnValue = castBooleanToNative(returnValue, _tessaTypeFactory->integerType());
			}
		}
		TessaAssert(returnValue != NULL);

		_returnCount++;
		TessaAssertMessage(_returnCount == 1, "Should only have one return instruction");
		createEpilogue();
		llvm::Value* llvmReturnInst = ReturnInst::Create(*context, returnValue, llvmBasicBlock);
		putValue(returnInstruction, llvmReturnInst);
	}

	llvm::Value* LlvmIRGenerator::compileBitwiseBinaryOp(TessaInstructions::TessaBinaryOp opcode, llvm::Value* leftInteger, llvm::Value* rightInteger) {
		TessaAssert(leftInteger->getType() == _intType);
		TessaAssert(rightInteger->getType() == _intType);
		llvm::Instruction* resultInteger;

		switch (opcode) {
			case BITWISE_LSH:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::Shl, leftInteger, rightInteger, "", llvmBasicBlock);
				break;
			case BITWISE_RSH:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::AShr, leftInteger, rightInteger, "", llvmBasicBlock);
				break;
			case BITWISE_AND:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::And, leftInteger, rightInteger, "", llvmBasicBlock);
				break;
			case BITWISE_OR:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::Or, leftInteger, rightInteger, "", llvmBasicBlock);
				break;
			case BITWISE_XOR:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::Xor, leftInteger, rightInteger, "", llvmBasicBlock);
				break;
			default:
				TessaAssertMessage(false, "Unknown bitwise operation");
				break;
		}

		return resultInteger;
	}

	/***
	 * Assumes values are already integers
	 */
	llvm::Value* LlvmIRGenerator::compileIntegerBinaryOp(TessaInstructions::TessaBinaryOp binaryOp, llvm::Value* leftIntegerValue, llvm::Value* rightIntegerValue) {
		TessaAssert(leftIntegerValue->getType() == _intType);
		TessaAssert(rightIntegerValue->getType() == _intType);
		llvm::Instruction* resultInteger;

		switch (binaryOp) {
			case ADD:
				TessaAssert(false);
				break;
			case SUBTRACT:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::Sub, leftIntegerValue, rightIntegerValue, "", llvmBasicBlock);
				break;
			case MULTIPLY:
				resultInteger = llvm::BinaryOperator::Create(llvm::Instruction::Mul, leftIntegerValue, rightIntegerValue, "", llvmBasicBlock);
				break;
			default:
				TessaAssertMessage(false, "Unknown integer binary instruction");
				break;
		}

		TessaAssert(resultInteger!= NULL);
		return resultInteger;
	}

	/*** 
	 * This assumes that the values are already unboxed in native form
	 */ 
	llvm::Value* LlvmIRGenerator::compileDoubleBinaryOp(TessaInstructions::TessaBinaryOp binaryOp, llvm::Value* leftDoubleValue, llvm::Value* rightDoubleValue) {
		TessaAssert(leftDoubleValue->getType() == _doubleType);
		TessaAssert(rightDoubleValue->getType() == _doubleType);
		llvm::Instruction* resultDouble;

		switch (binaryOp) {
			case SUBTRACT:
				resultDouble = llvm::BinaryOperator::Create(llvm::Instruction::FSub, leftDoubleValue, rightDoubleValue, "", llvmBasicBlock);
				break;
			case MULTIPLY:
				resultDouble = llvm::BinaryOperator::Create(llvm::Instruction::FMul, leftDoubleValue, rightDoubleValue, "", llvmBasicBlock);
				break;
			case DIVIDE:
				resultDouble = llvm::BinaryOperator::Create(llvm::Instruction::FDiv, leftDoubleValue, rightDoubleValue, "", llvmBasicBlock);
				break;
			case MOD:
				resultDouble = createLlvmCallInstruction(GET_LLVM_CALLINFO(MathUtilsMod), 2, leftDoubleValue, rightDoubleValue);
				break;
			default:
				TessaAssertMessage(false, "Unknown double binary instruction");
				break;
		}

		TessaAssert(resultDouble != NULL);
		return resultDouble;
	}

	llvm::Value* LlvmIRGenerator::compileUnsignedRightShift(TessaBinaryOp binaryOp, llvm::Value* uintLeft, llvm::Value* uintRight) {
		TessaAssert(binaryOp == BITWISE_URSH);
		// Create ( (uint32_t)(u1 >> (u2 & 0x1F)) );	
		TessaAssert(uintLeft->getType() == _uintType);

		llvm::Value* andedValue = llvm::BinaryOperator::Create(Instruction::And, uintRight, createConstantInt(0x1F), "", llvmBasicBlock);
		return llvm::BinaryOperator::Create(Instruction::LShr, uintLeft, andedValue, "", llvmBasicBlock);
	}

	llvm::Value* LlvmIRGenerator::compileTypedBinaryOperation(TessaInstructions::BinaryInstruction* binaryInstruction) {
		TessaValue* leftInstruction = binaryInstruction->getLeftOperand();
		TessaValue* rightInstruction = binaryInstruction->getRightOperand();

		TessaTypes::Type* resultType = binaryInstruction->getType();
		TessaTypes::Type* leftType = leftInstruction->getType();
		TessaTypes::Type* rightType = rightInstruction->getType();

		llvm::Value* leftOperand = getValue(leftInstruction);
		llvm::Value* rightOperand = getValue(rightInstruction);
		TessaInstructions::TessaBinaryOp binaryOp = binaryInstruction->getOpcode();

		TessaAssert(hasType(leftInstruction) || hasType(rightInstruction));
		llvm::Value* llvmBinaryInstruction;
		
		switch (binaryOp) {
			case ADD:
			{
				TessaTypes::Type* resultType = binaryInstruction->getType();
				if (resultType->isNumber()) {
					leftOperand = castToNative(leftOperand, leftType, _tessaTypeFactory->numberType());
					rightOperand = castToNative(rightOperand, rightType, _tessaTypeFactory->numberType());
					llvmBinaryInstruction = llvm::BinaryOperator::Create(llvm::Instruction::FAdd, leftOperand, rightOperand, "", llvmBasicBlock);
				} else if (resultType->isString()) { 
					leftOperand = castToNative(leftOperand, leftType, _tessaTypeFactory->stringType());
					rightOperand = castToNative(rightOperand, rightType, _tessaTypeFactory->stringType());
					llvmBinaryInstruction = createLlvmCallInstruction(GET_LLVM_CALLINFO(StringConcatStrings), 2, leftOperand, rightOperand);
					llvmBinaryInstruction = createBitCast(llvmBinaryInstruction, _pointerType);
				} else if (resultType == _tessaTypeFactory->integerType()) {
					leftOperand = castToNative(leftOperand, leftType, _tessaTypeFactory->integerType());
					rightOperand = castToNative(rightOperand, rightType, _tessaTypeFactory->integerType());
					llvmBinaryInstruction = llvm::BinaryOperator::Create(llvm::Instruction::Add, leftOperand, rightOperand, "", llvmBasicBlock);
				} else {
					TessaAssert(resultType->isObject()); 
					leftOperand = nativeToAtom(leftOperand, leftType);
					rightOperand = nativeToAtom(rightOperand, rightType);
					llvmBinaryInstruction = createLlvmCallInstruction(GET_LLVM_CALLINFO(op_add), 3, _avmcorePointer, leftOperand, rightOperand);
				}
				break;
			}
			case BITWISE_URSH: 
			{
				leftOperand = castToNative(leftOperand, leftType, _tessaTypeFactory->uintType()); 
				rightOperand = castToNative(rightOperand, rightType, _tessaTypeFactory->uintType()); 
				llvmBinaryInstruction = compileUnsignedRightShift(binaryOp, leftOperand, rightOperand);
				break;
			}
			case BITWISE_LSH:
			case BITWISE_RSH:
			case BITWISE_OR:
			case BITWISE_XOR:
			case BITWISE_AND:
			{
				leftOperand = castToNative(leftOperand, leftType, _tessaTypeFactory->integerType());
				rightOperand = castToNative(rightOperand, rightType, _tessaTypeFactory->integerType());
				llvmBinaryInstruction = compileBitwiseBinaryOp(binaryOp, leftOperand, rightOperand);
				break;
			}
			case SUBTRACT:
			case MULTIPLY:
			case DIVIDE:
			case MOD:
			{
				if (leftType->isInteger() && (rightType->isInteger()) && (resultType->isInteger())) { 
					llvmBinaryInstruction = compileIntegerBinaryOp(binaryOp, leftOperand, rightOperand);
				} else {
					leftOperand = castToNative(leftOperand, leftType, _tessaTypeFactory->numberType());
					rightOperand = castToNative(rightOperand, rightType, _tessaTypeFactory->numberType());
					llvmBinaryInstruction = compileDoubleBinaryOp(binaryOp, leftOperand, rightOperand);
					llvmBinaryInstruction = castToNative(llvmBinaryInstruction, _tessaTypeFactory->numberType(), binaryInstruction->getType());
				}
				break;
			}
			default:
			{
				TessaAssertMessage(false, "Unknown binary operand");
				break;
			}
		}

		return llvmBinaryInstruction;
	}

	llvm::Value* LlvmIRGenerator::compileAtomBinaryOperation(TessaInstructions::BinaryInstruction* binaryInstruction) {
		TessaValue* leftValue = binaryInstruction->getLeftOperand();
		TessaValue* rightValue = binaryInstruction->getRightOperand();

		TessaTypes::Type* leftType = leftValue->getType();
		TessaTypes::Type* rightType = rightValue->getType();

		llvm::Value* leftOperand = getValue(leftValue);
		llvm::Value* rightOperand = getValue(rightValue);
		leftOperand = nativeToAtom(leftOperand, leftType);
		rightOperand = nativeToAtom(rightOperand, rightType);

		TessaInstructions::TessaBinaryOp binaryOp = binaryInstruction->getOpcode();

		TessaAssert(!hasType(binaryInstruction->getLeftOperand()) || !hasType(binaryInstruction->getRightOperand()));
		llvm::Value* llvmBinaryInstruction;
		
		switch (binaryOp) {
			case ADD:
			{
				llvmBinaryInstruction = createLlvmCallInstruction(GET_LLVM_CALLINFO(op_add), 3, _avmcorePointer, leftOperand, rightOperand);
				llvmBinaryInstruction = atomToNative(llvmBinaryInstruction, binaryInstruction->getType());
				break;
			}
			case BITWISE_URSH: 
			{
				llvmBinaryInstruction = compileUnsignedRightShift(binaryOp, leftOperand, rightOperand);
				break;
		    }
			case BITWISE_LSH:
			case BITWISE_RSH:
			case BITWISE_OR:
			case BITWISE_XOR:
			case BITWISE_AND:
			{
				leftOperand = atomToNative(leftOperand, _tessaTypeFactory->integerType());
				rightOperand = atomToNative(rightOperand, _tessaTypeFactory->integerType()); 
				llvmBinaryInstruction = compileBitwiseBinaryOp(binaryOp, leftOperand, rightOperand);
				break;
			}
			case SUBTRACT:
			case MULTIPLY:
			case DIVIDE:
			case MOD:
			{
				leftOperand = atomToNative(leftOperand, _tessaTypeFactory->numberType());
				rightOperand = atomToNative(rightOperand, _tessaTypeFactory->numberType());
				llvmBinaryInstruction = compileDoubleBinaryOp(binaryOp, leftOperand, rightOperand);
				break;
			}
			default:
			{
				TessaAssertMessage(false, "Unknown binary operand");
				break;
			}
		}

		return llvmBinaryInstruction; 
	}

	void LlvmIRGenerator::visit(BinaryInstruction* binaryInstruction) {
		TessaValue* leftOp = binaryInstruction->getLeftOperand();
		TessaValue* rightOp = binaryInstruction->getRightOperand();
		llvm::Value* llvmBinaryInstruction;
		
		if (hasType(leftOp) && hasType(rightOp)) {
			llvmBinaryInstruction = compileTypedBinaryOperation(binaryInstruction);
		} else {
			llvmBinaryInstruction = compileAtomBinaryOperation(binaryInstruction);
		}

		TessaAssert(llvmBinaryInstruction != NULL);
		putValue(binaryInstruction, llvmBinaryInstruction);
	}

	/***
	 * Avm2 spec for IFNGT, IFNLE, IFNGE, IFNLT say must branch even if a condition uses NaN
	 **/
	bool LlvmIRGenerator::requiresNaNCheck(TessaBinaryOp opcode) {
		switch (opcode) {
			case NOT_GREATER_THAN:
			case NOT_GREATER_EQUAL_THAN:
			case NOT_LESS_THAN:
			case NOT_LESS_EQUAL_THAN:
				return true;
			default:
				return false;
		}
	}

	llvm::Instruction* LlvmIRGenerator::checkForNaNLlvm(llvm::Value* compareResult , llvm::Value* leftValue, llvm::Value* rightValue) {
		AvmAssert(leftValue->getType()->isDoubleTy());
		AvmAssert(rightValue->getType()->isDoubleTy());
		llvm::Value* isNaN = new llvm::FCmpInst(*llvmBasicBlock, llvm::CmpInst::FCMP_UNO, leftValue, rightValue);

		/***
		 * We got here because we were comparing a floating point value versus 0.
		 * If the value is NaN, we need to return true.
		 * If the value is NOT NaN, return the original comparison against 0
		 */
		return llvm::SelectInst::Create(isNaN, createConstantBoolean(true), compareResult, "NaN Select", llvmBasicBlock);
	}

	llvm::Instruction* LlvmIRGenerator::checkForNaNAtomLlvm(TessaBinaryOp conditionOpcode, llvm::Value* compareResult) {
		AvmAssert(isAtomType(compareResult));

		// NaN is defined as the undefined atom when boxed
		llvm::Value* isNaN = new llvm::ICmpInst(*llvmBasicBlock, llvm::CmpInst::ICMP_EQ, compareResult, _undefinedAtom);
		llvm::Value* returnAtom = _falseAtom;
		if (requiresNaNCheck(conditionOpcode)) {
			returnAtom = _trueAtom;
		} 

		return llvm::SelectInst::Create(isNaN, returnAtom, compareResult, "Atom NaN select", llvmBasicBlock);
	}

	llvm::Value* LlvmIRGenerator::compileTypedCondition(TessaInstructions::ConditionInstruction* conditionInstruction) {
		TessaValue* leftOperand = conditionInstruction->getLeftOperand();
		TessaValue* rightOperand = conditionInstruction->getRightOperand();
		TessaAssert(hasType(leftOperand) && hasType(rightOperand));
		llvm::Instruction* result;

		TessaTypes::Type* leftType = leftOperand->getType();
		TessaTypes::Type* rightType = rightOperand->getType();

		llvm::Value* leftValue = getValue(conditionInstruction->getLeftOperand());
		llvm::Value* rightValue = getValue(conditionInstruction->getRightOperand());
		TessaBinaryOp conditionOpcode = conditionInstruction->getOpcode();

		bool useInt = true;
		bool useBoolean = false;
		if ((isInteger(leftOperand) && isInteger(rightOperand)) ||
			(leftOperand->isUnsignedInteger() && rightOperand->isUnsignedInteger())) {
			useInt = true;
		} else if (isInteger(leftOperand) && rightOperand->isNumber()) {
			useInt = false;
			leftValue = castToNative(leftValue, _tessaTypeFactory->integerType(), _tessaTypeFactory->numberType());
		} else if (isInteger(rightOperand) && leftOperand->isNumber()) {
			useInt = false;
			rightValue = castToNative(rightValue, _tessaTypeFactory->integerType(), _tessaTypeFactory->numberType());
		} else if (leftOperand->isBoolean() || rightOperand->isBoolean()) {
			useInt = true;
			useBoolean = true;
			leftValue = castToNative(leftValue, leftType, _tessaTypeFactory->boolType());
			rightValue = castToNative(rightValue, rightType, _tessaTypeFactory->boolType());
		} else if ((leftOperand->isUnsignedInteger() && !rightOperand->isUnsignedInteger()) ||
				(rightOperand->isUnsignedInteger() && !leftOperand->isUnsignedInteger())) {
			AvmAssert(leftOperand->isNumeric());
			AvmAssert(rightOperand->isNumeric());
			useInt = false;
			leftValue = castToNative(leftValue, leftType, _tessaTypeFactory->numberType());
			rightValue = castToNative(rightValue, rightType, _tessaTypeFactory->numberType());
		} else {
			TessaAssert(leftOperand->isNumber() && rightOperand->isNumber());
			useInt = false;
		}

		bool useFloat = !useInt;
		llvm::ICmpInst::Predicate integerOpcode;
		llvm::FCmpInst::Predicate floatOpcode;

		/****
		 * Core->compare() returns true if less than.
		 */
		switch (conditionOpcode) {
			case NOT_EQUAL:
			{
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_NE;					
				} else {
					floatOpcode = CmpInst::FCMP_ONE;
				}
				break;
			}
			case EQUAL: 
			{
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_EQ;
				} else {
					floatOpcode = CmpInst::FCMP_OEQ;
				}
				break;
			}
			case IFFALSE:
			case IFTRUE:
			{
				TessaAssert(useBoolean);
				TessaAssert(isBoolType(leftValue));
				integerOpcode = ICmpInst::ICMP_EQ;
				break;
			}

			case LESS_THAN:
			case NOT_GREATER_EQUAL_THAN:
			{
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_SLT;
				} else {
					floatOpcode = CmpInst::FCMP_OLT;
				}
				break;
			}
			
			case GREATER_EQUAL_THAN:
			case NOT_LESS_THAN:
			{
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_SGE;
				} else {
					floatOpcode = CmpInst::FCMP_OGE;
				}
				break;
			}

			// !(a > b) === !(b<a)
			case NOT_GREATER_THAN:
			case LESS_EQUAL_THAN: 
			{
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_SLE;
				} else {
					floatOpcode = CmpInst::FCMP_OLE;
				}
				break;
			}
   
			// a > b === b < a
			case NOT_LESS_EQUAL_THAN:
			case GREATER_THAN:
			{
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_SGT;
				} else {
					floatOpcode = CmpInst::FCMP_OGT;
				}
				break;
			}

			case STRICT_NOT_EQUAL:
			{
				TessaTypes::Type* leftType = leftOperand->getType();
				TessaTypes::Type* rightType = rightOperand->getType();
				if (leftType != rightType) {
					return (llvm::Instruction*) createConstantBoolean(false);
				}
				if (useInt) {
					integerOpcode = ICmpInst::ICMP_NE;					
				} else {
					floatOpcode = CmpInst::FCMP_ONE;
				}
				break;
			}
			case STRICT_EQUAL:
			{
				TessaTypes::Type* leftType = leftOperand->getType();
				TessaTypes::Type* rightType = rightOperand->getType();
				if (leftType != rightType) {
					return (llvm::Instruction*) createConstantBoolean(false);
				}

				if (useInt) {
					integerOpcode = ICmpInst::ICMP_EQ;
				} else {
					floatOpcode = CmpInst::FCMP_OEQ;
				}
				break;
			}
			default:
			{
				TessaAssert(false);
				break;
			}
		}

		if (useInt) {
			result = new llvm::ICmpInst(*llvmBasicBlock, integerOpcode, leftValue, rightValue);
		} else {
			result = new llvm::FCmpInst(*llvmBasicBlock, floatOpcode, leftValue, rightValue);
			if (requiresNaNCheck(conditionOpcode)) {
				result = checkForNaNLlvm(result, leftValue, rightValue);
			}
		}

		TessaAssert(result != NULL);
		return result;
	}

	llvm::Value* LlvmIRGenerator::compileAtomCondition(TessaInstructions::ConditionInstruction* conditionInstruction) {
		TessaValue* tessaLeftOperand = conditionInstruction->getLeftOperand();
		TessaValue* tessaRightOperand = conditionInstruction->getRightOperand();
		llvm::Value* leftValue = nativeToAtom(getValue(tessaLeftOperand), tessaLeftOperand->getType());
		llvm::Value* rightValue = nativeToAtom(getValue(tessaRightOperand), tessaRightOperand->getType());
		TessaBinaryOp conditionOpcode = conditionInstruction->getOpcode();
		llvm::Value* compareResult;
		llvm::Value* result;

		/****
		 * Core->compare() returns true if less than.
		 */
		switch (conditionOpcode) {
			case NOT_EQUAL:
			case EQUAL: 
			case IFFALSE:
			case IFTRUE:
			{
				result = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreEquals), 3, _avmcorePointer, leftValue, rightValue);
				break;
			}
			
			case NOT_GREATER_EQUAL_THAN:
			case LESS_THAN:
			case GREATER_EQUAL_THAN:
			case NOT_LESS_THAN:
			{
				result = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreCompare), 3, _avmcorePointer, leftValue, rightValue);
				break;
			}

			// !(a > b) === !(b<a)
			case NOT_GREATER_THAN:
			case LESS_EQUAL_THAN: 
   
			// a > b === b < a
			case NOT_LESS_EQUAL_THAN:
			case GREATER_THAN:
			{
				result = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreCompare), 3, _avmcorePointer, rightValue, leftValue);
				break;
			}

			case STRICT_NOT_EQUAL:
			case STRICT_EQUAL:
			{
				result = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreStrictEquals), 3, _avmcorePointer, leftValue, rightValue);
				break;
			}
			default:
			{
				TessaAssert(false);
				break;
			}
		}

		compareResult = result;
		
		switch (conditionOpcode) {
			case NOT_EQUAL:
			case GREATER_EQUAL_THAN:
			case NOT_LESS_THAN:
			case STRICT_NOT_EQUAL:
			{
				/* inverse the boolean result
				 * compare          ^8    <8
				 * true       1101  0101   y
				 * false      0101  1101   n
				 * undefined  0100  1100   n
				 */
				result = llvm::BinaryOperator::Create(Instruction::Xor, result, createConstantInt(8), "ConditionFlipResult", llvmBasicBlock);
				break;
			}
			
			case NOT_GREATER_THAN:
			case LESS_EQUAL_THAN: 
			{
				// Test for (a<=b) and jf for !(a<=b)
		        // compare          ^1    <=4
		        // true       1101  1100  n
		        // false      0101  0100  y
		        // undefined  0100  0101  n
				llvm::Value* xorValue = llvm::BinaryOperator::Create(Instruction::Xor, result, createConstantInt(1), "LEQXor", llvmBasicBlock);
				llvm::Value* lessThan = new ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_SLE, xorValue, createConstantInt(4), "LEQXorLEQ4");
				result = llvm::SelectInst::Create(lessThan, _trueAtom, _falseAtom, "LEQ Select", llvmBasicBlock);
				break;
			}

			case NOT_LESS_EQUAL_THAN:
			case GREATER_THAN:
			{
				// Test for (a<=b) and jf for !(a<=b)
		        // compare          ^1    <=4
		        // true       1101  1100  n
		        // false      0101  0100  y
		        // undefined  0100  0101  n
				llvm::Value* xorValue = llvm::BinaryOperator::Create(Instruction::Xor, result, createConstantInt(1), "LEQXor", llvmBasicBlock);
				llvm::Value* lessThan = new ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_SLE, xorValue, createConstantInt(4), "LEQXorLEQ4");
				result = llvm::SelectInst::Create(lessThan, _falseAtom, _trueAtom, "LEQ Select", llvmBasicBlock);
				break;
			}
			default:
				break;
		}

		return (llvm::Instruction*) atomToNative(result, _tessaTypeFactory->boolType());
	}

	llvm::Value* LlvmIRGenerator::compileConditionNullCheck(TessaBinaryOp opcode, TessaValue* leftOperand, TessaValue* rightOperand) {
		llvm::Value* leftValue = getValue(leftOperand);
		llvm::Value* rightValue = getValue(rightOperand);
		leftValue = castToNative(leftValue, leftOperand->getType(), _tessaTypeFactory->integerType());
		rightValue = castToNative(rightValue, rightOperand->getType(), _tessaTypeFactory->integerType());
		llvm::ICmpInst::Predicate integerOpcode;

		switch (opcode) {
			case EQUAL:
				integerOpcode = ICmpInst::ICMP_EQ;
				break;
			case NOT_EQUAL:
				integerOpcode = ICmpInst::ICMP_NE;
				break;
			default:
				TessaAssert(false);
		}

		return new llvm::ICmpInst(*llvmBasicBlock, integerOpcode, leftValue, rightValue);
	}

	llvm::Value* LlvmIRGenerator::compareAbsolutePointerAddresses(ConditionInstruction* conditionInstruction, llvm::Value* leftOperand, llvm::Value* rightOperand)  {
		llvm::Value* result = NULL;

		switch (conditionInstruction->getOpcode()) {
		case EQUAL:
			result = new llvm::ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_EQ, leftOperand, rightOperand);
			break;
		case NOT_EQUAL:
			//AvmAssert(false);
			result = new llvm::ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_NE, leftOperand, rightOperand);
			break;
		default:
			AvmAssert(false);
		}	
		
		AvmAssert(result != NULL);
		return result;
	}

	llvm::Value* LlvmIRGenerator::compilePointerCompare(ConditionInstruction* conditionInstruction) {
		TessaValue* leftTessaValue = conditionInstruction->getLeftOperand();
		TessaValue* rightTessaValue = conditionInstruction->getRightOperand();

		llvm::Value* leftOperand = getValue(leftTessaValue);
		llvm::Value* rightOperand = getValue(rightTessaValue);
		TessaAssert(leftOperand->getType()->isPointerTy());
		TessaAssert(rightOperand->getType()->isPointerTy());
		llvm::Value* result;

		if (leftTessaValue->getType()->isString() && rightTessaValue->getType()->isString()) {
			// String compares have to be done via atoms :(
			result = compileAtomCondition(conditionInstruction);
		} else {
			result = compareAbsolutePointerAddresses(conditionInstruction, leftOperand, rightOperand);
		}
	
		AvmAssert(result != NULL);
		return result;
	}

	void LlvmIRGenerator::visit(ConditionInstruction* conditionInstruction) {
		TessaValue* leftOperand = conditionInstruction->getLeftOperand();
		TessaValue* rightOperand = conditionInstruction->getRightOperand();
		llvm::Value* result;

		if (conditionInstruction->forceOptimizeNullPointerCheck) {
			result = compileConditionNullCheck(conditionInstruction->getOpcode(), leftOperand, rightOperand);
		} else if ( (leftOperand->isNumeric() || leftOperand->isBoolean()) && (rightOperand->isNumeric() || rightOperand->isBoolean()) ) {
			result = compileTypedCondition(conditionInstruction);
		} else if (leftOperand->isPointer() && rightOperand->isPointer()) {
			result = compilePointerCompare(conditionInstruction);
		} else {
			result = compileAtomCondition(conditionInstruction);
		}

		/*
		if (conditionInstruction->forceOptimizeNullPointerCheck) {
			result = compileConditionNullCheck(conditionInstruction->getOpcode(), leftOperand, rightOperand);
		} else if ((leftOperand->isNumber() || isInteger(leftOperand) || leftOperand->isBoolean()) &&
			(rightOperand->isNumber() || isInteger(rightOperand) || rightOperand->isBoolean())) {
			result = compileTypedCondition(conditionInstruction);
		} else if (leftOperand->isPointer() && rightOperand->isPointer()) {
			result = compilePointerCompare(conditionInstruction);
		} else {
			result = compileAtomCondition(conditionInstruction);
		}
		*/

		TessaAssert(result != NULL);
		putValue(conditionInstruction, result);
	}

	llvm::Value* LlvmIRGenerator::compileTypedUnaryOperation(TessaInstructions::UnaryInstruction* unaryInstruction) {
		llvm::Value* result;
		llvm::Value* operand = getValue(unaryInstruction->getOperand());
		TessaTypes::Type* operandType = unaryInstruction->getOperand()->getType();

		switch (unaryInstruction->getOpcode()) {
			case BITWISE_NOT:
			{
				operand = castToNative(operand, operandType, _tessaTypeFactory->integerType());
				result = llvm::BinaryOperator::Create(Instruction::Xor, createConstantInt(0xFFFFFFFF), operand, "UnaryBitwiseNot", llvmBasicBlock);
				break;
			}
			case NEGATE:
			{
				operand = castToNative(operand, operandType, _tessaTypeFactory->numberType());
				result = llvm::BinaryOperator::Create(Instruction::FSub, createConstantDouble(0), operand, "UnaryNegate", llvmBasicBlock);
				break;
			}
			case NOT:
			{
				operand = castToNative(operand, operandType, _tessaTypeFactory->boolType());
				result = llvm::SelectInst::Create(operand, createConstantBoolean(false), createConstantBoolean(true), "", llvmBasicBlock);
				break;
			}
			default:
				TessaAssert(false);
		}

		return (llvm::Instruction*) result;
	}

	llvm::Value* LlvmIRGenerator::compileAtomUnaryOperation(TessaInstructions::UnaryInstruction* unaryInstruction) {
		TessaInstruction* tessaOperand = unaryInstruction->getOperand();
		llvm::Value* operand = nativeToAtom(getValue(unaryInstruction->getOperand()), tessaOperand->getType());
		llvm::Value* result;

		switch (unaryInstruction->getOpcode()) {
			case BITWISE_NOT:
			{
				operand = atomToNative(operand, _tessaTypeFactory->integerType());
				// Implement C ~ operator as (value ^ 0xFFFFFFFF)
				result = llvm::BinaryOperator::Create(Instruction::Xor, createConstantInt(0xFFFFFFFF), operand, "UnaryBitwiseNot", llvmBasicBlock);
				break;
			}
			case NEGATE:
			{
				operand = atomToNative(operand, _tessaTypeFactory->numberType());
				result = llvm::BinaryOperator::Create(Instruction::Sub, createConstantDouble(0), operand, "UnaryNegate", llvmBasicBlock);
				break;
			}
			case NOT:
			{
				result = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreBooleanAtom), 1, operand);
				llvm::Value* xorTrueFalse = llvm::BinaryOperator::Create(Instruction::Xor, _trueAtom, _falseAtom, "", llvmBasicBlock);
				result = llvm::BinaryOperator::Create(Instruction::Xor, result, xorTrueFalse, "UnaryNot", llvmBasicBlock);
				result = atomToNative(result, _tessaTypeFactory->boolType());
				break;
			}
			default:
				TessaAssert(false);
		}

		TessaAssert(result != NULL);
		return (llvm::Instruction*) result;
	}

	void LlvmIRGenerator::visit(TessaInstructions::UnaryInstruction* unaryInstruction) {
		TessaInstruction* tessaOperand = unaryInstruction->getOperand();
		TessaTypes::Type* operandType = tessaOperand->getType();
		llvm::Value* result;

		if (!operandType->isAnyType()) { 
			result = compileTypedUnaryOperation(unaryInstruction);
		} else {
			result = compileAtomUnaryOperation(unaryInstruction);
		}

		TessaAssert(result != NULL);
		putValue(unaryInstruction, result);
	}

	/***
	 * Allocates space on the stack and puts the instructions in the array of instructions 
	 * in a contigous memory space
	 */
	void LlvmIRGenerator::visit(ArrayOfInstructions* arrayOfInstructions) {
	}

	void LlvmIRGenerator::visit(NewObjectInstruction* newObjectInstruction) {
		llvm::Value* objectPropertyPairs = createAtomArgumentsPointer(newObjectInstruction->getObjectProperties());
		int numberOfProperties = newObjectInstruction->getNumberOfProperties(); 

		llvm::Value* newObjectScriptObject = createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvNewObject), 3, _envPointer, objectPropertyPairs, createConstantInt(numberOfProperties)); 
		putValue(newObjectInstruction, newObjectScriptObject);
	}

	void LlvmIRGenerator::visit(ConstructInstruction* constructInstruction) {
		TessaInstruction* receiverObject = constructInstruction->getReceiverObject();
		llvm::Value* llvmReceiverObject = getValue(receiverObject);
		llvm::Value* constructedObject;

		if (constructInstruction->isEarlyBound()) {
			TessaAssert(receiverObject->isScriptObject());
			constructedObject = callEarlyBoundConstruct(constructInstruction);
			constructedObject = ScriptObjectToAtom(constructedObject);
		} else {
			if (receiverObject->isScriptObject()) {
				llvmReceiverObject = ScriptObjectToAtom(llvmReceiverObject);
			} else {
				TessaAssert(receiverObject->isAny());	
			}

			llvm::Value* arguments = createAtomArgumentsPointer(constructInstruction->getArguments());
			int argCount = constructInstruction->getNumberOfArgs();
			constructedObject = createLlvmCallInstruction(GET_LLVM_CALLINFO(op_construct), 4, _toplevelPointer, llvmReceiverObject, createConstantInt(argCount), arguments); 
		}

		// Constructed objects have to be atoms
		TessaAssert(constructedObject->getType()->isIntegerTy());
		putValue(constructInstruction, constructedObject);
	}

	llvm::Value* LlvmIRGenerator::callLateBoundConstructProperty(ConstructPropertyInstruction* constructPropertyInstruction) {
		TessaAssert(!constructPropertyInstruction->isEarlyBound());
		const Multiname* propertyMultiname = constructPropertyInstruction->getPropertyMultiname();
		llvm::Value* propertyMultinamePointer = this->createConstantPtr((intptr_t)(propertyMultiname));
		ArrayOfInstructions* callArguments = constructPropertyInstruction->getArguments();
		int argCount = callArguments->size();
		TessaAssert(argCount >= 1);

		if (propertyMultiname->isRuntime()) {
			TessaAssertMessage(false, "Don't know how to handle runtime multinames");
		}

		llvm::Value* receiverObject = getValue(callArguments->getInstruction(0));
		llvm::Value* llvmArguments = createAtomArgumentsPointer(callArguments);

		// constructprop expects the arg count to NOT include the "this" pointer
		llvm::Value* constructedProperty = createLlvmCallInstruction(GET_LLVM_CALLINFO(constructprop), 4, _envPointer, propertyMultinamePointer, createConstantInt(argCount - 1), llvmArguments);
		constructedProperty = atomToNative(constructedProperty, constructPropertyInstruction->getType());
		TessaAssert(constructedProperty != NULL);
		return constructedProperty;
	}

	/***
	 * Do this
	 *  ScriptObject* ctor = AvmCore::atomToScriptObject(obj)->getSlotObject(AvmCore::bindingToSlotId(b));
	 */ 
	llvm::Value* LlvmIRGenerator::loadFromSlot(llvm::Value* receiverObject, TessaTypes::Type* tessaType, int slotNumber, Traits* objectTraits) {
		const TraitsBindingsp traitsBinding = objectTraits->getTraitsBindings();
		int offset = traitsBinding->getSlotOffset(slotNumber);

		TessaAssert(receiverObject->getType()->isPointerTy());
		TessaAssert(tessaType == _tessaTypeFactory->scriptObjectType());

		// Load from receiverObject + address
		llvm::Value* receiverObjectInt = castPtrToInt(receiverObject);
		llvm::Value* receiverObjectAddress = castIntToPtr(llvm::BinaryOperator::Create(llvm::BinaryOperator::Add, receiverObjectInt, createConstantInt(offset), "", llvmBasicBlock));
		return createLlvmLoadPtr(receiverObjectAddress, 0);
	}

	/***
	 * Do this:
	 *
	 * this == ClassClosure
	 * Vtable vtable = this->vtable();
	 * VTable* ivtable = vtable->ivtable
     *  ScriptObject* obj = newInstance();
	 * return obj
	 */
	llvm::Value* LlvmIRGenerator::emitCallToNewInstance(llvm::Value* receiverObject, llvm::Value* instanceVtable) {
		TessaAssert(receiverObject->getType()->isPointerTy());
		llvm::Value* createdInstanceAddress = createLlvmLoadPtr(instanceVtable, offsetof(VTable, createInstance) / sizeof(int32_t));

		llvm::Function* vtableCreateInstanceFunction = getLlvmFunction(GET_LLVM_CALLINFO(VTableCreateInstance));
		const llvm::FunctionType* functionType = vtableCreateInstanceFunction->getFunctionType();
		llvm::Value* newInstanceAsScriptObject = createLlvmIndirectCallInstruction(createdInstanceAddress, functionType, "NewInstance", 2, receiverObject, instanceVtable);
		return createBitCast(newInstanceAsScriptObject, _pointerType);
	}

	// Atom ivtable->init->coerceEnter(argc, argv);
	llvm::Value* LlvmIRGenerator::emitCallToMethodInfoImplgr(llvm::Value* instanceVtable, int argCount, llvm::Value* llvmArguments) {
		llvm::Value* loadedMethodInfo = createLlvmLoadPtr(instanceVtable, offsetof(VTable, init) / sizeof(int32_t));
		llvm::Value* methodToCall = createLlvmLoadPtr(loadedMethodInfo, offsetof(MethodEnvProcHolder, _implGPR) / sizeof(int32_t));
		llvm::Function* invokeFunction = getLlvmFunction(GET_LLVM_CALLINFO(InvokeReturnInt));
		const llvm::FunctionType* invokeFunctionType = invokeFunction->getFunctionType();
		return createLlvmIndirectCallInstruction(methodToCall, invokeFunctionType, "ConstructEarly", 3, loadedMethodInfo, createConstantInt(argCount), llvmArguments);
	}

	llvm::Value* LlvmIRGenerator::callEarlyBoundConstruct(ConstructInstruction* constructInstruction) {
		TessaAssert(constructInstruction->isEarlyBound());
		ArrayOfInstructions* callArguments = constructInstruction->getArguments();
		int argCount = callArguments->size();
		int argCountAdjustment = argCount - 1;	// Construct prop assumes arg count - 1 as the argc parameter
		TessaAssert(argCount >= 1);

		Traits* classTraits = constructInstruction->getResultTraits();
		Traits* instanceTraits = classTraits->itraits;
        MethodInfo* constructorMethodInfo = instanceTraits->init;
        MethodSignaturep constructorMethodSignature = constructorMethodInfo->getMethodSignature();
        AvmAssert(constructorMethodSignature->argcOk(argCount - 1)); // caller must check this before early binding to constructor

		TessaInstruction* tessaReceiver = callArguments->getInstruction(0);
		llvm::Value* receiverObject = getValue(tessaReceiver);
		llvm::Value* llvmArguments = createTypedArgumentsPointer(callArguments, constructorMethodSignature, argCountAdjustment);
		int slotIndex = constructInstruction->getSlotId();
		Traits* objectTraits = constructInstruction->getObjectTraits();

		llvm::Value* receiverObjectLoaded;
		if (constructInstruction->isConstructProperty()) {
			receiverObjectLoaded = loadFromSlot(receiverObject, tessaReceiver->getType(), slotIndex, objectTraits);
		} else {
			TessaAssert(tessaReceiver->isScriptObject());
			receiverObjectLoaded = receiverObject;
		}

		llvm::Value* vtable = loadVTable(receiverObjectLoaded, _tessaTypeFactory->scriptObjectType());
		llvm::Value* instanceVtable = createLlvmLoadPtr(vtable, offsetof(VTable, ivtable) / sizeof(int32_t));
		llvm::Value* createdObject = emitCallToNewInstance(receiverObjectLoaded, instanceVtable);

        // arguments[0] = newInstance; // new object is receiver
		TessaAssert(createdObject->getType()->isPointerTy());
		TessaAssert(tessaReceiver->isScriptObject());
		TessaAssert(getTessaType(constructorMethodSignature->paramTraits(0)) == _tessaTypeFactory->scriptObjectType());

		createLlvmStore(llvmArguments, createdObject, 0);
		emitCallToMethodInfoImplgr(instanceVtable, argCountAdjustment, llvmArguments);
		return createdObject;
	}

	void LlvmIRGenerator::visit(ConstructPropertyInstruction* constructPropertyInstruction) {
		llvm::Value* constructedProp;
		if (constructPropertyInstruction->isEarlyBound()) {
			constructedProp = callEarlyBoundConstruct(constructPropertyInstruction);
		} else {
			constructedProp = callLateBoundConstructProperty(constructPropertyInstruction);
		}

		TessaAssert(constructedProp != NULL);
		putValue(constructPropertyInstruction, constructedProp);
	}

	void LlvmIRGenerator::visit(ConstructSuperInstruction* constructSuperInstruction) {
		MethodInfo* methodInfo = constructSuperInstruction->getMethodInfo();
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		ArrayOfInstructions* arguments = constructSuperInstruction->getArguments();

		uint32_t argCount = constructSuperInstruction->getNumberOfArgs();
		llvm::Value* llvmArguments = createTypedArgumentsPointer(arguments, methodSignature, argCount);

		// Do vtable()->base->init->coerceenter;
		llvm::Value* vtable = _methodEnvVTable;
		llvm::Value* base = createLlvmLoadPtr(vtable, offsetof(VTable, base) / sizeof(int32_t)); 
		llvm::Value* methodEnv = createLlvmLoadPtr(base, offsetof(VTable, init) / sizeof(int32_t));
		llvm::Value* methodAddress = createLlvmLoadPtr(methodEnv, (offsetof(MethodEnvProcHolder,_implGPR) / sizeof(int32_t))); 

		const llvm::FunctionType* functionType = getInvokeFunctionType(_tessaTypeFactory->voidType()); 
		createLlvmIndirectCallInstruction(methodAddress, functionType, "construct super early", 3, methodEnv, createConstantInt(argCount), llvmArguments);
	}

	std::vector<llvm::Value*>* LlvmIRGenerator::createLlvmArguments(ArrayOfInstructions* arguments) {
		std::vector<llvm::Value*>* argumentVector = new std::vector<llvm::Value*>();
		for (uint32_t i = 0; i < arguments->size(); i++) {
			TessaInstruction* tessaInstruction = arguments->getInstruction(i);
			llvm::Value* llvmValue = getValue(tessaInstruction);
			argumentVector->push_back(llvmValue);
		}
		return argumentVector;
	}

	/***
	 * We assume that "object" is in its native representation. Ala an integer 10.
	 * We have to convert it to the appropriate Atom. Ugh
	 */ 
	llvm::Value* LlvmIRGenerator::createBitCast(llvm::Value* valueToCast, const llvm::Type* typeToCastTo) {
		if (valueToCast->getType() == typeToCastTo) return valueToCast;
		return new llvm::BitCastInst(valueToCast, typeToCastTo, "", llvmBasicBlock);
	}

	/***
	 * Given an atom, return the value in unboxed form.
	 */
	llvm::Value * LlvmIRGenerator::atomToNative(llvm::Value* atom, TessaTypes::Type* typeToNative) {
		AvmAssert(isAtomType(atom));
		if (typeToNative->isString()) {
			// Have to check for string type prior to casting to pointer type
			return createBitCast(createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreCoerce_s), 2, _avmcorePointer, atom), _pointerType);
		} else if (isLlvmPointerType(typeToNative)) {
			return AtomToScriptObject(atom);
		}

		if (typeToNative->isAnyType() || typeToNative->isObject()) {
				return atom;
		} else if (typeToNative->isBoolean()) {
			llvm::Value* atomBoolean = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreBooleanAtom), 1, atom);
			return new ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_EQ, atomBoolean, _trueAtom, "");
		} else if (typeToNative->isUnsignedInt()) {
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreToUint32), 1, atom);
		} else if (typeToNative->isInteger()) {
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreInteger), 1, atom);
		} else if (typeToNative->isNumber()) {
				return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreNumber), 1, atom);
		} else if (typeToNative->isString()) {
			AvmAssert(false);
			//return createBitCast(createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreCoerce_s), 2, _avmcorePointer, atom), _pointerType);
		} else {
			AvmAssert(false);
			return atom;
		}
	}

	/***
	 * Big big wins specializing float -> integer
	 * int intval = __asm__( cvttsd2si doubleValue );
	 * if (intval != 0x80000000)
     *       return intval;
	 * else {
	 *		call integer_d
	 * }
	 */
	llvm::Value* LlvmIRGenerator::specializeCastFloatToInteger(llvm::Value* doubleValue) {
		llvm::BasicBlock* castingErrorBlock = llvm::BasicBlock::Create(*context, "castFloatToIntErrorBlock", _llvmFunction);
		llvm::BasicBlock* mergeBlock= llvm::BasicBlock::Create(*context, "castFloatToIntMerge", _llvmFunction);

		llvm::BasicBlock* currentBlock = llvmBasicBlock;

		/***
		 * if castedValue != 0x80000000
		 */
		llvm::Value* errorValue = createConstantInt(0x80000000);
		llvm::Value* castedValue = new llvm::FPToSIInst(doubleValue, _intType, "", currentBlock);
		llvm::Value* errorCondition = new llvm::ICmpInst(*currentBlock, ICmpInst::ICMP_NE, castedValue, errorValue, "");
		llvm::Instruction* branchErrorInstruction = llvm::BranchInst::Create(mergeBlock, castingErrorBlock, errorCondition, currentBlock);

		/***
		 * Work on error block
		 */
		llvmBasicBlock = castingErrorBlock;
		llvm::Value* longCastedValue = createLlvmCallInstruction(GET_LLVM_CALLINFO(NumberToInteger), 1, doubleValue);
		llvm::Instruction* branchFromErrorToMerge = llvm::BranchInst::Create(mergeBlock, castingErrorBlock);

		llvmBasicBlock = mergeBlock;
		llvm::PHINode* correctIntegerValue = llvm::PHINode::Create(_intType, "", mergeBlock);
		correctIntegerValue->addIncoming(castedValue, currentBlock);
		correctIntegerValue->addIncoming(longCastedValue, castingErrorBlock);
		return correctIntegerValue;
	}

	llvm::Value* LlvmIRGenerator::castFloatToNative(llvm::Value* doubleValue, TessaTypes::Type* newNativeType) {
		TessaAssert(doubleValue->getType() == _doubleType);
		if (newNativeType->isBoolean()) {
			return new llvm::FCmpInst(*llvmBasicBlock, llvm::FCmpInst::FCMP_ONE, doubleValue, createConstantDouble(0));
		} else if (newNativeType->isString()) {
			llvm::Value* doubleAtom = nativeToAtom(doubleValue, _tessaTypeFactory->numberType()); 
			return atomToNative(doubleAtom, _tessaTypeFactory->stringType());
		} else if (newNativeType->isObject()) {
			return nativeToAtom(doubleValue, _tessaTypeFactory->numberType()); 
		} else if (newNativeType->isUnsignedInt()) {
			return new llvm::FPToUIInst(doubleValue, _uintType, "", llvmBasicBlock);
		} else if (newNativeType->isInteger()) {
			return specializeCastFloatToInteger(doubleValue);
		} else if (newNativeType->isNumber()) {
			return doubleValue;
		} else {
			AvmAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	llvm::Value* LlvmIRGenerator::castBooleanToNative(llvm::Value* booleanValue, TessaTypes::Type* newNativeType) {
		TessaAssert(booleanValue->getType() == _booleanType);
		if (newNativeType->isBoolean()) {
			return booleanValue;
		} else if (newNativeType->isString()) {
			llvm::Value* booleanAtom = llvm::SelectInst::Create(booleanValue, _trueAtom, _falseAtom, "", llvmBasicBlock);
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreString), 2, _avmcorePointer, booleanAtom);
		} else if (newNativeType->isObject()) {
			return nativeToAtom(booleanValue, _tessaTypeFactory->boolType());
		} else if (newNativeType->isUnsignedInt()) {
			llvm::Value* trueInt = createConstantUnsignedInt(1);
			llvm::Value* falseInt = createConstantUnsignedInt(0);
			return llvm::SelectInst::Create(booleanValue, trueInt, falseInt, "", llvmBasicBlock);
		} else if (newNativeType->isInteger()) {
			llvm::Value* trueInt = createConstantInt(1);
			llvm::Value* falseInt = createConstantInt(0);
			return llvm::SelectInst::Create(booleanValue, trueInt, falseInt, "", llvmBasicBlock);
		} else if (newNativeType->isNumber()) {
			llvm::Value* trueInt = createConstantDouble(1);
			llvm::Value* falseInt = createConstantDouble(0);
			return llvm::SelectInst::Create(booleanValue, trueInt, falseInt, "", llvmBasicBlock);
		} else {
			AvmAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	llvm::Value* LlvmIRGenerator::castIntegerToNative(llvm::Value* integerValue, TessaTypes::Type* newNativeType) {
		TessaAssert(integerValue->getType()->isIntegerTy());
		if (newNativeType->isNumber()) {
			return new llvm::SIToFPInst(integerValue, _doubleType, "", llvmBasicBlock);
		} else if (newNativeType->isUnsignedInt()) {
			 return new llvm::BitCastInst(integerValue, _uintType, "", llvmBasicBlock);
		} else if (newNativeType->isBoolean()) {
			return new llvm::ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_NE, integerValue, createConstantInt(0), "");
		} else if (newNativeType->isObject() || newNativeType->isVoid()) {
			// We probably have an undefined atom with VOID type
			return integerValue;
		} else {
			printf("Changing integer to type: %s\n", newNativeType->toString().data());
			TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	llvm::Value* LlvmIRGenerator::castScriptObjectToNative(llvm::Value* scriptObject, TessaTypes::Type* newNativeType) {
		if (newNativeType->isPointer()) {
			return scriptObject;
		} else if (newNativeType->isInteger()) {
			return castPtrToInt(scriptObject);
		} else if (newNativeType->isObject()) {
			return ScriptObjectToAtom(scriptObject);
		} else if (newNativeType->isBoolean()) {
			llvm::Value* boxedObject = ScriptObjectToAtom(scriptObject);
			return new llvm::ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_NE, boxedObject, _nullObjectAtom, "");
		} else if (newNativeType->isNumber()) {
			llvm::Value* atomValue = ScriptObjectToAtom(scriptObject);
			return atomToNative(atomValue, newNativeType);
		} else if (newNativeType->isNull()) {
			return _nullObjectAtom;
		} else {
			printf("native type is: %s\n", newNativeType->toString().data());
			TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	llvm::Value* LlvmIRGenerator::castUndefinedToNative(llvm::Value* undefinedValue, TessaTypes::Type* newNativeType) {
		if (isLlvmPointerType(newNativeType)) {
			return AtomToScriptObject(undefinedValue);
		}

		if (newNativeType->isNumber()) {
			return createConstantDouble(0);
		} else if (newNativeType->isInteger()) {
			return createConstantInt(0);
		} else if (newNativeType->isObject()) {
				return _nullObjectAtom;
		} else if (newNativeType->isVoid()) {
				return undefinedValue;
		} else if (newNativeType->isBoolean()) {
			return createConstantBoolean(false);
		} else {
			printf("Going from undefined or null to: %s\n", newNativeType->toString().data());
			TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	llvm::Value* LlvmIRGenerator::castAvmObjectToNative(llvm::Value* avmObject, TessaTypes::Type* newNativeType) {
		TessaAssert(avmObject->getType()->isIntegerTy());

		if (newNativeType->isPointer()) {
			return AtomToScriptObject(avmObject);
		} else if (newNativeType->isNumber()) {
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreAtomToNumberFast), 1, avmObject);
		} else if (newNativeType->isInteger()) {
			//AvmAssert(false);	// Ensure we have a 32 bit signed integer, otherw se have to call integer, not AvmCore::integer_i
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreInteger), 1, avmObject);
		} else if (newNativeType->isBoolean()) {
			return new llvm::ICmpInst(*llvmBasicBlock, llvm::ICmpInst::ICMP_NE, avmObject, _nullObjectAtom, "");
		} else {
			printf("Going from AvmObject to native: %s\n", newNativeType->toString().data());
			TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	llvm::Value* LlvmIRGenerator::castStringToNative(llvm::Value* stringValue, TessaTypes::Type* newNativeType) {
		TessaAssert(stringValue->getType()->isPointerTy());
		
		if (newNativeType->isObject() || newNativeType->isScriptObject()) {
			return stringValue;
		} else if (newNativeType->isBoolean()) {
			llvm::Value* atomString = nativeToAtom(stringValue, _tessaTypeFactory->stringType());
			// AvmCore boolean returns an int8ty, convert it to a int1ty
			llvm::Value* avmcoreBoolean = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreBoolean), 1, atomString);
			// Compare to shift back down to a int1ty
			return new llvm::ICmpInst(*llvmBasicBlock, llvm::ICmpInst::ICMP_NE, avmcoreBoolean, createConstantInt(0), "");
		} else if (newNativeType->isNull()) {
			return _nullObjectAtom;
		} else {
			printf("Error casting string to native type %s\n", newNativeType->toString().data());
			TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;

	}

	/***
	 * All llvm values must be typed. This creates the appropriate casting operations for primitive types. (Eg int -> double).
	 */
	llvm::Value* LlvmIRGenerator::castToNative(llvm::Value* native, TessaTypes::Type* nativeType, TessaTypes::Type* newNativeType) {
		if (nativeType == newNativeType) {
			return (llvm::Instruction*) native;
		}

		if (newNativeType->isAnyType()) {
			return nativeToAtom(native, nativeType);
		}

		if (newNativeType->isString()) { 
			llvm::Value* atomValue = nativeToAtom(native, nativeType);
			return createBitCast(createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreString), 2, _avmcorePointer, atomValue), _pointerType);
		}

		if (nativeType->isBoolean()) {
			return castBooleanToNative(native, newNativeType);
		} else if (nativeType->isUnsignedInt()) {
			if (newNativeType->isNumber()) { 
				return new llvm::UIToFPInst(native, _doubleType, "", llvmBasicBlock);
			} else if (newNativeType->isInteger()) { 
				return (llvm::Instruction*)native;
			} else if (newNativeType == _tessaTypeFactory->boolType()) {
				return new llvm::ICmpInst(*llvmBasicBlock, ICmpInst::ICMP_NE, native, createConstantInt(0), "");
			} else {
				printf("native type is: %s\n", newNativeType->toString().data());
				TessaAssert(false);
				return native;
			}
		} else if (nativeType->isInteger()) {
			return castIntegerToNative(native, newNativeType);
		} else if (nativeType->isNumber()) {
			return castFloatToNative(native, newNativeType);
		} else if (nativeType->isString()) {
			return castStringToNative(native, newNativeType);
		} else if (nativeType->isPointer()) {
			return castScriptObjectToNative(native, newNativeType);
		} else if (nativeType->isObject()) {
			return castAvmObjectToNative(native, newNativeType);
		} else if (nativeType->isAnyType()) {
			return atomToNative(native, newNativeType);
		} else if (nativeType->isUndefined() || nativeType->isNull()) {
			return castUndefinedToNative(native, newNativeType);
		} else {
			printf("Casting native %s to %s\n", nativeType->toString().data(), newNativeType->toString().data());
			AvmAssert(false);
			return NULL;
		}
	}

	llvm::Value* LlvmIRGenerator::nativeToAtom(llvm::Value* object, Traits* objectTraits) {
		TessaTypes::Type* tessaType = getTessaType(objectTraits);
		return nativeToAtom(object, tessaType);
	}

	/*** 
	 * ActionSCript objects in JITted code are represented as Atoms. Many of these atoms are actually the
	 * AvmCore::ScriptObject. Cast the Atom -> ScriptObject* pointer type
	 */
	llvm::Instruction* LlvmIRGenerator::AtomToScriptObject(llvm::Value* scriptObject) {
		TessaAssert(scriptObject->getType()->isIntegerTy());
		// To go from atom to AvmCore::ScriptObject : ptr & ~7
		return castIntToPtr(llvm::BinaryOperator::Create(Instruction::And, scriptObject, createConstantInt(~7), "", llvmBasicBlock));
	}

	// Go from a native AvmCore::scriptObject back to atom
	llvm::Instruction* LlvmIRGenerator::ScriptObjectToAtom(llvm::Value* scriptObject) {
		// Do C equivalent of return kObjectType|(uintptr)this;
		TessaAssert(scriptObject->getType()->isPointerTy());
		llvm::Value* intObject = castPtrToInt(scriptObject);
		return llvm::BinaryOperator::Create(Instruction::Or, intObject, createConstantInt(kObjectType), "", llvmBasicBlock);
	}

	/***
	 * If the int value is a constant, return the atom integer value instead of calling
	 * AvmCore::intToAtom
	 */
	llvm::Value* LlvmIRGenerator::optimizeNativeIntegerToAtom(llvm::Value* intValue) {
		//return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreIntToAtom), 2, _avmcorePointer, intValue);

		if (dynamic_cast<llvm::ConstantInt*>(intValue)) {
			llvm::ConstantInt* constantInt = dynamic_cast<llvm::ConstantInt*>(intValue);
			int rawIntValue = constantInt->getSExtValue();
			if (atomIsValidIntptrValue(rawIntValue)) {
				return createConstantInt(core->intToAtom(rawIntValue));
			}
		} 

		return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreIntToAtom), 2, _avmcorePointer, intValue);
	}

	llvm::Value* LlvmIRGenerator::nativeToAtom(llvm::Value* value, TessaTypes::Type* typeOfValue) {
		if (typeOfValue->isBoolean()) {
			return llvm::SelectInst::Create(value, _trueAtom, _falseAtom, "Boolean toAtom", llvmBasicBlock);
		} else if (typeOfValue->isString()) {
			 // (AtomConstants::kStringType | uintptr_t(this));
			value = castPtrToInt(value);
			return llvm::BinaryOperator::Create(llvm::Instruction::Or, value, createConstantInt(kStringType), "", llvmBasicBlock);
		} else if (typeOfValue->isUnsignedInt()) {
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreUintToAtom), 2, _avmcorePointer, value);
		} else if (typeOfValue->isInteger()) {
			return optimizeNativeIntegerToAtom(value);
		} else if (typeOfValue->isNumber()) {
			return createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreDoubleToAtom), 2, _avmcorePointer, value);
		} else if (typeOfValue->isPointer()) {
			return ScriptObjectToAtom(value);
		} else {
			// Value should already be an atom
			TessaAssert(isAtomType(value));
			return value;
		}

		TessaAssert(false);
		return NULL;
	}

	TessaTypes::Type* LlvmIRGenerator::getTessaType(Traits* traits) {
		avmplus::BuiltinType type = Traits::getBuiltinType(traits);
		switch (type) 
		{
        case BUILTIN_number:
			return _tessaTypeFactory->numberType();
        case BUILTIN_any:
			return _tessaTypeFactory->anyType();
        case BUILTIN_object:
			return _tessaTypeFactory->objectType();
        case BUILTIN_void:
			return _tessaTypeFactory->voidType();
        case BUILTIN_int:
			return _tessaTypeFactory->integerType();
        case BUILTIN_uint:
			return _tessaTypeFactory->uintType();
        case BUILTIN_boolean:
			return _tessaTypeFactory->boolType();
        case BUILTIN_string:
			return _tessaTypeFactory->stringType();
		case BUILTIN_array:
			return _tessaTypeFactory->anyArrayType();
		case BUILTIN_regexp:
		case BUILTIN_none:
			return _tessaTypeFactory->scriptObjectType();
        case BUILTIN_namespace:
		{
			TessaAssert(false);
			return _tessaTypeFactory->anyType();
		}
		case BUILTIN_vector:
			return _tessaTypeFactory->anyVectorType();
		case BUILTIN_vectorobj:
			return _tessaTypeFactory->objectVectorType();
		case BUILTIN_vectoruint:
			return _tessaTypeFactory->uintVectorType();
		case BUILTIN_vectorint:
			return _tessaTypeFactory->intVectorType();
		case BUILTIN_vectordouble:
			return _tessaTypeFactory->numberVectorType();
		case BUILTIN_function:
		case BUILTIN_date:
		case BUILTIN_class:
		{
			return _tessaTypeFactory->scriptObjectType();
		}
        default:
		{
			printf("No tessa type for builtin type: %d\n", type);
			TessaAssert(false);
			return _tessaTypeFactory->anyType();
		}
        }
	}

	/***
	 * AVM defaults to saying this is a script object
	 */
	llvm::Value* LlvmIRGenerator::loadVTable(llvm::Value* object, TessaTypes::Type* tessaType) {
		// Assumes object is a boxed Atom
		if (tessaType->isArray() || tessaType->isScriptObject()) { 
			return createLlvmLoadPtr(object, offsetof(ScriptObject, vtable) / sizeof(int32_t));
		} else {
			// Atom
			//TessaAssert((tessaType == TESSA_ANY) || (tessaType == ObjectType::getObjectType())); 
			// Can optimize this a bit later
			llvm::Value* objectAtom = nativeToAtom(object, tessaType);
			return createBitCast(createLlvmCallInstruction(GET_LLVM_CALLINFO(toVtable), 2, _toplevelPointer, objectAtom), _pointerType);
		}
	}

	void LlvmIRGenerator::checkPointerSizes() {
	/***
		 * All of our calculations assume int, void*, ptr* are the same time. Otherwise, we misaling with visual studio.
		 * We also assume doubles are 8 bytes
		 */
		TessaAssert(sizeof(intptr_t) == 4);
		TessaAssert(sizeof(uintptr_t) == 4);
		TessaAssert(sizeof(void*) == 4);
		TessaAssert(sizeof(double) == 8);
	}

	/***
	 * The arguments pointer consists of all the arguments to a method call.
	 * Each argument should be unboxed into their native value. Thus, any calls to this compiled method must have already
	 * unboxed the values of the arguments.
	 * arguments look like this:
	 * Arguments[0] = "this" pointer unboxed as a value.
	 * Arguments[1 - n] = values unboxed.
	 * In addition, all double values are 8 byte aligned. The spacing between 8 byte alignments are pads.
	 */
	llvm::Value* LlvmIRGenerator::createTypedArgumentsPointer(ArrayOfInstructions* arrayOfInstructions, MethodSignaturep methodSignature, int argCount) {
		bool isSigned = false;
		llvm::Value* numberOfInstructions = this->createConstantInt(arrayOfInstructions->size());
		int paramCount = methodSignature->param_count();
		checkPointerSizes();
		TessaAssert(arrayOfInstructions->size() <= maxArgPointerSize);

		checkPointerSizes();
		TessaAssert((int32_t) arrayOfInstructions->size() >= methodSignature->requiredParamCount());

		int displacement = 0;
		for (int i = 0; i <= argCount; i++) {
			TessaInstruction* arrayElement = arrayOfInstructions->getInstruction(i);
			TessaTypes::Type* instructionType = arrayElement->getType();
            Traits* paramTraits = i <= paramCount ? methodSignature ->paramTraits(i) : NULL;
			TessaTypes::Type* declaredParamType = getTessaType(paramTraits);

			llvm::Value* coercedParamValue = castToNative(getValue(arrayElement), instructionType, declaredParamType);
			if (isLlvmPointerType(declaredParamType)) {
				createLlvmStorePointer(_allocedArgumentsPointer, coercedParamValue, displacement / sizeof(int32_t));
				displacement += sizeof(intptr_t);
			} else if (declaredParamType == _tessaTypeFactory->numberType()) {
				llvm::Value* doubleAddress = doPointerArithmetic(_allocedArgumentsPointer, displacement);
				createLlvmStoreDouble(doubleAddress, coercedParamValue, 0);
				displacement += sizeof(double);
			} else if (declaredParamType == _tessaTypeFactory->boolType()) {
				// llvm stores booleans as 1 bit values. We have to expand it to the size of an int
				coercedParamValue = castToNative(coercedParamValue, _tessaTypeFactory->boolType(), _tessaTypeFactory->integerType());
				createLlvmStoreInt(_allocedArgumentsPointer, coercedParamValue, displacement / sizeof(int32_t));
				displacement += sizeof(void*);
			} else {
				TessaAssert((declaredParamType->isInteger()) || (declaredParamType->isUnsignedInt())  
					|| (declaredParamType->isObject()) || (declaredParamType->isAnyType()));
				createLlvmStoreInt(_allocedArgumentsPointer, coercedParamValue, displacement / sizeof(int32_t));
				displacement += sizeof(void*);
			}
		}

		return _allocedArgumentsPointer;
	}

	/***
	 * Creates an array of atoms. Each value will be converted to an atom
	 */
	llvm::Value* LlvmIRGenerator::createAtomArgumentsPointer(ArrayOfInstructions* arrayOfInstructions) {
		bool isSigned = false;
		TessaAssert(arrayOfInstructions->size() <= maxArgPointerSize);

		for (uint32_t i = 0; i < arrayOfInstructions->size(); i++) {
			TessaInstruction* arrayElement = arrayOfInstructions->getInstruction(i);
			TessaTypes::Type* instructionType = arrayElement->getType();
			llvm::Value* atomValue = nativeToAtom(getValue(arrayElement), instructionType);

			// Have to cast this because alloced is of pointer type
			atomValue = castIntToPtr(atomValue);
			createLlvmStore(_allocedArgumentsPointer, atomValue, i);
		}

		return _allocedArgumentsPointer;
	}

	llvm::Value* LlvmIRGenerator::castReturnValue(CallInstruction* callInstruction, llvm::Value* returnValue) {
		TessaTypes::Type* returnType = callInstruction->getType();
		Traits* resultTraits = callInstruction->getResultTraits();

		if (returnType->isPointer()) {
			if (isAtomType(returnValue)) {
				AvmAssert(callInstruction->isInlined());
				return castToNative(returnValue, callInstruction->resolve()->getType(), returnType);
			} else {
				TessaAssert(returnValue->getType()->isPointerTy());
				return createBitCast(returnValue, _pointerType);
			}
		} else if (returnType->isBoolean()) {
			if (isAtomType(returnValue)) {
				return castToNative(returnValue, _tessaTypeFactory->integerType(), _tessaTypeFactory->boolType());
			} else {
				TessaAssert(returnValue->getType() == llvm::Type::getInt1Ty(*context));
				return returnValue;
			}
		} else if (returnType->isObject()) {
			// Have to coerce undefined values to null types. Can optimize this away with the verifier
			llvm::Value* isUndefined = new llvm::ICmpInst(*llvmBasicBlock, llvm::ICmpInst::ICMP_EQ, _undefinedAtom, returnValue, "");
			return llvm::SelectInst::Create(isUndefined, _nullObjectAtom, returnValue, "", llvmBasicBlock);
		} else {
			return returnValue;
		}
	}

	const llvm::FunctionType* LlvmIRGenerator::getInvokeFunctionType(TessaTypes::Type* callReturnType) {
		const LlvmCallInfo* llvmCallInfo;
		if (callReturnType->isNumber()) {
			llvmCallInfo = GET_LLVM_CALLINFO(InvokeReturnNumber);
		} else if (callReturnType->isPointer()) {
			llvmCallInfo = GET_LLVM_CALLINFO(InvokeReturnPointer);
		} else {
			llvmCallInfo = GET_LLVM_CALLINFO(InvokeReturnInt);
		}

		return getLlvmFunction(llvmCallInfo)->getFunctionType();
	}

	const llvm::FunctionType* LlvmIRGenerator::getInterfaceInvokeFunctionType(TessaTypes::Type* interfaceReturnType) {
		const LlvmCallInfo* llvmCallInfo;
		if (interfaceReturnType->isNumber()) {
			llvmCallInfo = GET_LLVM_CALLINFO(InterfaceInvokeReturnNumber);
		} else if (interfaceReturnType->isPointer()) {
			llvmCallInfo = GET_LLVM_CALLINFO(InterfaceInvokeReturnPointer);
		} else {
			llvmCallInfo = GET_LLVM_CALLINFO(InterfaceInvokeReturnInt);
		}

		return getLlvmFunction(llvmCallInfo)->getFunctionType();
	}

	const llvm::FunctionType* LlvmIRGenerator::getCallCacheHandlerFunctionType() {
		return getLlvmFunction(GET_LLVM_CALLINFO(CallCacheHandler))->getFunctionType();
	}

	const llvm::FunctionType* LlvmIRGenerator::getGetCacheHandlerFunctionType() {
		return getLlvmFunction(GET_LLVM_CALLINFO(GetCacheHandler))->getFunctionType();
	}

	const llvm::FunctionType* LlvmIRGenerator::getSetCacheHandlerFunctionType() {
		return getLlvmFunction(GET_LLVM_CALLINFO(SetCacheHandler))->getFunctionType();
	}

	llvm::Value* LlvmIRGenerator::loadMethodEnv(TessaInstruction* receiverObject, int methodId) {
		llvm::Value* llvmReceiverObject = getValue(receiverObject);
		llvm::Value* vtable = loadVTable(llvmReceiverObject, receiverObject->getType());
		int methodOffset = int32_t(offsetof(VTable, methods) + sizeof(MethodEnv*) * methodId) / sizeof(int32_t);	// divide by sizeof(int) since llvm multiplies by it later
		return createLlvmLoadPtr(vtable, methodOffset);
	}

	llvm::Value* LlvmIRGenerator::loadMethodInfo(llvm::Value* loadedMethodEnv) {
		TessaAssert(loadedMethodEnv->getType()->isPointerTy());
		int methodInfoOffset = int32_t(offsetof(MethodEnv, method)) / sizeof(int32_t);
		return createLlvmLoadPtr(loadedMethodEnv, methodInfoOffset);
	}

	// Do MethodInfo*->_implGPR(MethodEnv, int argCount, Typed Args*) - big win to inline this
	llvm::Value* LlvmIRGenerator::callMethodInfoImplGPR(llvm::Value* loadedMethodInfo, llvm::Value* loadedMethodEnv, llvm::Value* argCount, llvm::Value* argumentsPointer, TessaTypes::Type* resultType) {
		const llvm::FunctionType* invokeFunctionType = getInvokeFunctionType(resultType); 
		llvm::PointerType* invokeFunctionPointer = llvm::PointerType::get(invokeFunctionType, 0);

		llvm::Value* methodToCall = createLlvmLoadPtr(loadedMethodInfo, offsetof(MethodInfo, _implGPR) / sizeof(int32_t));
		methodToCall = createBitCast(methodToCall, invokeFunctionPointer);
		return createLlvmIndirectCallInstruction(methodToCall, invokeFunctionType, "MethodInfoInvoke", 3, loadedMethodEnv, argCount, argumentsPointer);
	}

	llvm::Value* LlvmIRGenerator::executeNonInlinedCallInstruction(TessaInstructions::CallInstruction* callInstruction, int argCount) {
		TessaAssert(false);
		int methodId = callInstruction->getMethodId();
		Traits* resultTraits = callInstruction->getResultTraits();

		llvm::Value* loadedMethodEnv = loadMethodEnv(callInstruction->getReceiverObject(), methodId);
		llvm::Value* loadedMethodInfo = loadMethodInfo(loadedMethodEnv); 
		llvm::Value* argumentsPointer = createTypedArgumentsPointer(callInstruction->getArguments(), callInstruction->getMethodInfo()->getMethodSignature(), callInstruction->getNumberOfArgs());

		TessaAssert(argCount == callInstruction->getNumberOfArgs());
		llvm::Value* llvmArgCount = createConstantInt(argCount);

		llvm::Value* returnValue = callMethodInfoImplGPR(loadedMethodInfo, loadedMethodEnv, llvmArgCount, argumentsPointer, callInstruction->getType());
		return castReturnValue(callInstruction, returnValue);
	}

	/***
	 * Call this method:
	 * Atom MethodInfoInvoke(MethodInfo* methodInfo, MethodEnv* env, int argc, Atom* args) { }
	 */
	llvm::Value* LlvmIRGenerator::executeCallInstruction(TessaInstructions::CallInstruction* callInstruction, int argCount) {
		TessaAssert(!callInstruction->isDynamicMethod());
		return executeNonInlinedCallInstruction(callInstruction, argCount);
	}

	llvm::Value* LlvmIRGenerator::executeDynamicCallInstruction(CallInstruction* callInstruction, int argCount) {
		TessaAssert(callInstruction->isDynamicMethod() && !callInstruction->hasAbcOpcode()); 
		TessaTypes::Type* functionObjectType = callInstruction->getFunctionObject()->getType();
		llvm::Value* functionObject = getValue(callInstruction->getFunctionObject());

		if (functionObjectType == _tessaTypeFactory->scriptObjectType()) {
			functionObject = ScriptObjectToAtom(functionObject);
		}

		//TessaAssert(callInstruction->getFunctionObject()->isScriptObject());
		
		TessaAssert(isAtomType(functionObject));
		llvm::Value* arguments = createAtomArgumentsPointer(callInstruction->getArguments());
		return createLlvmCallInstruction(GET_LLVM_CALLINFO(op_call_toplevel), 4, _toplevelPointer, functionObject, createConstantInt(argCount), arguments);
	}

	llvm::Value* LlvmIRGenerator::executeAbcCallInstruction(CallInstruction* callInstruction, int argCount) {
		TessaAssert(callInstruction->hasAbcOpcode());
		llvm::Value* arguments = createAtomArgumentsPointer(callInstruction->getArguments());
		llvm::Value* llvmArgCount = createConstantInt(argCount);

		switch (callInstruction->getAbcOpcode()) {
			case OP_applytype:
			{
				llvm::Value* functionObject = getValue(callInstruction->getFunctionObject());
				functionObject = nativeToAtom(functionObject, callInstruction->getFunctionObject()->getType());

				// Have to do arguments + 1 because arguments[0] is the function object, and apply_type Atom* arg can't have that
				return createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvOp_applytype), 4, _envPointer, functionObject, llvmArgCount, arguments);
			}
			case OP_newfunction:
			{
				// ArgCount doesn't count the "this" pointer
				TessaAssert(argCount == 0);
				TessaInstruction* methodIndex = callInstruction->getArguments()->getInstruction(0);
				llvm::Value* llvmMethodIndex = getValue(methodIndex);
				llvmMethodIndex = nativeToAtom(llvmMethodIndex, methodIndex->getType());
				llvm::Instruction* newFunctionResult = createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvNewFunction), 5, _envPointer, _avmcorePointer, _currentScopeDepth, _scopeStack, llvmMethodIndex);
				return createBitCast(newFunctionResult, _pointerType);
			}
			default:
				TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	void LlvmIRGenerator::visit(CallInstruction* callInstruction) {
		int argCount = callInstruction->getNumberOfArgs();
		llvm::Value* llvmCallInstruction;

		if (callInstruction->hasAbcOpcode()) {
			llvmCallInstruction = executeAbcCallInstruction(callInstruction, argCount);
		} else if (callInstruction->isDynamicMethod()) {
			llvmCallInstruction = executeDynamicCallInstruction(callInstruction, argCount);
		} else {
			TessaAssert(false);
			llvmCallInstruction = executeCallInstruction(callInstruction, argCount);
		}

		TessaAssert(llvmCallInstruction != NULL);
		putValue(callInstruction, llvmCallInstruction);
	}

	void LlvmIRGenerator::restoreMethodEnvPointers() {
		_methodEnvCallStack->removeLast();
		_toplevelPointerStack->removeLast();
		_methodEnvScopeChainStack->removeLast();
		_methodEnvVTableStack->removeLast();

		_envPointer = _methodEnvCallStack->last();
		_toplevelPointer = _toplevelPointerStack->last();
		_methodEnvScopeChain = _methodEnvScopeChainStack->last();
		_methodEnvVTable = _methodEnvVTableStack->last();
	}

	void LlvmIRGenerator::restoreScopeStackPointers() {
		_scopeStack = _scopeStackCallStack->removeLast();
		_currentScopeDepth = _scopeStackCountStack->removeLast();
	}

	void LlvmIRGenerator::visit(CallVirtualInstruction* callVirtualInstruction) {
		llvm::Value* returnValue = NULL;
		if (callVirtualInstruction->isInlined()) {
			returnValue = castReturnValue(callVirtualInstruction, getValue(callVirtualInstruction->resolve()));
			restoreMethodEnvPointers();
			restoreScopeStackPointers();
			methodInfo = _methodInfoStack->removeLast();
		} else {
			LoadVirtualMethodInstruction* loadVirtualMethod = callVirtualInstruction->getLoadedMethodToCall();
			llvm::Value* loadedMethodEnv = getValue(loadVirtualMethod->getLoadedMethodEnv());
			llvm::Value* loadedMethodInfo = getValue(loadVirtualMethod->getLoadedMethodInfo());
			
			int argCount = callVirtualInstruction->getNumberOfArgs();
			llvm::Value* llvmArgCount = createConstantInt(argCount);
			llvm::Value* argumentsPointer = createTypedArgumentsPointer(callVirtualInstruction->getArguments(), callVirtualInstruction->getMethodInfo()->getMethodSignature(), argCount);

			returnValue = callMethodInfoImplGPR(loadedMethodInfo, loadedMethodEnv, llvmArgCount, argumentsPointer, callVirtualInstruction->getType());
		}

		returnValue = castReturnValue(callVirtualInstruction, returnValue);
		putValue(callVirtualInstruction, returnValue);
	}

	void LlvmIRGenerator::visit(CallStaticInstruction* callStaticInstruction) {
		TessaAssert(false);
	}

	void LlvmIRGenerator::visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) {
		llvm::Value* loadedMethodEnv = loadMethodEnv(loadVirtualMethodInstruction->getReceiverObject(), loadVirtualMethodInstruction->getMethodId());
		llvm::Value* loadedMethodInfo = loadMethodInfo(loadedMethodEnv); 

		putValue(loadVirtualMethodInstruction->getLoadedMethodEnv(), loadedMethodEnv);
		putValue(loadVirtualMethodInstruction->getLoadedMethodInfo(), loadedMethodInfo);
	}

	void LlvmIRGenerator::visit(CallInterfaceInstruction* callInterfaceInstruction) {
		MethodInfo* methodInfo = callInterfaceInstruction->getMethodInfo();
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		int argCount = callInterfaceInstruction->getNumberOfArgs();

		llvm::Value* receiverObject = getValue(callInterfaceInstruction->getReceiverObject());
		llvm::Value* llvmArguments = createTypedArgumentsPointer(callInterfaceInstruction->getArguments(), methodSignature, argCount);
		llvm::Value* vtable = loadVTable(receiverObject, callInterfaceInstruction->getReceiverObject()->getType());

		// note, could be MethodEnv* or ImtThunkEnv*
		int index = int(callInterfaceInstruction->getMethodId() % VTable::IMT_SIZE);
		int offset = (offsetof(VTable,imt) + (sizeof(ImtThunkEnv*) * index)) / sizeof(int32_t);	// Let LLVM recalculate
		llvm::Value* methodEnv = createLlvmLoadPtr(vtable, offset);   
		llvm::Value* methodIid = createConstantPtr((intptr_t) callInterfaceInstruction->getMethodId());
		llvm::Value* methodAddress = createLlvmLoadPtr(methodEnv, (offsetof(MethodEnvProcHolder, _implGPR)) / sizeof(int32_t)); 

		TessaTypes::Type* returnType = callInterfaceInstruction->getType();
		int callInterfaceCount = 4;
		const llvm::FunctionType* indirectType = getInterfaceInvokeFunctionType(returnType);
		llvm::CallInst* returnValue = createLlvmIndirectCallInstruction(methodAddress, indirectType, "call interface", callInterfaceCount,
					methodEnv, createConstantInt(argCount), llvmArguments, methodIid
					);

		llvm::Value* castedReturnValue = castReturnValue(callInterfaceInstruction, returnValue);
		TessaAssert(castedReturnValue != NULL);
		putValue(callInterfaceInstruction, castedReturnValue);
	}

	llvm::Value* LlvmIRGenerator::callLateBoundCallSuper(CallSuperInstruction* callSuperInstruction) {
		TessaAssert(false);
		llvm::Value* receiverObject = getValue(callSuperInstruction->getReceiverObject());
		TessaAssert(callSuperInstruction->getReceiverObject()->getType() == _tessaTypeFactory->scriptObjectType());
		llvm::Value* receiverObjectAtom = ScriptObjectToAtom(receiverObject);
		llvm::Value* arguments = createAtomArgumentsPointer(callSuperInstruction->getArguments());
		llvm::Value* argCount = createConstantInt(callSuperInstruction->getNumberOfArgs());

		const Multiname* multiname = callSuperInstruction->getMultiname();
		if (multiname->isRuntime()) {
			TessaAssert(false);
		}

		createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvNullCheck), 2, _envPointer, receiverObjectAtom);
		return createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvCallSuper), 4, _envPointer, createConstantPtr((intptr_t)multiname), argCount, arguments);
	}

	llvm::Value* LlvmIRGenerator::callEarlyBoundCallSuper(CallSuperInstruction* callSuperInstruction) {
		TessaAssert(callSuperInstruction->isEarlyBound());
		TessaInstruction* tessaReceiverObject = callSuperInstruction->getReceiverObject();
		TessaAssert(tessaReceiverObject->isScriptObject());
		llvm::Value* receiverObject = getValue(callSuperInstruction->getReceiverObject());

		int argCount = callSuperInstruction->getNumberOfArgs();
		MethodInfo* methodInfo = callSuperInstruction->getMethodInfo();
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		TessaAssert(methodSignature->argcOk(argCount));

		int methodId = callSuperInstruction->getMethodId();
		// Do MethodEnv->method->implgpr()
		llvm::Value* baseVtable = createLlvmLoadPtr(_methodEnvVTable, offsetof(VTable, base) / sizeof(int32_t));
		int offset = int32_t(offsetof(VTable, methods) + sizeof(MethodEnv*) * methodId) / sizeof(int32_t);
		llvm::Value* loadedMethodEnv = createLlvmLoadPtr(baseVtable, offset);
		llvm::Value* methodAddress = createLlvmLoadPtr(loadedMethodEnv, offsetof(MethodEnvProcHolder,_implGPR) / sizeof(int32_t));
		llvm::Value* arguments = createTypedArgumentsPointer(callSuperInstruction->getArguments(), methodSignature, argCount);

		const llvm::FunctionType* invokeType = getInvokeFunctionType(callSuperInstruction->getType());
		return createLlvmIndirectCallInstruction(methodAddress, invokeType, "callSuperEarly", 3, loadedMethodEnv, createConstantInt(argCount), arguments);
	}

	void LlvmIRGenerator::visit(CallSuperInstruction* callSuperInstruction) {
		const Multiname* multiname = callSuperInstruction->getMultiname();
		if (multiname->isRuntime()) {
			TessaAssert(false);
		}

		llvm::Value* result;
		if (callSuperInstruction->isEarlyBound()) {
			result = callEarlyBoundCallSuper(callSuperInstruction);
		} else {
			result = callLateBoundCallSuper(callSuperInstruction);
		}
		
		TessaAssert(result != NULL);
		putValue(callSuperInstruction, result);
	}

	llvm::Value* LlvmIRGenerator::callPropertyLateBound(CallPropertyInstruction* callPropertyInstruction) {
		TessaAssert(!callPropertyInstruction->hasValidCacheSlot());
		const Multiname* propertyName = callPropertyInstruction->getProperty();
		if (propertyName->isRuntime()) {
			TessaAssert(false);
		} 

		TessaInstruction* tessaBaseObject = callPropertyInstruction->getReceiverObject()->resolve();
		TessaTypes::Type* receiverType = tessaBaseObject->getType();
		
		llvm::Value* llvmPropertyName = createConstantPtr((intptr_t)propertyName);
		llvm::Value* receiverObject = nativeToAtom(getValue(tessaBaseObject), receiverType);
		llvm::Value* arguments = createAtomArgumentsPointer(callPropertyInstruction->getArguments());
		llvm::Value* argCount = createConstantInt(callPropertyInstruction->getNumberOfArgs());
		llvm::Value* vtable = loadVTable(receiverObject, _tessaTypeFactory->anyType());

		// We can't do anything at all
		return createLlvmCallInstruction(GET_LLVM_CALLINFO(ToplevelCallProperty), 6, _toplevelPointer, receiverObject, llvmPropertyName, argCount, arguments, vtable); 
	}

	llvm::Value* LlvmIRGenerator::callPropertyWithCallCache(CallPropertyInstruction* callPropertyInstruction) {
		TessaAssert(callPropertyInstruction->hasValidCacheSlot());
		CallCache* cacheSlot = callPropertyInstruction->cacheSlot;
		int argCount = 5;
		
		TessaInstruction* tessaBaseObject = callPropertyInstruction->getReceiverObject();
		TessaTypes::Type* receiverType = tessaBaseObject->getType();
		llvm::Value* receiverObject = nativeToAtom(getValue(tessaBaseObject), receiverType);

		llvm::Value* atomArguments = createAtomArgumentsPointer(callPropertyInstruction->getArguments());
		llvm::Value* callCacheAddress = createConstantPtr((intptr_t)cacheSlot);
		llvm::Value* cacheHandler = createLlvmLoadPtr(callCacheAddress, (callPropertyInstruction->cacheHandlerOffset / sizeof(int32_t)));
		llvm::Value* callPropArgCount = createConstantInt(callPropertyInstruction->getNumberOfArgs());
		const llvm::FunctionType* callCacheFunctionType = getCallCacheHandlerFunctionType();

		// we call (*cache->handler)(cacheAddress, Atom obj, argc, Atom args*, MethodEnv*)
		return createLlvmIndirectCallInstruction(cacheHandler, callCacheFunctionType, "CallPropCache", argCount, 
			callCacheAddress, receiverObject, callPropArgCount, atomArguments, _envPointer);
	}

	llvm::Value* LlvmIRGenerator::callPropertySlotBound(CallPropertyInstruction* callPropertyInstruction) {
		TessaAssert(callPropertyInstruction->isSlotBound());
		llvm::Value* functionValue = getValue(callPropertyInstruction->getFunctionValue());

		llvm::Value* functionAtom = functionValue;
		if (!functionValue->getType()->isPointerTy()) {
			functionAtom = nativeToAtom(functionValue, callPropertyInstruction->getFunctionValue()->getType());
		}
		
		llvm::Value* atomArguments = createAtomArgumentsPointer(callPropertyInstruction->getArguments());
		int argCount = callPropertyInstruction->getNumberOfArgs();
		return createLlvmCallInstruction(GET_LLVM_CALLINFO(op_call_toplevel), 4, _toplevelPointer, functionAtom, createConstantInt(argCount), atomArguments);	
	}

	void LlvmIRGenerator::visit(CallPropertyInstruction* callPropertyInstruction) {
		llvm::Value* result;
		if (callPropertyInstruction->hasValidCacheSlot()) {
			result = callPropertyWithCallCache(callPropertyInstruction);
		} else {
			result = callPropertyLateBound(callPropertyInstruction);
		}

		TessaAssert(result != NULL);
		putValue(callPropertyInstruction, result);
	}


	void LlvmIRGenerator::visit(ConditionalBranchInstruction* conditionalBranchInstruction) {
		TessaInstruction* branchCondition = conditionalBranchInstruction->getBranchCondition();
		TessaAssert(branchCondition->isBoolean());
		llvm::Value* evaluatedCondition = getValue(branchCondition);

		TessaVM::BasicBlock* tessaTrueTarget = conditionalBranchInstruction->getTrueTarget();
		TessaVM::BasicBlock* tessaFalseTarget = conditionalBranchInstruction->getFalseTarget();

		llvm::BasicBlock* trueBasicBlock = getLlvmBasicBlock(tessaTrueTarget);
		llvm::BasicBlock* falseBasicBlock = getLlvmBasicBlock(tessaFalseTarget);

		AvmAssert(isBoolType(evaluatedCondition));
		llvm::Instruction* llvmBranchInstruction = llvm::BranchInst::Create(trueBasicBlock, falseBasicBlock, evaluatedCondition, llvmBasicBlock);
		putValue(conditionalBranchInstruction, llvmBranchInstruction);
	}

	void LlvmIRGenerator::visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) {
		llvm::BasicBlock* targetBlock = getLlvmBasicBlock(unconditionalBranchInstruction->getBranchTarget());
		llvm::Instruction* llvmUnconditionalBranch = llvm::BranchInst::Create(targetBlock, llvmBasicBlock);
		putValue(unconditionalBranchInstruction, llvmUnconditionalBranch);
	}
	
	/***
	 * When an AS3 variable is defined with the var keyword, it also has type information. However, before that point in the program, 
	 * the variable is defined as the undefined atom. If we have that case in a phi, where one operand is undefined, and the other is the defined var,
	 * we have to satisfy LLVM's type requirements. So here, insert a cast from the incoming edge casting undefined to the appropriate type.
	 */
	llvm::Value* LlvmIRGenerator::castUndefinedOperandInIncomingEdge(llvm::Value* castedIncomingValue, llvm::Instruction* incomingTerminator, TessaTypes::Type* tessaPhiType) {
		TessaAssert(incomingTerminator->isTerminator());
		if (castedIncomingValue == _undefinedAtom) {
			if (tessaPhiType->isNumber()) {
				castedIncomingValue = new llvm::SIToFPInst(castedIncomingValue, _doubleType, "", incomingTerminator);
			} else if (tessaPhiType->isPointer()) {
				castedIncomingValue = new llvm::IntToPtrInst(castedIncomingValue, _pointerType, "", incomingTerminator);
			} else if (tessaPhiType->isBoolean()) {
				castedIncomingValue = createConstantBoolean(false);
			} 
		} 

		return castedIncomingValue;
	}

	llvm::Value* LlvmIRGenerator::insertCastInIncomingBlock(llvm::Value* castedIncomingValue, llvm::BasicBlock* incomingBlock, TessaTypes::Type* tessaPhiType, TessaTypes::Type* tessaIncomingType) {
		if (tessaPhiType != tessaIncomingType) {
			/***
			 * Ugly ugly hack since castToNative may insert multiple instructions past a terminator.
			 * We have to take thoe instructions and reinsert them BEFORE the terminator of the incoming edge
			 */
			llvm::Instruction* incomingTerminator = incomingBlock->getTerminator();
			llvm::Instruction* currentTerminator = llvmBasicBlock->getTerminator();
			llvm::BasicBlock* lastBasicBlock = llvmBasicBlock;
			std::vector<llvm::Instruction*> castInstructions;

			llvm::Value* toNativeValue = castToNative(castedIncomingValue, tessaIncomingType, tessaPhiType);

			// Make sure cast to native didn't create new basic blocks.
			TessaAssert(lastBasicBlock == llvmBasicBlock);

			/***
			 * This could happen in cases where the builtin type is different
			 * but they are represented in the same format by the machine
			 * Ala Object -> Any
			 */
			if (toNativeValue == castedIncomingValue) {
				return castedIncomingValue;
			}

			/***
			 * Can happen if the cast to native finds two constants and pre computes the value for us
			 */
			if (dynamic_cast<llvm::Constant*>(toNativeValue)) {
				return toNativeValue;
			}

			castedIncomingValue = static_cast<llvm::Instruction*>(toNativeValue);
			TessaAssert(toNativeValue == (--(llvmBasicBlock->end())));

			/***
			 * Go through the current basic block and find the instructions that were added for the cast to native
			 */
			while (true) {
				llvm::Instruction* castInstruction = (--(llvmBasicBlock->end()));
				if (castInstruction == currentTerminator) {
					break;
				}
				castInstruction->removeFromParent();
				castInstructions.push_back(castInstruction);
			}

			/***
			 * Add those instructions back into the incoming edge's basic block
			 */
			for (int i = castInstructions.size() - 1; i >= 0; i--) {
				llvm::Instruction* castInstruction = castInstructions.at(i);
				castInstruction->insertBefore(incomingTerminator);
			}
		}

		return castedIncomingValue;
	}

	/***
	 * At this point we've already created all of the LLVM IR instructions and now finally updating phi operands.
	 * We also have to satisfy the LLVM type system. To do this, we have to insert the appropriate casting instructions
	 * right before the branch of the incoming edge.
	 */
	llvm::Value* LlvmIRGenerator::castPhiOperandInIncomingEdge(llvm::PHINode* phiNode, llvm::Value* llvmIncomingValue, llvm::BasicBlock* llvmIncomingBasicBlock, TessaTypes::Type* tessaPhiType, TessaTypes::Type* tessaIncomingType) {
		llvm::BasicBlock* incomingBlock = llvmIncomingBasicBlock;
		const llvm::Type* phiType = phiNode->getType();
		llvm::Value* castedIncomingValue = llvmIncomingValue;

		castedIncomingValue = insertCastInIncomingBlock(castedIncomingValue, incomingBlock, tessaPhiType, tessaIncomingType);
		castedIncomingValue = castUndefinedOperandInIncomingEdge(castedIncomingValue, incomingBlock->getTerminator(), tessaPhiType);

		/*** 
		 * Finally, if the phi represents a pointer, we have to cast the incoming operand pointer to the generic pointer type.
		 */
		if (phiType->isPointerTy()) { 
			const llvm::Type* operandType = castedIncomingValue->getType();
			TessaAssert(operandType->isPointerTy());
			if (phiType != operandType)  {
				castedIncomingValue = llvm::BitCastInst::Create(llvm::Instruction::BitCast, castedIncomingValue, phiType, "", incomingBlock->getTerminator());
			}
		}

		return castedIncomingValue;
	}

	/***
	 * Create the phi for now. For loops, we have to add the incoming values AFTER
	 * we go through the loop body
	 */
	void LlvmIRGenerator::addLlvmPhiValues(List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks) {
		debugMessage("Adding LLVM Phi Values");
		for (uint32_t i = 0; i < basicBlocks->size(); i++) {
			TessaVM::BasicBlock* basicBlock = basicBlocks->get(i);
			List<TessaInstructions::PhiInstruction*, avmplus::LIST_GCObjects>* phiInstructions = basicBlock->getPhiInstructions();
			//basicBlock->printResults();

			for (uint32_t phiIndex = 0; phiIndex < phiInstructions->size(); phiIndex++) {
				TessaInstructions::PhiInstruction* phiInstruction = phiInstructions->get(phiIndex);
				TessaTypes::Type* phiType = phiInstruction->getType();
				llvm::PHINode* llvmPhi = (llvm::PHINode*) getValue(phiInstruction);
				int numberOfOperands = phiInstruction->numberOfOperands();

				//phiInstruction->print();

				for (int phiOperandIndex = 0; phiOperandIndex < numberOfOperands; phiOperandIndex++) {
					TessaVM::BasicBlock* incomingBlock = phiInstruction->getIncomingEdge(phiOperandIndex);
					TessaInstruction* tessaIncomingValue = phiInstruction->getOperand(incomingBlock)->resolve();
					TessaTypes::Type* incomingType = tessaIncomingValue->getType();

					llvm::Value* llvmIncomingValue = getValue(tessaIncomingValue);
					llvm::BasicBlock* llvmIncomingBlock = _tessaToEndLlvmBasicBlockMap->get(incomingBlock);
					llvmIncomingValue = castPhiOperandInIncomingEdge(llvmPhi, llvmIncomingValue, llvmIncomingBlock, phiType, incomingType);
				//	llvmIncomingBlock->dump();

					// Could be another instruction that creates another basic block.
					/*
					if (llvm::Instruction* castedInstruction = dynamic_cast<llvm::Instruction*>(llvmIncomingValue)) {
						llvmIncomingBlock = castedInstruction->getParent();
					} 
					*/
					llvmPhi->addIncoming(llvmIncomingValue, llvmIncomingBlock);
				}	// End adding phi operands
			} // end finding phi instructions in basic block
		}	// End basic block
	}

	void LlvmIRGenerator::visit(PhiInstruction* phiInstruction) {
		const llvm::Type* typeOfPhi = this->getLlvmType(phiInstruction->getType());
		llvm::PHINode* llvmPhi = llvm::PHINode::Create(typeOfPhi, "", llvmBasicBlock);
		int numberOfOperands = phiInstruction->numberOfOperands();
		llvmPhi->reserveOperandSpace((uint32_t)numberOfOperands);
		putValue(phiInstruction, llvmPhi);
	}

	void LlvmIRGenerator::visit(ParameterInstruction* parameterInstruction) {
		// Parameter Instructions should already have been mapped to something
		TessaInstruction* resolvedInstruction = parameterInstruction->resolve();
		if (resolvedInstruction != parameterInstruction) {
			putValue(parameterInstruction, getValue(resolvedInstruction));
		} else {
			if (currentBasicBlock->getBasicBlockId() != 0) {
				/***
				 * When we inline a method, local variables that are not parameters
				 * will point to themselves. Those parameter instructions are "initialized" to undefined,
				 * which then get set to their default value later in the instruction stream.
				 * If this is the first basic block of a method, the parameter instruction should be mapped
				 * to a value already
				 */
				putValue(parameterInstruction, _undefinedAtom);
			} else {
				AvmAssert(containsLlvmValue(parameterInstruction));
			}
		}
	}

	void LlvmIRGenerator::visit(CoerceInstruction* coerceInstruction) {
		TessaTypes::Type* typeToConvert = coerceInstruction->getType();
		TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce()->resolve();
		TessaTypes::Type* originalType = instructionToCoerce->getType();

		llvm::Value* instructionValue = getValue(instructionToCoerce);
		llvm::Value* coercedType;

		if (coerceInstruction->useCoerceObjAtom) {
			instructionValue = nativeToAtom(instructionValue, originalType);
			TessaAssert(isAtomType(instructionValue));
			TessaAssert(isLlvmPointerType(typeToConvert));
			createLlvmCallInstruction(GET_LLVM_CALLINFO(CoerceObject_Atom), 3, _envPointer, instructionValue, createConstantPtr((intptr_t)(coerceInstruction->resultTraits)));
			coercedType = AtomToScriptObject(instructionValue);
		} else if (hasType(instructionToCoerce)) {
			coercedType = castToNative(instructionValue, originalType, typeToConvert);
		} else {
			// Means we have an atom
			coercedType = atomToNative(instructionValue, typeToConvert);
		}

		putValue(coerceInstruction, coercedType);
	}

	void LlvmIRGenerator::visit(ConvertInstruction* convertInstruction) {
		TessaTypes::Type* typeToConvert = convertInstruction->getType();
		TessaInstruction* instructionToCoerce = convertInstruction->getInstructionToCoerce()->resolve();
		TessaTypes::Type* originalType = instructionToCoerce->getType();

		llvm::Value* instructionValue = getValue(instructionToCoerce);
		llvm::Value* convertedType;

		if (hasType(instructionToCoerce)) {
			convertedType = castToNative(instructionValue, originalType, typeToConvert);
		} else {
			convertedType = atomToNative(instructionValue, typeToConvert);
		}

		TessaAssert(convertedType != NULL);
		putValue(convertInstruction, convertedType);
	}

	bool LlvmIRGenerator::isArrayElementAccess(TessaInstruction* receiverObject, TessaInstruction* key) {
		TessaTypes::Type* receiverType = receiverObject->getType();
		if (receiverType == _tessaTypeFactory->anyArrayType()) {
			if (key != NULL) {
				TessaTypes::Type* keyType = key->getType();
				return (keyType->isNumber()) || (keyType->isInteger()); 
			}
		}

		return false;
	}

	/***
	 * We can't use the TESSA type system yet because vector object type
	 */
	bool LlvmIRGenerator::isArrayOrObjectType(TessaTypes::Type* tessaType, Traits* objectTraits) {
		if (tessaType->isArray() || tessaType->isVector()) {
			if (tessaType->isVector()) {
				TessaTypes::VectorType* vectorType = reinterpret_cast<TessaTypes::VectorType*>(tessaType);
				return vectorType->getElementType()->isObject();
			} else {
				return true;
			}
		}

		return false;
	}

	bool LlvmIRGenerator::isIntOrUIntVector(TessaTypes::Type* objectType, Traits* objectTraits) {
		if (objectType->isVector()) {
			TessaTypes::VectorType* vectorType = reinterpret_cast<TessaTypes::VectorType*>(objectType);
			TessaTypes::Type* elementType = vectorType->getElementType();
			return elementType->isInteger() || elementType->isUnsignedInt();
		}

		return false;
	}

	llvm::Value* LlvmIRGenerator::getArrayOrVectorObjectIntegerIndex(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		TessaTypes::Type* resultType = getPropertyInstruction->getType();
		TessaTypes::Type* objectType = getPropertyInstruction->getReceiverInstruction()->resolve()->getType();
		TessaTypes::Type* indextype = getPropertyInstruction->getPropertyKey()->resolve()->getType();

		llvm::Value* result;
		Traits* objectTraits = getPropertyInstruction->objectTraits;

		AvmAssert(indextype->isInteger() || indextype->isUnsignedInt() || indextype->isNumber());

		const LlvmCallInfo* llvmCallInfo;
		if (objectType == _tessaTypeFactory->anyArrayType()) {
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(ArrayObjectGetUintProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(ArrayObjectGetIntProperty);
			}
		} else {
			AvmAssert(objectType->isVector());
			if (isUnsignedInteger) {
				TessaAssert(false);
				llvmCallInfo = GET_LLVM_CALLINFO(ObjectVectorObject_GetIntProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(ObjectVectorObject_GetIntProperty);
			}
		}

		result = createLlvmCallInstruction(llvmCallInfo, 2, receiverObject, llvmIndex);
		return atomToNative(result, resultType);
	}

	llvm::Value* LlvmIRGenerator::getIntegerVectorPropertyInline(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		llvm::BasicBlock* integerVectorErrorBasicBlock = llvm::BasicBlock::Create(*context, "GetVectorIntError", _llvmFunction);
		llvm::BasicBlock* integerVectorFastBlock = llvm::BasicBlock::Create(*context, "GetVectorIntFast", _llvmFunction);
		llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "GetVectorIntMerge", _llvmFunction);

		/***
		 * generate LLVM IR for "if (index <= IntVectorObject::m_length)"
		 */
		llvm::Value* arrayLength = createLlvmLoad(receiverObject, offsetof(IntVectorObject, m_length) / sizeof(int32_t));
		llvm::Value* isIndexValid = new llvm::ICmpInst(*llvmBasicBlock, llvm::CmpInst::ICMP_SLE, llvmIndex, arrayLength, "");
		llvm::Value* branchToFastPath = llvm::BranchInst::Create(integerVectorFastBlock, integerVectorErrorBasicBlock, isIndexValid, llvmBasicBlock);

		/***
		 * generate LLVM IR for "IntVectorObject::m_array[llvmIndex]"
		 */
		this->llvmBasicBlock = integerVectorFastBlock;
		llvm::Value* rawPointer = createLlvmLoad(receiverObject, offsetof(IntVectorObject, m_array) / sizeof(int32_t));
		llvm::Value* arrayOffset = llvm::BinaryOperator::Create(llvm::BinaryOperator::Shl, llvmIndex, createConstantInt(2), "", llvmBasicBlock);
		llvm::Value* arrayAddress = llvm::BinaryOperator::Create(llvm::BinaryOperator::Add, rawPointer, arrayOffset, "", llvmBasicBlock);
		llvm::Value* arrayAddressPointer = castIntToPtr(arrayAddress, "vector int pointer");
		llvm::Value* loadedIntegerVectorProperty = createLlvmLoad(arrayAddressPointer, 0);
		llvm::BranchInst::Create(mergeBlock, llvmBasicBlock);

		/***
		 * Fill out the error block
		 */
		this->llvmBasicBlock = integerVectorErrorBasicBlock;
		llvm::Value* slowPathLoadedIntVectorProperty = getIntegerVectorPropertyCall(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
		llvm::BranchInst::Create(mergeBlock, llvmBasicBlock);

		/***
		 * Merge block. Model the resulting vector element as a phi
		 */
		this->llvmBasicBlock = mergeBlock;
		llvm::PHINode* getIntProperty = llvm::PHINode::Create(_intType, "GetIntVector", llvmBasicBlock);
		getIntProperty->addIncoming(loadedIntegerVectorProperty, integerVectorFastBlock);
		getIntProperty->addIncoming(slowPathLoadedIntVectorProperty, integerVectorErrorBasicBlock);
		return getIntProperty;
		//return getIntegerVectorPropertyCall(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
	}

	llvm::Value* LlvmIRGenerator::getIntegerVectorPropertyCall(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		const LlvmCallInfo* llvmCallInfo;
		Traits* objectTraits = getPropertyInstruction->objectTraits;
		TessaTypes::Type* objectType = getPropertyInstruction->objectType;

		if (objectType == _tessaTypeFactory->intVectorType()) {
			TessaAssert(objectTraits == VECTORINT_TYPE); 
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(IntVectorObject_GetNativeUIntProperty); 
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(IntVectorObject_GetNativeIntProperty); 
			}
		} else {
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(UIntVectorObject_GetNativeUIntProperty); 
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(UIntVectorObject_GetNativeIntProperty); 
			}
		}

		TessaAssert(receiverObject->getType()->isPointerTy());
		return createLlvmCallInstruction(llvmCallInfo, 2, receiverObject, llvmIndex);
	}

	llvm::Value* LlvmIRGenerator::getIntegerVectorProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		Traits* objectTraits = getPropertyInstruction->objectTraits;
		Traits* resultTraits = getPropertyInstruction->resultTraits;

		TessaTypes::Type* resultType = getPropertyInstruction->getType();
		TessaTypes::Type* objectType = getPropertyInstruction->getReceiverInstruction()->resolve()->getType();
		TessaTypes::Type* keyType = getPropertyInstruction->getPropertyKey()->getType();

		if ((resultType == _tessaTypeFactory->integerType()) || (resultType == _tessaTypeFactory->uintType())) {
			//TessaAssert((resultTraits == INT_TYPE) || (resultTraits == UINT_TYPE)); 
			llvmIndex = castToNative(llvmIndex, keyType, _tessaTypeFactory->integerType());
			return getIntegerVectorPropertyInline(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
		} else {
			TessaAssert(false);
			return NULL;
		}
	}

	llvm::Value* LlvmIRGenerator::getDoubleVectorProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		Traits* resultTraits = getPropertyInstruction->resultTraits;
		TessaTypes::Type* resultType = getPropertyInstruction->getType();

		if (resultType == _tessaTypeFactory->numberType()) {
			//TessaAssert(resultTraits == NUMBER_TYPE);
			const LlvmCallInfo* llvmCallInfo;
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(DoubleVectorObject_GetNativeUIntProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(DoubleVectorObject_GetNativeIntProperty);
			}
			return createLlvmCallInstruction(llvmCallInfo, 2, receiverObject, llvmIndex);
		} else {
			TessaAssert(false);
			return NULL;
		}
	}

	llvm::Value* LlvmIRGenerator::getLateBoundIntegerProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		TessaTypes::Type* resultType = getPropertyInstruction->getType();
		TessaInstruction* tessaReceiver = getPropertyInstruction->getReceiverInstruction()->resolve();

		if (tessaReceiver->getType()->isPointer()) {
			AvmAssert(isPointerType(receiverObject));
			receiverObject = nativeToAtom(receiverObject, tessaReceiver->getType());
		}

		TessaAssert(isAtomType(receiverObject));
		const LlvmCallInfo* llvmCallInfo;
		if (isUnsignedInteger) {
			llvmCallInfo = GET_LLVM_CALLINFO(MethodEnvGetPropertyLateUnsignedInteger);
		} else {
			llvmCallInfo = GET_LLVM_CALLINFO(MethodEnvGetPropertyLateInteger);
		}

		llvm::Value* result = createLlvmCallInstruction(llvmCallInfo, 3, _envPointer, receiverObject, llvmIndex);
		return atomToNative(result, resultType);
	}

	llvm::Value* LlvmIRGenerator::earlyBindGetIntegerIndexResult(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger) {
		TessaTypes::Type* resultType = getPropertyInstruction->getType();
		TessaTypes::Type* objectType = getPropertyInstruction->getReceiverInstruction()->resolve()->getType();
		TessaTypes::Type* indextype = getPropertyInstruction->getPropertyKey()->getType();
		Traits* objectTraits = getPropertyInstruction->objectTraits;

		if (isArrayOrObjectType(objectType, objectTraits)) {
			TessaAssert(receiverObject->getType()->isPointerTy());
			TessaAssert(isAtomType(llvmIndex));
			AvmAssert(indextype->isInteger() || indextype->isUnsignedInt() || indextype->isNumber());
			AvmAssert(objectType->isArray() || objectType->isScriptObject()); 
			return getArrayOrVectorObjectIntegerIndex(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
		} else if (isIntOrUIntVector(objectType, objectTraits)) {
			//TessaAssert ((objectTraits == VECTORINT_TYPE) || (objectTraits == VECTORUINT_TYPE)); 
			return getIntegerVectorProperty(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
		} else if (objectType == _tessaTypeFactory->numberVectorType()) {
			//TessaAssert(objectTraits == VECTORDOUBLE_TYPE);
			return getDoubleVectorProperty(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
		} else {
			return getLateBoundIntegerProperty(getPropertyInstruction, receiverObject, llvmIndex, isUnsignedInteger);
		}
	}

	llvm::Value* LlvmIRGenerator::callGetCacheHandler(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObjectAtom) {
		TessaAssert(receiverObjectAtom->getType()->isIntegerTy());	// Receiver object has to be boxed into an atom
		GetCache* cacheSlot = getPropertyInstruction->getCacheSlot;
		TessaAssert(cacheSlot != NULL);
		
	    // Call with this signature: Atom getprop_miss(GetCache& c, MethodEnv* env, Atom obj)
		llvm::Value* cacheAddress = createConstantPtr((intptr_t) cacheSlot);
		llvm::Value* cacheHandler = createLlvmLoadPtr(cacheAddress, getPropertyInstruction->getCacheGetHandlerOffset / sizeof(int32_t));
		const llvm::FunctionType* getCacheFunctionType = getGetCacheHandlerFunctionType();
		int argCount = 3;
		return createLlvmIndirectCallInstruction(cacheHandler, getCacheFunctionType, "GetPropertyCache", argCount, 
			cacheAddress, _envPointer, receiverObjectAtom);
	}

	llvm::Value* LlvmIRGenerator::emitGetPropertySlow(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* llvmPropertyName) {
		TessaTypes::Type* resultType = getPropertyInstruction->getType();
		TessaTypes::Type* receiverType = getPropertyInstruction->getReceiverInstruction()->getType();
		receiverObject = nativeToAtom(receiverObject, receiverType);

		llvm::Value* result;
		if (getPropertyInstruction->usePropertyCache) {
			result = callGetCacheHandler(getPropertyInstruction, receiverObject);
		} else {
			// We're super lost
			TessaInstruction* tessaKey = getPropertyInstruction->getPropertyKey();
			llvmKey = nativeToAtom(llvmKey, tessaKey->getType());
			result = createLlvmCallInstruction(GET_LLVM_CALLINFO(ToplevelGetProperty), 4, _toplevelPointer, receiverObject, llvmKey, llvmPropertyName);
		}

		return atomToNative(result, resultType);
	}

	llvm::Value* LlvmIRGenerator::earlyBindGetPropertyWithMultiname(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* llvmKeyAtom, TessaTypes::Type* resultType) {
		const Multiname* propertyName = getPropertyInstruction->getPropertyMultiname();
		llvm::Value* llvmPropertyName = createConstantPtr((intptr_t)propertyName);
		TessaAssert(isAtomType(receiverObjectAtom));
		TessaAssert(isAtomType(llvmKeyAtom));

		llvm::Value* result = createLlvmCallInstruction(GET_LLVM_CALLINFO(GetPropertyIndex), 4, _envPointer, receiverObjectAtom, llvmPropertyName, llvmKeyAtom);
		return atomToNative(result, resultType);
	}

	/***
	 * This is mostly a copy of emit(OP_getproperty) attemps to early bind. This is horrific, and should later be cleaned out
	 * with real TESSA types. For now, we need it because the performance improvements are too good.
	 */
	llvm::Value* LlvmIRGenerator::earlyBindGetProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* llvmPropertyName, llvm::Value* receiverObject, llvm::Value* llvmKey) {
		TessaInstruction* tessaReceiverObject = getPropertyInstruction->getReceiverInstruction();
		TessaTypes::Type* receiverType = tessaReceiverObject->resolve()->getType();
		TessaTypes::Type* resultType = getPropertyInstruction->getType();
		TessaTypes::Type* indexType = _tessaTypeFactory->anyType();
		TessaValue* propertyKey = getPropertyInstruction->getPropertyKey();
		if (propertyKey != NULL) {
			indexType = propertyKey->getType();
		}

		Traits* indexTraits = getPropertyInstruction->getIndexTraits();
		Traits* objectTraits = getPropertyInstruction->objectTraits;
		Traits* resultTraits = getPropertyInstruction->resultTraits;

		const Multiname* propertyName = getPropertyInstruction->getPropertyMultiname();
		bool attribute = propertyName->isAttr();
		bool maybeIntegerIndex = !attribute && propertyName->isRtname() && propertyName->containsAnyPublicNamespace();
		llvm::Value* result;

		if ((maybeIntegerIndex && (indexTraits == INT_TYPE)) || (indexType->isInteger())) {
			bool isUnsignedInteger = false;
			if (indexType->isNumber()) {
				AvmAssert(llvmKey->getType() == _doubleType);
				llvmKey = castFloatToNative(llvmKey, _tessaTypeFactory->integerType());
			}
			AvmAssert(llvmKey->getType()->isIntegerTy());
			result = earlyBindGetIntegerIndexResult(getPropertyInstruction, receiverObject, llvmKey, isUnsignedInteger);
		} else if ((maybeIntegerIndex && (indexTraits == UINT_TYPE)) || (indexType->isUnsignedInt())) {
			bool isUnsignedInteger = true;
			result = earlyBindGetIntegerIndexResult(getPropertyInstruction, receiverObject, llvmKey, isUnsignedInteger);
		} else if (maybeIntegerIndex && (indexTraits != STRING_TYPE)) {
			TessaInstruction* tessaKey = getPropertyInstruction->getPropertyKey();
			receiverObject = nativeToAtom(receiverObject, receiverType);
			llvmKey = nativeToAtom(llvmKey, tessaKey->getType());
			result = earlyBindGetPropertyWithMultiname(getPropertyInstruction, receiverObject, llvmKey, resultType);
		} else {
			result = emitGetPropertySlow(getPropertyInstruction, receiverObject, llvmKey, llvmPropertyName);
		}

		TessaAssert(result != NULL);
		return result;
	}

	void LlvmIRGenerator::visit(GetPropertyInstruction* getPropertyInstruction) {
		const Multiname* propertyName = getPropertyInstruction->getPropertyMultiname();
		llvm::Value* llvmPropertyName = createConstantPtr((intptr_t)propertyName);

		TessaInstruction* receiverInstruction = getPropertyInstruction->getReceiverInstruction();
		TessaTypes::Type* receiverType = receiverInstruction->getType();
		llvm::Value* receiverObject = getValue(receiverInstruction);
		llvm::Value* result;
		
		TessaInstruction* key = getPropertyInstruction->getPropertyKey();
		llvm::Value* llvmKey = _nullObjectAtom;
		if (key != NULL) {
			llvmKey = getValue(key);	
		} 

		result = earlyBindGetProperty(getPropertyInstruction, llvmPropertyName, receiverObject, llvmKey);
		TessaAssert(result != NULL);
		putValue(getPropertyInstruction, result);
	}

	void LlvmIRGenerator::emitSetPropertySlow(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* valueToSet, llvm::Value* llvmPropertyName) {
		TessaTypes::Type* receiverType = setPropertyInstruction->getReceiverInstruction()->resolve()->getType();
		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->resolve()->getType();

		receiverObject = nativeToAtom(receiverObject, receiverType);

		if (setPropertyInstruction->usePropertyCache) {
			// Give up, we have no idea what kind of property we are getting, so just use the cache
			valueToSet = nativeToAtom(valueToSet, valueType);
			callSetCacheHandler(setPropertyInstruction, receiverObject, valueToSet);
		} else {
			if (setPropertyInstruction->isInitProperty()) {
				AvmAssert(false);
			} else {
				// Super duper lost
				TessaAssert(false);
				TessaTypes::Type* keyType = setPropertyInstruction->getPropertyKey()->getType();
				llvmKey = nativeToAtom(llvmKey, keyType);
				valueToSet = nativeToAtom(valueToSet, valueType);
				createLlvmCallInstruction(GET_LLVM_CALLINFO(ToplevelSetProperty), 5, _toplevelPointer, receiverObject, valueToSet, llvmKey, llvmPropertyName);
			}
		}
	}

	void LlvmIRGenerator::earlyBindSetIntegerIndexResult(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		TessaTypes::Type* resultType = setPropertyInstruction->getType();
		TessaTypes::Type* objectType = setPropertyInstruction->getReceiverInstruction()->resolve()->getType();
		Traits* objectTraits = setPropertyInstruction->objectTraits;

		if (isArrayOrObjectType(objectType, objectTraits)) {
			TessaAssert(receiverObject->getType()->isPointerTy());
			(isAtomType(llvmIndex));
			TessaTypes::Type* indexType = setPropertyInstruction->getPropertyKey()->resolve()->getType();
			AvmAssert(indexType->isInteger() || indexType->isUnsignedInt() || indexType->isNumber());
			AvmAssert(objectType->isArray() || objectType->isScriptObject()); 
			return setArrayOrVectorObjectIntegerIndex(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
		} else if (isIntOrUIntVector(objectType, objectTraits)) {
			//TessaAssert ((objectTraits == VECTORINT_TYPE) || (objectTraits == VECTORUINT_TYPE)); 
			TessaTypes::Type* indexType = setPropertyInstruction->getPropertyKey()->getType();
			AvmAssert(indexType->isInteger() || indexType->isUnsignedInt() || indexType->isNumber());
			
			return setIntegerVectorProperty(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
		} else if (objectType == _tessaTypeFactory->numberVectorType()) {
			//TessaAssert(objectTraits == VECTORDOUBLE_TYPE);
			return setDoubleVectorProperty(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
		} else {
			return setLateBoundIntegerProperty(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
		}
	}

	void LlvmIRGenerator::setPropertyWithMultiname(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* indexAtom, llvm::Value* valueToSetAtom) {
		const Multiname* propertyName = setPropertyInstruction->getPropertyName();
		llvm::Value* multinameConstant = createConstantPtr((intptr_t)propertyName); // precomputed multiname
		TessaAssert(isAtomType(receiverObjectAtom));
		TessaAssert(isAtomType(valueToSetAtom));
		TessaAssert(isAtomType(indexAtom));
		createLlvmCallInstruction(GET_LLVM_CALLINFO(SetPropertyIndex), 5, _envPointer, receiverObjectAtom, multinameConstant, valueToSetAtom, indexAtom);
	}

	/***
	 * The early binding semantics are WTF crazy
	 */ 
	void LlvmIRGenerator::earlyBindSetProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* llvmPropertyName, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* valueToSet) {
		const Multiname* propertyName = setPropertyInstruction->getPropertyName();
		bool attr = propertyName->isAttr();
		bool indexMaybeInteger = !attr && (propertyName->isRtname()) && propertyName->containsAnyPublicNamespace();
		Traits* indexTraits = setPropertyInstruction->getIndexTraits();
		Traits* objectTraits = setPropertyInstruction->objectTraits;
		Traits* valueTraits = setPropertyInstruction->valueTraits;
		TessaTypes::Type* indexType = _tessaTypeFactory->anyType();
		TessaValue* propertyKey = setPropertyInstruction->getPropertyKey();
		if (propertyKey != NULL) {
			indexType = propertyKey->getType();
		}

		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->getType();
		TessaTypes::Type* receiverType = setPropertyInstruction->getReceiverInstruction()->resolve()->getType();

		if ((indexMaybeInteger && (indexTraits == INT_TYPE)) || (indexType->isInteger())) {
			bool isUnsignedInteger = false;
			if (indexType->isNumber()) {
				AvmAssert(llvmKey->getType() == _doubleType);
				llvmKey = castFloatToNative(llvmKey, _tessaTypeFactory->integerType());
			}

			AvmAssert(isAtomType(llvmKey));
			earlyBindSetIntegerIndexResult(setPropertyInstruction, receiverObject, llvmKey, valueToSet, isUnsignedInteger);
		} else if (indexMaybeInteger && (indexTraits == UINT_TYPE)) {
			TessaAssert(indexMaybeInteger && (indexTraits == UINT_TYPE)); 
			TessaAssert(indexType->isInteger() || indexType->isUnsignedInt()); 
			bool isUnsignedInteger = true;
			earlyBindSetIntegerIndexResult(setPropertyInstruction, receiverObject, llvmKey, valueToSet, isUnsignedInteger);
		} else if (indexMaybeInteger) {
			TessaTypes::Type* keyType = setPropertyInstruction->getPropertyKey()->getType();
			receiverObject = nativeToAtom(receiverObject, receiverType);
			llvmKey = nativeToAtom(llvmKey, keyType);
			valueToSet = nativeToAtom(valueToSet, valueType);
			setPropertyWithMultiname(setPropertyInstruction, receiverObject, llvmKey, valueToSet);
		} else {
			emitSetPropertySlow(setPropertyInstruction, receiverObject, llvmKey, valueToSet, llvmPropertyName);
		} // end else
	} // end WTF 

	/**
	 * The object we are setting the property on is either an array or vector.<Object> type.
	 * The "property name" is an integer, hence llvmIndex.
	 * The value to set can be anything.
	 */
	void LlvmIRGenerator::setArrayOrVectorObjectIntegerIndex(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->resolve()->getType();
		valueToSet = nativeToAtom(valueToSet, valueType);
		TessaAssert(receiverObject->getType()->isPointerTy());
		TessaAssert(isAtomType(llvmIndex));

		TessaTypes::Type* indexType = setPropertyInstruction->getPropertyKey()->resolve()->getType();
		Traits* objectTraits = setPropertyInstruction->objectTraits;
		TessaTypes::Type* objectType = setPropertyInstruction->getReceiverInstruction()->resolve()->getType();
		const LlvmCallInfo* llvmCallInfo;

		if (objectType == _tessaTypeFactory->anyArrayType()) {
			AvmAssert(indexType->isInteger() || indexType->isUnsignedInt() || indexType->isNumber());
			//TessaAssert(objectTraits == ARRAY_TYPE);

			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(ArrayObjectSetUintProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(ArrayObjectSetIntProperty);
			}
		} else {
			if (isUnsignedInteger) {
				TessaAssert(false);
				llvmCallInfo = GET_LLVM_CALLINFO(ObjectVectorObject_SetIntProperty);
			} else {
				AvmAssert(indexType->isInteger());
				llvmCallInfo = GET_LLVM_CALLINFO(ObjectVectorObject_SetIntProperty);
			}
		}

		createLlvmCallInstruction(llvmCallInfo, 3, receiverObject, llvmIndex, valueToSet);
	}

	void LlvmIRGenerator::setIntegerVectorPropertyInline(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		llvm::BasicBlock* integerVectorErrorBasicBlock = llvm::BasicBlock::Create(*context, "SetVectorIntError", _llvmFunction);
		llvm::BasicBlock* integerVectorFastBlock = llvm::BasicBlock::Create(*context, "SetVectorIntFast", _llvmFunction);
		llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "SetVectorIntMerge", _llvmFunction);

		/***
		 * do if (index < IntVectorObject::m_length)
		 */
		llvm::Value* arrayLength = createLlvmLoad(receiverObject, offsetof(IntVectorObject, m_length) / sizeof(int32_t));
		llvm::Value* isIndexValid = new llvm::ICmpInst(*llvmBasicBlock, llvm::CmpInst::ICMP_SLT, llvmIndex, arrayLength, "");
		llvm::Value* branchToFastPath = llvm::BranchInst::Create(integerVectorFastBlock, integerVectorErrorBasicBlock, isIndexValid, llvmBasicBlock);

		/***
		 * do IntVectorObject::m_array[llvmIndex] = valueToSet
		 */
		this->llvmBasicBlock = integerVectorFastBlock;
		llvm::Value* rawPointer = createLlvmLoad(receiverObject, offsetof(IntVectorObject, m_array) / sizeof(int32_t));
		llvm::Value* arrayOffset = llvm::BinaryOperator::Create(llvm::BinaryOperator::Shl, llvmIndex, createConstantInt(2), "", llvmBasicBlock);
		llvm::Value* arrayAddress = llvm::BinaryOperator::Create(llvm::BinaryOperator::Add, rawPointer, arrayOffset, "", llvmBasicBlock);
		llvm::Value* arrayAddressPointer = castIntToPtr(arrayAddress, "vector int pointer");
		createLlvmStore(arrayAddressPointer, valueToSet, 0);
		llvm::BranchInst::Create(mergeBlock, llvmBasicBlock);

		/***
		 * Fill out the error block
		 */
		this->llvmBasicBlock = integerVectorErrorBasicBlock;
		setIntegerVectorPropertyCall(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
		llvm::BranchInst::Create(mergeBlock, llvmBasicBlock);

		/***
		 * Merge
		 */
		this->llvmBasicBlock = mergeBlock;

		//setIntegerVectorPropertyCall(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
	}

	void LlvmIRGenerator::setIntegerVectorPropertyCall(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		Traits* objectTraits = setPropertyInstruction->objectTraits;
		TessaTypes::Type* keyType = setPropertyInstruction->getPropertyKey()->getType();
		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->getType();
		TessaTypes::Type* objectType = setPropertyInstruction->objectType;

		const LlvmCallInfo* llvmCallInfo;
		if (objectTraits == VECTORINT_TYPE) {
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(IntVectorObject_SetNativeUIntProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(IntVectorObject_SetNativeIntProperty); 
			}
		} else {
			TessaAssert(objectTraits != VECTORINT_TYPE);
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(UIntVectorObject_SetNativeUIntProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(UIntVectorObject_SetNativeIntProperty);
			}
		}
	
		TessaAssert(receiverObject->getType()->isPointerTy());
		createLlvmCallInstruction(llvmCallInfo, 3, receiverObject, llvmIndex, valueToSet);
	}

	/*** 
	 * The value we are setting is either a signed or unsigned integer.
	 * The object we are setting is either an integer or unsigned integer vector
	 * The index is either an integer or unsigned integer.
	 */
	void LlvmIRGenerator::setIntegerVectorProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		Traits* valueTraits = setPropertyInstruction->valueTraits;
		Traits* objectTraits = setPropertyInstruction->objectTraits;
		
		TessaTypes::Type* keyType = setPropertyInstruction->getPropertyKey()->getType();
		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->getType();
		TessaTypes::Type* receiverType = setPropertyInstruction->getReceiverInstruction()->getType();
		//TessaTypes::Type* objectType = setPropertyInstruction->objectType;
		AvmAssert(receiverType->isVector());
		AvmAssert(receiverObject->getType()->isPointerTy());

		//if ((valueTraits == INT_TYPE) || (valueTraits == UINT_TYPE)) {
		if (valueType->isInteger() || valueType->isUnsignedInt()) {
			AvmAssert(keyType->isInteger() || keyType->isUnsignedInt() || keyType->isNumber());
			/*** 
			* Bug where keytype is number but the llvm is actually integer
			*/
			if (keyType->isNumber()) {
				AvmAssert(isAtomType(llvmIndex));
				keyType = _tessaTypeFactory->integerType();
			}

			valueToSet = castToNative(valueToSet, valueType, _tessaTypeFactory->integerType());
			llvmIndex = castToNative(llvmIndex, keyType, _tessaTypeFactory->integerType());
			setIntegerVectorPropertyInline(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
		} else {
			AvmAssert(valueType->isNumeric() || valueType->isAnyType());
			AvmAssert(keyType->isNumeric());
			AvmAssert(llvmIndex->getType()->isIntegerTy());

			valueToSet = castToNative(valueToSet, valueType, _tessaTypeFactory->integerType());
			llvmIndex = castToNative(llvmIndex, keyType, _tessaTypeFactory->integerType());
			setIntegerVectorPropertyInline(setPropertyInstruction, receiverObject, llvmIndex, valueToSet, isUnsignedInteger);
			//TessaAssert(false);
		}
	}

	/***
	 * The object we are setting is a double vector type
	 * The index is an integer
	 * The value to set mnust be a double
	 */
	void LlvmIRGenerator::setDoubleVectorProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		Traits* valueTraits = setPropertyInstruction->valueTraits;
		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->getType();
		TessaAssert(valueToSet->getType()->isDoubleTy());

		if (valueType == _tessaTypeFactory->numberType()) {
			TessaAssert(valueTraits == NUMBER_TYPE); 
			const LlvmCallInfo* llvmCallInfo;
			if (isUnsignedInteger) {
				llvmCallInfo = GET_LLVM_CALLINFO(DoubleVectorObject_SetNativeUIntProperty);
			} else {
				llvmCallInfo = GET_LLVM_CALLINFO(DoubleVectorObject_SetNativeIntProperty);
			}

			createLlvmCallInstruction(llvmCallInfo, 3, receiverObject, llvmIndex, valueToSet);
		} else {
			TessaAssert(false);
		}
	}

	/***
	 * The index must be an integer, but the object and value to set can be anything
	 */
	void LlvmIRGenerator::setLateBoundIntegerProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger) {
		TessaTypes::Type* receiverType = setPropertyInstruction->getReceiverInstruction()->getType();
		TessaTypes::Type* valueType = setPropertyInstruction->getValueToSet()->getType();

		receiverObject = nativeToAtom(receiverObject, receiverType);
		valueToSet = nativeToAtom(valueToSet, valueType);

		const LlvmCallInfo* llvmCallInfo;
		if (isUnsignedInteger) {
			llvmCallInfo = GET_LLVM_CALLINFO(MethodEnvSetPropertyLateInteger);	
		} else {
			llvmCallInfo = GET_LLVM_CALLINFO(MethodEnvSetPropertyLateUnsignedInteger);
		}

		createLlvmCallInstruction(llvmCallInfo, 4, _envPointer, receiverObject, llvmIndex, valueToSet);
	}

    void LlvmIRGenerator::callSetCacheHandler(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* valueToSet) {
		TessaAssert(receiverObjectAtom->getType()->isIntegerTy());	// Receiver object has to be boxed into an atom
		TessaAssert(valueToSet->getType()->isIntegerTy());	// Receiver object has to be boxed into an atom

		SetCache* cacheSlot = setPropertyInstruction->setCacheSlot;
		TessaAssert(cacheSlot != NULL);

		// Call void setprop_miss(SetCache& c, Atom obj, Atom val, MethodEnv* env)
		llvm::Value* cacheAddress = createConstantPtr((intptr_t) cacheSlot);
		llvm::Value* cacheHandler = createLlvmLoadPtr(cacheAddress, setPropertyInstruction->setCacheHandlerOffset / sizeof(int32_t));
		const llvm::FunctionType* setCacheFunctionType = getSetCacheHandlerFunctionType();
		int argCount = 4;
		createLlvmIndirectCallInstruction(cacheHandler, setCacheFunctionType, "SetPropertyCache", argCount, 
			cacheAddress, receiverObjectAtom, valueToSet, _envPointer);
	}

	void LlvmIRGenerator::visit(SetPropertyInstruction* setPropertyInstruction) {
		const Multiname* propertyName = setPropertyInstruction->getPropertyName();
		llvm::Value* llvmPropertyName = createConstantPtr((intptr_t)propertyName);
		TessaInstruction* key = setPropertyInstruction->getPropertyKey();

		TessaInstruction* tessaValue = setPropertyInstruction->getValueToSet();
		TessaTypes::Type* valueType = tessaValue->getType();
		llvm::Value* valueToSet = getValue(tessaValue);

		TessaInstruction* receiverInstruction = setPropertyInstruction->getReceiverInstruction();
		TessaTypes::Type* receiverType = receiverInstruction->getType();
		llvm::Value* receiverObject = getValue(receiverInstruction);

		llvm::Value* llvmKey = _nullObjectAtom;
		if (key != NULL) {
			llvmKey = getValue(key);	
		} 

		earlyBindSetProperty(setPropertyInstruction, llvmPropertyName, receiverObject, llvmKey, valueToSet);
	}

	void LlvmIRGenerator::visit(InitPropertyInstruction* initPropertyInstruction) {
		this->visit(reinterpret_cast<SetPropertyInstruction*>(initPropertyInstruction));
		/*
		TessaInstruction* receiverInstruction = initPropertyInstruction->getReceiverInstruction();
		llvm::Value* receiverObject = getValue(receiverInstruction);
		llvm::Value* receiverObjectAtom = nativeToAtom(receiverObject, receiverInstruction->getType());

		llvm::Value* valueToInitializeAtom = nativeToAtom(getValue(initPropertyInstruction->getValueToInit()), initPropertyInstruction->getValueToInit()->getType());
		llvm::Value* vtable = loadVTable(receiverObject, receiverInstruction->getType());
		llvm::Value* multiname = initMultiname(initPropertyInstruction->getPropertyName(), initPropertyInstruction->getNamespaceInstruction());
		createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvInitProperty), 5, _envPointer, receiverObjectAtom, multiname, valueToInitializeAtom, vtable);
		*/
	}

	void LlvmIRGenerator::visit(GetSlotInstruction* getSlotInstruction) {
		TessaInstruction* tessaReceiverObject = getSlotInstruction->getReceiverObject();
		TessaTypes::Type* receiverType = tessaReceiverObject->getType();
		TessaTypes::Type* slotType = getSlotInstruction->getType();

		llvm::Value* receiverObject = getValue(tessaReceiverObject);
		llvm::Value* receiverObjectPtr = castToNative(receiverObject, receiverType, _tessaTypeFactory->scriptObjectType());
		llvm::Value* slotIndex = createConstantInt(getSlotInstruction->getSlotNumber());

		llvm::Value* slotResultNative;
		if (slotType == _tessaTypeFactory->numberType()) {
			llvm::Value* doublePointer = createBitCast(receiverObjectPtr, _doublePointerType);
			slotResultNative = createLlvmLoadDouble(doublePointer, getSlotInstruction->getSlotOffset() / sizeof(double));
		} else {
			slotResultNative = createLlvmLoad(receiverObjectPtr, getSlotInstruction->getSlotOffset() / sizeof(int32_t));
			/***
			 * At this point LLVM thinks we have an integer because loads are ints. 
			 * We have to create an llvm cast to the correct type
			 */
			if (slotType->isBoolean()) {
				slotResultNative = castToNative(slotResultNative, _tessaTypeFactory->integerType(), _tessaTypeFactory->boolType());
			} else if (slotType->isPointer()) {
				slotResultNative = castIntToPtr(slotResultNative);
			}
		}

		putValue(getSlotInstruction, slotResultNative);
	}

	bool LlvmIRGenerator::needsWriteBarrier(Traits* slotTraits) {
		/***
		 * Object and string types need a write barrier
		 */
		return (!slotTraits || !slotTraits->isMachineType() || slotTraits== OBJECT_TYPE);
	}

	void LlvmIRGenerator::setSlotWithWriteBarrier(Traits* slotTraits, SetSlotInstruction* setSlotInstruction) {
		llvm::Value* slotNumber = createConstantInt(setSlotInstruction->getSlotNumber());
		TessaInstruction* tessaReceiverObject = setSlotInstruction->getReceiverObject()->resolve();
		TessaTypes::Type* receiverType = tessaReceiverObject->resolve()->getType();
		llvm::Value* receiverObject = getValue(tessaReceiverObject);

		llvm::Value* gcPointer = createConstantPtr((intptr_t)core->gc);
		TessaAssert(receiverType->isScriptObject()); 
		TessaInstruction* tessaValueToSet = setSlotInstruction->getValueToSet();
		TessaTypes::Type* valueType = tessaValueToSet->getType();
		llvm::Value* valueToSet = getValue(tessaValueToSet);
		valueToSet = castToNative(valueToSet, valueType, setSlotInstruction->getType());

		llvm::Value* receiverObjectPtr = receiverObject;
		llvm::Value* receiverObjectPtrInt = castPtrToInt(receiverObject); 
		llvm::Value* writeAddress = llvm::BinaryOperator::Create(llvm::Instruction::Add, receiverObjectPtrInt, createConstantInt(setSlotInstruction->getSlotOffset()), "", llvmBasicBlock);
		writeAddress = castIntToPtr(writeAddress);
				
		// use fast atom wb
        if (slotTraits == NULL || slotTraits == OBJECT_TYPE) {
			// Objects use a write barrier
			TessaAssert(setSlotInstruction->isObject() || setSlotInstruction->isAny());
			createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreAtomWriteBarrier), 4, gcPointer, receiverObjectPtr, writeAddress, valueToSet);
		} else {
			// Strings use this write barrier. Strings are represented as pure pointers in their native type Stringp*
			createLlvmCallInstruction(GET_LLVM_CALLINFO(MMgcPrivateWriteBarrierRC), 4, gcPointer, receiverObjectPtr, writeAddress, valueToSet);
		}
	}

	void LlvmIRGenerator::visit(SetSlotInstruction* setSlotInstruction) {
		TessaInstruction* tessaReceiverObject = setSlotInstruction->getReceiverObject();
		TessaTypes::Type* slotType = setSlotInstruction->getType();
		TessaTypes::Type* receiverType = tessaReceiverObject->resolve()->getType();
		llvm::Value* receiverObject = getValue(tessaReceiverObject);
		if (receiverType->isAnyType()) {
			receiverObject = AtomToScriptObject(receiverObject);
		}

		TessaAssert(receiverType->isAnyType() || receiverType->isScriptObject()); 
		TessaInstruction* tessaValueToSet = setSlotInstruction->getValueToSet();
		TessaTypes::Type* valueType = tessaValueToSet->getType();
		llvm::Value* valueToSet = getValue(tessaValueToSet);
		valueToSet = castToNative(valueToSet, valueType, slotType);

		/***
		 * Base pointers in LLVM are typed. In order to correctly create a store for ActionSCript Number type
		 * We have to cast the base pointer to the appropriate types that can handle native string and number types.
		 * For ints and pointers, the standard pointer type will do.
		 */
		Traits* slotTraits = setSlotInstruction->getSlotTraits();
		if (needsWriteBarrier(slotTraits)) {
			setSlotWithWriteBarrier(slotTraits, setSlotInstruction);
		} else if (slotType == _tessaTypeFactory->numberType()) {
			llvm::Value* doubleReceiverPointer = createBitCast(receiverObject, _doublePointerType);
			createLlvmStore(doubleReceiverPointer, valueToSet, setSlotInstruction->getSlotOffset() / sizeof(double));
		} else {
			if (valueType->isBoolean()) {
				valueToSet = llvm::ZExtInst::Create(llvm::Instruction::ZExt, valueToSet, _intType, "", llvmBasicBlock);
			}
			createLlvmStore(receiverObject, valueToSet, setSlotInstruction->getSlotOffset() / sizeof(int32_t));
		}
	}

	void LlvmIRGenerator::visit(NewArrayInstruction* newArrayInstruction) {
		bool isSigned = false;
		List<TessaInstruction*, LIST_GCObjects>* arrayElements = newArrayInstruction->getArrayElements();
		llvm::Value* argCount = createConstantInt(newArrayInstruction->numberOfElements());
		TessaAssert(newArrayInstruction->numberOfElements() <= (int)maxArgPointerSize);

		for (uint32_t i = 0; i < arrayElements->size(); i++) {
			TessaInstruction* arrayElement = arrayElements->get(i);
			TessaTypes::Type* elementType = arrayElement->getType();

			llvm::Value* atomValue = nativeToAtom(getValue(arrayElement), elementType);
			atomValue = castIntToPtr(atomValue);
			createLlvmStore(_allocedArgumentsPointer, atomValue, i);
		}

		llvm::Value* newArrayScriptObject = createLlvmCallInstruction(GET_LLVM_CALLINFO(newarray), 3, _toplevelPointer, argCount, _allocedArgumentsPointer); 
		llvm::Value* castArrayObjectToPointerType = createBitCast(newArrayScriptObject, _pointerType);
		putValue(newArrayInstruction, castArrayObjectToPointerType);
	}

	void LlvmIRGenerator::visit(NextNameInstruction* nextNameInstruction) {
		TessaAssert(false);
	}

	void LlvmIRGenerator::visit(PushScopeInstruction* pushScopeInstruction) {
		TessaInstruction* tessaScopeObject = pushScopeInstruction->getScopeObject();
		TessaAssert(tessaScopeObject->isObject() || tessaScopeObject->isPointer());
		llvm::Value* scopeObject = getValue(tessaScopeObject);

		if (tessaScopeObject->isObject()) {
			TessaAssert(scopeObject->getType()->isIntegerTy());
			scopeObject = castIntToPtr(scopeObject);
		} 

		TessaAssert(scopeObject->getType()->isPointerTy());
		TessaAssert(_scopeStack != NULL);
		llvm::Value* scopeStackLocation = llvm::GetElementPtrInst::Create(_scopeStack, _currentScopeDepth, "PushScope", llvmBasicBlock);
		new llvm::StoreInst(scopeObject, scopeStackLocation, llvmBasicBlock);
		_currentScopeDepth = llvm::BinaryOperator::Create(llvm::Instruction::Add, _currentScopeDepth, createConstantInt(1), "", llvmBasicBlock);
	}

	void LlvmIRGenerator::visit(PopScopeInstruction* popScopeInstruction) {
		TessaAssert(false);
	}

	llvm::Value* LlvmIRGenerator::getScopeObject(int scopeIndex) {
		int offset = offsetof(ScopeChain, _scopes) + (scopeIndex * sizeof(Atom));
		offset = offset / sizeof(Atom);
		llvm::Value* scopeObject = createLlvmLoad(_methodEnvScopeChain, offset);
		return AtomToScriptObject(scopeObject);
	}

	void LlvmIRGenerator::visit(GetScopeObjectInstruction* getScopeObjectInstruction) {
		int32_t scopeIndex = getScopeObjectInstruction->getScopeIndex();
		llvm::Value* scopeObject;

		if (getScopeObjectInstruction->isOuterScope()) {
			scopeObject = getScopeObject(scopeIndex);
		} else {
			TessaAssert(_scopeStack != NULL);
			scopeObject = createLlvmLoadPtr(_scopeStack, scopeIndex);
		}
		
		TessaAssert(scopeObject != NULL);
		putValue(getScopeObjectInstruction, scopeObject);
	}

	void LlvmIRGenerator::visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) {
		llvm::Value* globalScope = NULL;
		const ScopeTypeChain* scope = methodInfo->declaringScope();
        int captured_depth = scope->size;
		globalScope = createLlvmLoadPtr(_scopeStack, 0);

		if (captured_depth > 0) {
			globalScope = getScopeObject(0);
		} else {
			globalScope = createLlvmLoadPtr(_scopeStack, 0);
		}

		/*** 
		 * BOth return atoms but should be script objects
		 */
		TessaAssert(globalScope != NULL);
		putValue(getGlobalScopeInstruction, globalScope);
	}

	void LlvmIRGenerator::visit(WithInstruction* withInstruction) {
		TessaAssert(false);
	}

	void LlvmIRGenerator::visit(TypeOfInstruction* typeOfInstruction) {
		TessaAssert(typeOfInstruction->isLateCheck());
		TessaInstruction* tessaValue = typeOfInstruction->getObjectToTest();
		llvm::Value* valueToCheckAtom = nativeToAtom(getValue(tessaValue), tessaValue->getType());

		TessaInstruction* tessaTypeToCompare = typeOfInstruction->getTypeToCompare();
		llvm::Value* typeToCompareAtom = nativeToAtom(getValue(tessaTypeToCompare), tessaTypeToCompare->getType());

		llvm::Value* traits = createLlvmCallInstruction(GET_LLVM_CALLINFO(ToplevelToClassITraits), 2, _toplevelPointer, typeToCompareAtom);
		llvm::Value* result = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreIsTypeAtom), 2, valueToCheckAtom, traits);
		putValue(typeOfInstruction, result);
	}

	/***
	 * Has next 2 modifies many values at once. We have to load/store the object iterator values explicitly
	 * at each iteration :(
	 */
	void LlvmIRGenerator::visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) {
/*
		HasMorePropertiesRegisterInstruction* registerIndex = hasMorePropertiesInstruction->getRegisterIndex(); 
		llvm::Value* indexValue = getValue(registerIndex->getRegisterInstruction());
		llvm::Value* indexInteger = createLlvmCallInstruction(GET_LLVM_CALLINFO(AvmCoreInteger), 1, indexValue);
		llvm::Value* hasNextResult;

		if (hasMorePropertiesInstruction->modifiesObject()) {
			// AVM Opcode hasnext2
			HasMorePropertiesObjectInstruction* hasMorePropertiesObjectInstruction = hasMorePropertiesInstruction->getObjectIndex();
			TessaInstruction* objectIterator = hasMorePropertiesObjectInstruction->getObjectInstruction();
			llvm::Value* object = getValue(objectIterator);

			hasNextResult = env->hasnextproto(objectAtom, indexInteger) ? trueAtom : falseAtom;
			putValue(hasMorePropertiesObjectInstruction, object);
		} else {
			// This is for AVMOpcode hasnext
			TessaAssert(false);
		}

		// This is changed in hasNextProto
		indexAtom = core->intToAtom(indexInteger);
		putOperand(registerIndex, indexAtom);
		putOperand(hasMorePropertiesInstruction, hasNextResult);
		*/
		TessaAssert(false);
	}

	void LlvmIRGenerator::visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesIndexInstruction) {
		/***
		 * Don't do anything for this. The hasMorePropertiesInstruction needs to set the value for this.
		 * Read in hasMoreProperties header file to find out. Also look at the hasnext2 AVM opcode.
		 */
	}

	void LlvmIRGenerator::visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) {
		/***
		 * Don't do anything for this. The hasMorePropertiesInstruction needs to set the value for this.
		 * Read in hasMoreProperties header file to find out. Also look at the hasnext2 AVM opcode.
		 */
	}

	void LlvmIRGenerator::visit(SwitchInstruction* switchInstruction) {
		int numberOfCases = switchInstruction->numberOfCases() - 1;
		TessaInstruction* switchValue = switchInstruction->getSwitchValue()->resolve();
		llvm::Value* llvmSwitchValue = getValue(switchValue);
		TessaAssert(switchValue->isInteger());

		llvm::BasicBlock* defaultBasicBlock = getLlvmBasicBlock(switchInstruction->getDefaultCase()->getTargetBlock());
		llvm::SwitchInst* llvmSwitchInstruction = llvm::SwitchInst::Create(llvmSwitchValue, defaultBasicBlock, numberOfCases, llvmBasicBlock);

		for (int i = 0; i < numberOfCases; i++) {
			CaseInstruction* caseInstruction = switchInstruction->getCase(i);
			TessaInstruction* caseValue = caseInstruction->getCaseValue();
			// llvm can only switch on constant ints.
			TessaAssert(caseValue->isConstantValue());
			ConstantValueInstruction* constantValue = (ConstantValueInstruction*) caseValue;
			TessaTypes::ConstantInt* constantInt = (TessaTypes::ConstantInt*) (constantValue->getConstantValue());
			llvm::ConstantInt* llvmCaseValue = createConstantInt(constantInt->getValue());

			llvm::BasicBlock* caseBasicBlock = getLlvmBasicBlock(caseInstruction->getTargetBlock());
			llvmSwitchInstruction->addCase(llvmCaseValue, caseBasicBlock);
		}
		
		putValue(switchInstruction, llvmSwitchInstruction);
	}

	void LlvmIRGenerator::visit(CaseInstruction* caseInstruction) {
	}

	void LlvmIRGenerator::visit(NewActivationInstruction* newActiviationInstruction) {
		llvm::Value* newActivation = createLlvmCallInstruction(GET_LLVM_CALLINFO(MethodEnvNewActivation), 1, _envPointer);
		newActivation = createBitCast(newActivation, _pointerType);
		putValue(newActiviationInstruction, newActivation);
	}

	void LlvmIRGenerator::adjustScopeStackPointerForNewMethod(int callerScopeSize)  {
		llvm::Value* newScopeStack = castPtrToInt(_scopeStack);
		llvm::Value* scopeAdjustment = llvm::BinaryOperator::Create(Instruction::Mul, createConstantInt(callerScopeSize), createConstantInt(sizeof(Atom)), "inline scope adjustment", llvmBasicBlock);
		newScopeStack = llvm::BinaryOperator::Create(Instruction::Add, newScopeStack, scopeAdjustment, "inline begin newScopeStack", llvmBasicBlock);
		newScopeStack = castIntToPtr(newScopeStack);
		newScopeStack = createBitCast(newScopeStack, _scopeStack->getType());

		_scopeStackCallStack->add(_scopeStack);
		_scopeStackCountStack->add(_currentScopeDepth);

		_scopeStack = newScopeStack;
		_currentScopeDepth = createConstantInt(0);
		TessaAssert(_scopeStack->getType()->isPointerTy());
	}

	void LlvmIRGenerator::visit(InlineBeginInstruction* inlineBeginInstruction) {
		int methodId = inlineBeginInstruction->getmethodId();
		LoadVirtualMethodInstruction* loadedVirualMethod = inlineBeginInstruction->getLoadedMethod();
		llvm::Value* loadedMethodEnv = getValue(loadedVirualMethod->getLoadedMethodEnv());
		llvm::Value* loadedMethodInfo = getValue(loadedVirualMethod->getLoadedMethodInfo());

		_envPointer = loadedMethodEnv;
		setMethodEnvPointers();
		adjustScopeStackPointerForNewMethod(inlineBeginInstruction->_callerScopeSize);
		_methodInfoStack->add(methodInfo);
		this->methodInfo = inlineBeginInstruction->_inlinedMethod;
	}
	/***
	 * replicate MethodFrame ctor inline
	 */
	void LlvmIRGenerator::createPrologue(MethodInfo* methodInfo) {
		TessaAssert(_envPointer != NULL);
		int methodFrameOffset = offsetof(AvmCore, currentMethodFrame) / sizeof(int32_t);	// Calculated by offsetof(AvmCore, currentMethodFrame); but not a friend class of avmcore
		int envOrCodeContextOffset = offsetof(MethodFrame, envOrCodeContext) / sizeof(int32_t);	// Calculated by offsetof(MethodFrame, envOrCodeContexT);
		int nextOffset = offsetof(MethodFrame, next);				// Calculated by offsetof(MethodFrame, next);

		llvm::Value* currentMethodFrame = createLlvmLoad(_avmcorePointer, methodFrameOffset); 
		_methodFrame = new llvm::AllocaInst(_pointerType, createConstantInt(sizeof(MethodFrame)), "MethodFrame", llvmBasicBlock);
		createLlvmStorePointer(_methodFrame, _envPointer, envOrCodeContextOffset);
		createLlvmStorePointer(_methodFrame, castIntToPtr(currentMethodFrame), nextOffset);

		// LLVM thinks Avmcore is a pointer to integers
		createLlvmStoreInt(_avmcorePointer, currentMethodFrame, methodFrameOffset);
	}

	void LlvmIRGenerator::createEpilogue() {
		/***
		 * Replicate MethodFrame dtor inline
		 */
		int nextOffset = offsetof(MethodFrame, next) / sizeof(int32_t);				// Calculated by offsetof(MethodFrame, next);
		int methodFrameOffset = offsetof(AvmCore, currentMethodFrame) / sizeof(int32_t);	// Calculated by offsetof(AvmCore, currentMethodFrame); but not a friend class of avmcore

		llvm::Value* nextMethodFrame = createLlvmLoad(_methodFrame, nextOffset);
		createLlvmStoreInt(_avmcorePointer, nextMethodFrame, methodFrameOffset);
		/*
#ifdef DEBUG
		llvm::BasicBlock* lastBasicBlock = &(_llvmFunction->back());
		llvm::Instruction* lastInstruction = &(lastBasicBlock->back());
		lastInstruction->dump();
		TessaAssert(lastInstruction->isTerminator());

		llvm::Function* functionToCall = getLlvmFunction(GET_LLVM_CALLINFO(MethodEndSymbol));
		std::vector<llvm::Value*> arguments;
		llvm::CallInst* callInstruction = CallInst::Create(functionToCall, arguments.begin(), arguments.end(), "", lastInstruction);
#endif
		*/
	}



	
}
