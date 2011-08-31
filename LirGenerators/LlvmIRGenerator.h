
namespace LlvmCodeGenerator {
	class LlvmCallInfo;
}

namespace LirGenerators {
	using namespace llvm;
	using namespace LlvmCodeGenerator;

	class LlvmIRGenerator : public TessaVisitorInterface {
	private:
		// Data from Tamarin Namespace
		avmplus::AvmCore* core;
		MMgc::GC* gc;
		MethodInfo* methodInfo;
		uint32_t maxArgPointerSize;
		TypeFactory* _tessaTypeFactory;

		// Any hashmap will work as long as it can take GC objects and map it to non GC objects. Couldn't find any other hashmap
		avmplus::GCSortedMap<TessaTypes::TessaValue*, llvm::Value*, avmplus::LIST_NonGCObjects>*	_tessaValueToLlvmValueMap;

		// This maps a Tessa Basic block -> the beginning of an LLVM Basic Block
		avmplus::GCSortedMap<TessaVM::BasicBlock*, llvm::BasicBlock*, avmplus::LIST_NonGCObjects>*	_tessaToLlvmBasicBlockMap;

		/***
		 * This maps a Tessa VM Basic Block -> end of an llvm basic block. End means after LLVM inlines something.
		 * For example, a TESSA VM Basic block may become multiple LLVM Basic Blocks. Assume LLVM inlines a method that has an if/else statement.
		 * The _tessaToLlvmBasicBlockMap maps the Tessa Block to the top of the if/else block. 
		 * The _tessaToEndLlvmBasicBlock maps the TEssa Block to the merge block of the if /else combo.
		 */
		avmplus::GCSortedMap<TessaVM::BasicBlock*, llvm::BasicBlock*, avmplus::LIST_NonGCObjects>*	_tessaToEndLlvmBasicBlockMap;

		/***
		 * When we inline a method, we need to have a new env pointer
		 */
		avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>* _methodEnvCallStack;
		avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>* _methodEnvScopeChainStack;
		avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>* _methodEnvVTableStack;
		avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>* _toplevelPointerStack;
		avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>* _scopeStackCallStack;
		avmplus::List<llvm::Value*, avmplus::LIST_NonGCObjects>* _scopeStackCountStack;
		avmplus::List<MethodInfo*, avmplus::LIST_GCObjects>* _methodInfoStack;


		// Data from LLVM
		llvm::Module* _module;
		llvm::Function* _llvmFunction;
		llvm::LLVMContext* context;
		llvm::BasicBlock* llvmBasicBlock;

		// Commonly used Data pointers
		llvm::Value* _envPointer;
		llvm::Value* _toplevelPointer;
		llvm::Value* _avmcorePointer;
		llvm::Value* _falseAtom;
		llvm::Value* _trueAtom;
		llvm::Value* _undefinedAtom;
		llvm::Value* _nullObjectAtom;
		llvm::Value* _methodFrame;

		llvm::Value* _scopeStack;
		llvm::Value* _currentScopeDepth;
		llvm::Value* _allocedArgumentsPointer;

		// Things that should only be loaded once
		llvm::Value* _methodEnvScopeChain;
		llvm::Value* _methodEnvVTable;

		TessaVM::BasicBlock* currentBasicBlock;

		// Usuable types
		const llvm::IntegerType* _atomType;
		const llvm::PointerType* _pointerType; 
		const llvm::PointerType* _doublePointerType;
		const llvm::PointerType* _booleanPointerType;
		const llvm::ArrayType*	 _argumentsType;
		const llvm::IntegerType* _intType;
		const llvm::Type*		 _doubleType;
		const llvm::IntegerType* _uintType;
		const llvm::IntegerType* _booleanType;
		const llvm::Type*		 _voidType;
		int _returnCount;

	private:
		// Methods
		void putValue(TessaValue* tessaValue, llvm::Value* llvmValue);
		bool containsLlvmValue(TessaValue* tessaValue);
		llvm::Value* getValue(TessaValue* tessaValue);
		llvm::Function* getLlvmFunction(const LlvmCallInfo* llvmCallInfo);
		bool		isUndefinedOrNull(llvm::Value* value);

