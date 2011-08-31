
namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;

	class TessaInterpreter : public TessaVisitorInterface { 
	private:
		// Data
		avmplus::AvmCore* core;
		avmplus::PoolObject* poolObject;
		MMgc::GC* gc;
		MethodEnv* env;
		avmplus::HeapHashtable values;
		int*	branchTable;

		int		flowingFromBasicBlock;
		int		currentBasicBlockId;
		int		targetBlock;
		ASFunction*	currentFunction;
		Atom	_returnValue;
		List<TessaInstruction*, avmplus::LIST_GCObjects>* scopeStack;

	private:
		// Methods
		void putOperand(TessaValue* tessaValue, Atom value);
		Atom getOperand(TessaValue* tessaValue);
		uint32_t toUint32(Atom value);
		bool atomCanBeUint32(Atom atom);
		bool isInteger(Atom atom);
		Atom doDoubleBinaryOp(TessaBinaryOp opcode, Atom leftOperand, Atom rightOperand); 
		Atom atomPointer(void* pointer);
		void jumpToTarget(BasicBlock* targetBlock);
		void mapParametersToArguments(BasicBlock* firstBasicBlock, int argCount, Atom* arguments);
		void initOptionalArguments(BasicBlock* firstBasicBlock); 

		bool isDone;
		void interpretBasicBlock(BasicBlock* basicBlock); 
		Atom interpretBitwiseOp(TessaBinaryOp opcode, Atom leftOperand, Atom rightOperand);
		Atom inverseBooleanAtom(Atom atom);
		Atom* createArguments(List<TessaInstruction*, avmplus::LIST_GCObjects>* arguments);
		Atom* createArguments(ArrayOfInstructions* arguments); 

		void	pushScope(TessaInstruction* scopeObject);
		int		getScopeDepth();
		TessaInstruction* popScope();
		TessaInstruction* peekScope(uint32_t index);

		Traits* getTraits(const Multiname* name, PoolObject* pool, Toplevel* toplevel, avmplus::AvmCore* core);

		Atom	executeDynamicCallInstruction(CallInstruction* callInstruction, Atom* arguments, int argCount);
		Atom	executeCallInstruction(CallInstruction*	callInstruction, Atom* arguments, int argCount);
		Atom	executeAbcCallInstruction(CallInstruction* callInstruction, Atom* arguments, int argCount);
		Atom	coerceAtomToOtherAtomType(TessaTypes::Type* typeToCoerce, Atom instructionValue);

	public:
		TessaInterpreter(avmplus::AvmCore* core, avmplus::PoolObject* poolObject);
		virtual ~TessaInterpreter();
		Atom interpret(MethodEnv* env, int32_t argc, Atom* args, ASFunction* function); 

		void visit(TessaInstruction* tessaInstruction); 
		void visit(FindPropertyInstruction* findPropertyInstruction); 
		void visit(ConstantValueInstruction* constantValueInstruction);
		void visit(ArrayOfInstructions* arrayOfInstructions); 
		void visit(NewObjectInstruction* newObjectInstruction); 
		void visit(ConstructInstruction* constructInstruction);
		void visit(ConstructPropertyInstruction* constructPropertyInstruction);
		void visit(ConstructSuperInstruction* constructSuperInstruction); 
		void visit(CallVirtualInstruction* callVirtualInstruction);
		void visit(CallStaticInstruction* callStaticInstruction); 
		void visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction); 
		void visit(CallInstruction* callInstruction); 
		void visit(CallSuperInstruction* callSuperInstruction);
		void visit(CallPropertyInstruction* callPropertyInstruction); 
		void visit(ReturnInstruction* returnInstruction); 
		void visit(BinaryInstruction* binaryInstruction); 
		void visit(ConditionInstruction* conditionInstruction); 
		void visit(UnaryInstruction* unaryInstruction); 
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
		void visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesRegisterInstruction); 
		void visit(SwitchInstruction* switchInstruction); 
		void visit(CaseInstruction* caseInstruction); 
		void visit(NewActivationInstruction* newActivationInstruction); 
		void visit(CallInterfaceInstruction* callInterfaceInstruction); 
		void visit(InlineBeginInstruction* inlineBeginInstruction);

		
	};
}