		// Lllvm helper methods
		llvm::ConstantInt*	createConstantInt(int integerValue);
		llvm::ConstantInt*	createConstantUnsignedInt(uint32_t integerValue); 
		llvm::ConstantInt*	createConstantBoolean(bool value);
		llvm::Constant*		createConstantDouble(double doubleValue);
		llvm::IntToPtrInst* castIntToPtr(llvm::Value* integer, std::string castName); 
		llvm::PtrToIntInst* castPtrToInt(llvm::Value* pointer, std::string castName); 
		llvm::IntToPtrInst*	createConstantPtr(intptr_t pointer);
		llvm::Instruction*		doPointerArithmetic(llvm::Value* pointer, int offset);
		llvm::Instruction*		createLlvmLoad(llvm::Value* pointer, int offset);
		llvm::Instruction*		createLlvmLoadDouble(llvm::Value* pointer, int offset);
		llvm::Instruction*		createLlvmLoadBoolean(llvm::Value* pointer, int offset);
		llvm::Instruction*		createLlvmLoad(llvm::Value* pointer, int offset, llvm::BasicBlock* basicBlockToInsert);
		llvm::Instruction*		createLlvmLoadPtr(llvm::Value* pointer, int offset);
		void				createLlvmStoreInt(llvm::Value* pointer, llvm::Value* valueToStore, int offset);
		void				createLlvmStorePointer(llvm::Value* pointer, llvm::Value* valueToStore, int offset);
		void				createLlvmStoreDouble(llvm::Value* pointer, llvm::Value* valueToStore, int offset);
		void				createLlvmStore(llvm::Value* pointer, llvm::Value* valueToStore, int offset); 

		const llvm::Type* getPointerType(TessaTypes::Type* tessaType); 

		llvm::Value* allocateArgumentsPointer(List<TessaVM::BasicBlock*, LIST_GCObjects>* basicBlocks);
		void insertConstantAtoms(TessaVM::ASFunction* asFunction); 

		void setMethodEnvPointers(); 
		void restoreMethodEnvPointers(); 
		void restoreScopeStackPointers();
		void adjustScopeStackPointerForNewMethod(int callerScopeSize); 

		void setParametersToUndefined(TessaVM::BasicBlock* firstBasicBlock);
		void mapParametersToValues(TessaVM::BasicBlock* firstBasicBlock);
		void buildJitTimeOptionalParameter(TessaInstruction* parameter, Atom defaultValue, int parameterIndex, int optionalIndex, int requiredCount, int displacement, TessaVM::BasicBlock* preOptionalBlock); 
		void initOptionalParameters(TessaVM::BasicBlock* firstBasicBlock);
		void copyRequiredParameters(TessaVM::BasicBlock* firstBasicBlock);
		void createRestParameters(TessaVM::BasicBlock* firstBasicBlock);
		void unboxParametersToNativeTypes(TessaVM::BasicBlock* firstBasicBlock);
		llvm::Argument* getArgCountParameter();
		llvm::Argument* getAtomArgumentsParameter();
		void	debugMessage(std::string message);

		void createLlvmBasicBlocks(ASFunction* function);
		llvm::BasicBlock* getLlvmBasicBlock(TessaVM::BasicBlock* tessaBasicBlock); 
		std::vector<llvm::Value*>* createLlvmArguments(ArrayOfInstructions* arguments); 

		// Helper instructions
		llvm::Value*	loadVTable(llvm::Value* object, TessaTypes::Type* type);
		const llvm::FunctionType*	getInterfaceInvokeFunctionType(TessaTypes::Type* interfaceReturnType);
		const llvm::FunctionType*	getInvokeFunctionType(TessaTypes::Type* callReturnType);
		const llvm::FunctionType*	getCallCacheHandlerFunctionType();
		const llvm::FunctionType*	getGetCacheHandlerFunctionType();
		const llvm::FunctionType*	getSetCacheHandlerFunctionType();
		llvm::Function*				getInvokeFunction(TessaTypes::Type* callReturnType);

		// All the various call instructions
		llvm::CallInst*		createLlvmIndirectCallInstruction(llvm::Value* indirectAddress, const llvm::FunctionType* functionType, std::string functionName, int argCount, ...);
		llvm::CallInst*		createLlvmCallInstruction(const LlvmCallInfo* llvmCallInfo, int argCount, ...);
		llvm::CallInst*		createLlvmCallInstruction(llvm::Value* valueToCall, const llvm::FunctionType* functionType, std::string functionName, std::vector<llvm::Value*>* arguments, AbiKind abiKind);
		void				setLlvmCallingConvention(llvm::CallInst*, AbiKind callingConvention);
		void				castCallArgumentsToDeclaredTypes(std::vector<llvm::Value*>* castedArgumentsLocation, const llvm::FunctionType* functionTypes, std::vector<llvm::Value*>* arguments); 

		void				checkPointerSizes();
		llvm::Value*		createTypedArgumentsPointer(ArrayOfInstructions* arrayOfInstructions, MethodSignaturep methodSignature, int argCount); 
		llvm::Value*		createAtomArgumentsPointer(ArrayOfInstructions* arrayOfInstructions);
		llvm::Value*		castReturnValue(CallInstruction* callInstruction, llvm::Value* returnValue); 
		
		llvm::Value*		executeAbcCallInstruction(CallInstruction* callInstruction, int argCount); 
		llvm::Value*		executeDynamicCallInstruction(CallInstruction* callInstruction, int argCount); 
		llvm::Value*		callMethodInfoImplGPR(llvm::Value* loadedMethodInfo, llvm::Value* loadedMethodEnv, llvm::Value* argCount, llvm::Value* argumentsPointer, TessaTypes::Type* resultType); 
		llvm::Value*		executeCallInstruction(TessaInstructions::CallInstruction* callInstruction, int argCount);
		llvm::Value*		executeNonInlinedCallInstruction(TessaInstructions::CallInstruction* callInstruction, int argCount); 
		llvm::Value*		loadMethodEnv(TessaInstruction* receiverObject, int methodId); 
		llvm::Value*		loadMethodInfo(llvm::Value* loadedMethodEnv); 

		llvm::Value*		callLateBoundConstructProperty(ConstructPropertyInstruction* constructPropertyInstruction);
		llvm::Value*		callEarlyBoundConstruct(ConstructInstruction* constructInstruction);
		llvm::Value*		callLateBoundCallSuper(CallSuperInstruction* callSuperInstruction);
		llvm::Value*		callEarlyBoundCallSuper(CallSuperInstruction* callSuperInstruction);

		llvm::Value*		callPropertyLateBound(CallPropertyInstruction* callPropertyInstruction);
		llvm::Value*		callPropertyWithCallCache(CallPropertyInstruction* callPropertyInstruction);
		llvm::Value*		callPropertySlotBound(CallPropertyInstruction* callPropertyInstruction);
		
		// Helper methods
		llvm::Value*		createBitCast(llvm::Value* valueToCast, const llvm::Type* typeToCastTo);
		llvm::Value*		initMultiname(const Multiname* multiname, TessaInstruction* namespaceInstruction, bool isDeleteProp); 
		llvm::Value*		copyMultiname(const Multiname* multiname); 
		void createPrologue(MethodInfo* methodInfo);
		void createEpilogue();

		// Specific opcode instructions
		llvm::Value*		compileIntegerBinaryOp(TessaInstructions::TessaBinaryOp binaryOp, llvm::Value* leftIntegerValue, llvm::Value* rightIntegerValue);
		llvm::Value*		compileDoubleBinaryOp(TessaInstructions::TessaBinaryOp binaryOp, llvm::Value* leftDoubleValue, llvm::Value* rightDoubleValue);
		llvm::Value*		compileBitwiseBinaryOp(TessaInstructions::TessaBinaryOp binaryOp, llvm::Value* leftInteger, llvm::Value* rightInteger); 
		llvm::Value*		compileUnsignedRightShift(TessaBinaryOp binaryOp, llvm::Value* uintLeft, llvm::Value* uintRight); 
		llvm::Value*		convertToNativeBoolean(llvm::Value* atomValue);
		llvm::Value*		getScopeObject(int scopeIndex); 
		bool				requiresNaNCheck(TessaBinaryOp opcode); 
		llvm::Instruction*	checkForNaNLlvm(llvm::Value* compareResult, llvm::Value* leftValue, llvm::Value* rightValue); 
		llvm::Instruction*	checkForNaNAtomLlvm(TessaBinaryOp conditionOpcode, llvm::Value* compareResult); 

		// Atom operations
		llvm::Value*		compileAtomBinaryOperation(TessaInstructions::BinaryInstruction* binaryInstruction);
		llvm::Value*		compileAtomCondition(TessaInstructions::ConditionInstruction* conditionInstruction);
		llvm::Value*		compileAtomUnaryOperation(TessaInstructions::UnaryInstruction* unaryInstruction);

		// Native operations
		llvm::Value*		compileTypedBinaryOperation(TessaInstructions::BinaryInstruction* binaryInstruction);
		llvm::Value*		compileTypedUnaryOperation(TessaInstructions::UnaryInstruction* unaryInstruction);
		llvm::Value*		compilePointerCompare(ConditionInstruction* conditionInstruction);

		llvm::Value*		compareAbsolutePointerAddresses(ConditionInstruction* conditionInstruction, llvm::Value* leftOperand, llvm::Value* rightOperand); 
		llvm::Value*		compileTypedCondition(TessaInstructions::ConditionInstruction* conditionInstruction);
		llvm::Value*		compileConditionNullCheck(TessaBinaryOp opcode, TessaValue* leftOperand, TessaValue* rightOperand);

		// Get, set, find property, helper early binding methods
		bool				isArrayElementAccess(TessaInstruction* receiverObject, TessaInstruction* key);
		bool				isArrayOrObjectType(TessaTypes::Type* tessaType, Traits* objectTraits);
		bool				isIntOrUIntVector(TessaTypes::Type* objectType, Traits* objectTraits);

		// Get property early bindigns
		llvm::Value*		getArrayOrVectorObjectIntegerIndex(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		getIntegerVectorPropertyInline(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		getIntegerVectorPropertyCall(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		getIntegerVectorProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		getDoubleVectorProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		getLateBoundIntegerProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		earlyBindGetIntegerIndexResult(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, bool isUnsignedInteger); 
		llvm::Value*		callGetCacheHandler(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObjectAtom);
		llvm::Value*		emitGetPropertySlow(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* llvmKey, llvm::Value* llvmPropertyName);
		llvm::Value*		earlyBindGetPropertyWithMultiname(GetPropertyInstruction* getPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* llvmKeyAtom, TessaTypes::Type* resultType);
		llvm::Value*		earlyBindGetProperty(GetPropertyInstruction* getPropertyInstruction, llvm::Value* llvmPropertyName, llvm::Value* receiverObject, llvm::Value* llvmIndex); 
		
		// Set property early bindings
		void	setArrayOrVectorObjectIntegerIndex(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	setIntegerVectorPropertyInline(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	setIntegerVectorPropertyCall(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	setIntegerVectorProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	setDoubleVectorProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	setLateBoundIntegerProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	earlyBindSetIntegerIndexResult(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmIndex, llvm::Value* valueToSet, bool isUnsignedInteger); 
		void	earlyBindSetIntegerIndexResult(Traits* indexTraits, Traits* objectTraits, Traits* valueTraits, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* valueToSet, TessaTypes::Type* valueType, TessaTypes::Type* receiverType, TessaTypes::Type* keyType); 

		void earlyBindSetUnsignedIntegerIndexResult(Traits* indexTraits, Traits* objectTraits, Traits* valueTraits, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* valueToSet, TessaTypes::Type* valueType); 
		void emitSetPropertySlow(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* valueToSet, llvm::Value* llvmPropertyName);
		void callSetCacheHandler(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* valueToSet);
		void setPropertyWithMultiname(SetPropertyInstruction* setPropertyInstruction, llvm::Value* receiverObjectAtom, llvm::Value* indexAtom, llvm::Value* valueToSetAtom);
		void earlyBindSetProperty(SetPropertyInstruction* setPropertyInstruction, llvm::Value* llvmPropertyName, llvm::Value* receiverObject, llvm::Value* llvmKey, llvm::Value* valueToSet); 

		// Boxing instructions
		TessaTypes::Type*	getTessaType(Traits* traits); 
		const llvm::Type*	getLlvmType(TessaTypes::Type* tessaType);
		llvm::Value*		nativeToAtom(llvm::Value* object, Traits* traits);
		llvm::Value*		nativeToAtom(llvm::Value* value, TessaTypes::Type* typeOfValue); 
		llvm::Value*		optimizeNativeIntegerToAtom(llvm::Value* intValue);
		llvm::Value*		atomToNative(llvm::Value* atom, TessaTypes::Type* typeToNative);
		llvm::Value*		castToNative(llvm::Value* native, TessaTypes::Type* nativeType, TessaTypes::Type* newNativeType); 

		llvm::Instruction*	AtomToScriptObject(llvm::Value* scriptObject);
		llvm::Instruction*	ScriptObjectToAtom(llvm::Value* scriptObject);

		llvm::Value*		castIntegerToNative(llvm::Value* integerValue, TessaTypes::Type* newNativeType);
		llvm::Value*		castBooleanToNative(llvm::Value* booleanValue, TessaTypes::Type* newNativeType);
		llvm::Value*		specializeCastFloatToInteger(llvm::Value* doubleValue);
		llvm::Value*		castFloatToNative(llvm::Value* doubleValue, TessaTypes::Type* newNativeType);
		llvm::Value*		castAnyToNative(llvm::Value* native, TessaTypes::Type* newNativeType);
		llvm::Value*		castScriptObjectToNative(llvm::Value* scriptObject, TessaTypes::Type* newNativeType);
		llvm::Value*		castAvmObjectToNative(llvm::Value* avmObject, TessaTypes::Type* newNativeType);
		llvm::Value*		castUndefinedToNative(llvm::Value* undefinedValue, TessaTypes::Type* newNativeType);
		llvm::Value*		castStringToNative(llvm::Value* stringValue, TessaTypes::Type* newNativeType);

		// Type hints
		bool			isLlvmPointerType(TessaTypes::Type* tessaType);
		bool			isLlvmIntegerType(TessaTypes::Type* tessaType);
		bool			isAtomType(llvm::Value* value);
		bool			isPointerType(llvm::Value* value);
		bool			isBoolType(llvm::Value* value);
		bool			hasType(TessaValue* tessaValue);
		bool			isInteger(TessaValue* tessaValue);
		bool			isVoidType(const llvm::Type* type);

		// Other
		void visitBlocksInReversePostOrder(ASFunction* function);
		void addLlvmPhiValues(List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks); 
		llvm::Value* insertCastInIncomingBlock(llvm::Value* castedIncomingValue, llvm::BasicBlock* incomingBlock, TessaTypes::Type* tessaPhiType, TessaTypes::Type* tessaIncomingType);
		llvm::Value* castUndefinedOperandInIncomingEdge(llvm::Value* castedIncomingValue, llvm::Instruction* incomingTerminator, TessaTypes::Type* tessaPhiType);
		llvm::Value* castPhiOperandInIncomingEdge(llvm::PHINode* phiNode, llvm::Value* llvmIncomingValue, llvm::BasicBlock* llvmIncomingBasicBlock, TessaTypes::Type* tessaPhiType, TessaTypes::Type* tessaIncomingType); 
		llvm::Value* emitFindDefinition(FindPropertyInstruction* findPropertyInstruction, llvm::Value* multinamePointer);
		bool		needsWriteBarrier(Traits* slotTraits);
		void		setSlotWithWriteBarrier(Traits* slotTraits, SetSlotInstruction* setSlotInstruction);

		// Early binding helpers
		llvm::Value* emitCallToNewInstance(llvm::Value* receiverObject, llvm::Value* instanceVtable); 
		llvm::Value* emitCallToMethodInfoImplgr(llvm::Value* instanceVtable, int argCount, llvm::Value* llvmArguments); 
		llvm::Value* loadFromSlot(llvm::Value* receiverObject, TessaTypes::Type* tessaType, int slotNumber, Traits* objectTraits); 
		bool requiredVoidCheck(Traits* resultType); 

	public:
		LlvmIRGenerator(avmplus::AvmCore* core, Module* module, llvm::Function* llvmFunction);
		virtual ~LlvmIRGenerator();
		void createLIR(MethodInfo* methodInfo, ASFunction* function); 

		void visit(TessaInstruction* tessaInstruction); 
		void visit(FindPropertyInstruction* findPropertyInstruction); 
		void visit(ConstantValueInstruction* constantValueInstruction);
		void visit(ArrayOfInstructions* arrayOfInstructions); 
		void visit(ConstructInstruction* constructInstruction);
		void visit(NewObjectInstruction* newObjectInstruction); 
		void visit(ConstructPropertyInstruction* constructPropertyInstruction);
		void visit(ConstructSuperInstruction* constructSuperInstruction); 
		void visit(CallInstruction* callInstruction); 
		void visit(CallVirtualInstruction* callVirtualInstruction);
		void visit(CallStaticInstruction* callStaticInstruction); 
		void visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction); 
		void visit(CallInterfaceInstruction* callInterfaceInstruction); 
		void visit(CallSuperInstruction* callSuperInstruction);
		void visit(CallPropertyInstruction* callPropertyInstruction); 
		void visit(ReturnInstruction* returnInstruction); 
		void visit(BinaryInstruction* binaryInstruction); 
		void visit(ConditionInstruction* conditionInstruction);
		void visit(TessaInstructions::UnaryInstruction* unaryInstruction); 
		void visit(ConditionalBranchInstruction* conditionalBranchInstruction); 
		void visit(UnconditionalBranchInstruction* unconditionalBranchInstruction); 
		void visit(PhiInstruction* phiInstruction);
		void visit(ParameterInstruction* parameterInstruction);
		void visit(CoerceInstruction* coerceInstruction);
		void visit(ConvertInstruction* convertInstruction);
		void visit(GetPropertyInstruction* getPropertyInstruction); 
		void visit(SetPropertyInstruction* setPropertyInstruction); 
		void visit(InitPropertyInstruction* initPropertyInstruction); 
		void visit(GetSlotInstruction* getSlotInstruction); 
		void visit(SetSlotInstruction* setSlotInstruction);
		void visit(NewArrayInstruction* newArrayInstruction); 
		void visit(NextNameInstruction* nextNameInstruction); 
		void visit(PushScopeInstruction* pushScopeInstruction); 
		void visit(PopScopeInstruction* popScopeInstruction);
		void visit(WithInstruction* withInstruction);
		void visit(TypeOfInstruction* typeOfInstruction); 
		void visit(GetScopeObjectInstruction* getScopeObjectInstruction);
		void visit(GetGlobalScopeInstruction* getGlobalScopeInstruction); 
		void visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction);
		void visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction); 
		void visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesIndexInstruction); 
		void visit(SwitchInstruction* switchInstruction); 
		void visit(CaseInstruction* caseInstruction); 
		void visit(NewActivationInstruction* newActiviationInstruction); 
		void visit(InlineBeginInstruction* inlineBeginInstruction);
		
		


	};
}
