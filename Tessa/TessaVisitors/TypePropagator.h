
namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;

	class TypePropagator : public TessaVisitorInterface { 
	private:
		// Data
		avmplus::AvmCore* _core;
		MMgc::GC* _gc;
		TypeFactory* _typeFactory;
		List<PhiInstruction*, LIST_GCObjects>* _workList;
		avmplus::GCSortedMap<TessaInstruction*, Type*, avmplus::LIST_GCObjects>*	_verifierTypes;
		List<BinaryInstruction*, LIST_GCObjects>* _specializedIntegerBinaryInstructions;
		bool _isDone;

	private:
		bool isDone;
		bool isInteger(TessaInstruction* instruction);
		bool canSpecializeIntegerBinaryOperation(CoerceInstruction* coerceInstruction);
		bool isCoercingToIntegerType(TessaValue* instruction);
		void specializeIntegerBinaryOperation(CoerceInstruction* coerceInstruction); 
		TessaValue* getIntegerOperandToBinary(TessaValue* binaryOperand); 
		bool isSpecializedBinaryInteger(BinaryInstruction* binaryInstruction);

		bool canPrecomputeBinaryOperation(BinaryInstruction* binaryInstruction);
		TessaInstruction* getDirectInstruction(TessaInstruction* instruction);
		TessaInstruction* getProperlyTypedDirectInstruction(TessaInstruction* instruction, Type* typeToFind);
		Type* getStrictestType(TessaInstruction* originalInstruction, List<TessaInstruction*, LIST_GCObjects>* operands);
		Type* getMostConcreteType(List<TessaValue*, LIST_GCObjects>* operands);
		TessaInstruction* findNumericOperand(BinaryInstruction* binaryInstruction, TessaInstruction* leftOperand);
		void setBinaryType(BinaryInstruction* binaryInstruction, Type* strictestType);
		bool isConstantIntOne(TessaInstruction* constantValue); 
		bool binaryOperandsAreCompatible(TessaBinaryOp op, Type* binaryType, Type* leftType, Type* rightType); 

		void propagateTypes(ASFunction* function);
		void initializeTypes(ASFunction* function);
		bool canChangeTypes(TessaInstruction* instruction);
		Type* getVerifierType(TessaInstruction* value);

		bool optimizeArrayGetAndSetProperty(PropertyAccessInstruction* propertyAccesss); 
		bool optimizeIntVectorGetAndSetProperty(PropertyAccessInstruction* propertyAccess);

		Type* conservativePhiType(PhiInstruction* phiInstruction);

	public:
		TypePropagator(avmplus::AvmCore* core, MMgc::GC* gc);
		void propagateTypeInformation(ASFunction* function);
		void resolvePhiTypesOnly(ASFunction* function);

		void visit(TessaInstruction* tessaInstruction); 
		void visit(FindPropertyInstruction* findPropertyInstruction); 
		void visit(ConstantValueInstruction* constantValueInstruction);
		void visit(ArrayOfInstructions* arrayOfInstructions); 
		void visit(NewObjectInstruction* newObjectInstruction); 
		void visit(ConstructInstruction* constructInstruction);
		void visit(ConstructPropertyInstruction* constructPropertyInstruction);
		void visit(ConstructSuperInstruction* constructSuperInstruction); 
		void visit(CallInstruction* callInstruction); 
		void visit(CallVirtualInstruction* callVirtualInstruction);
		void visit(CallStaticInstruction* callStaticInstruction); 
		void visit(CallInterfaceInstruction* callInterfaceInstruction); 
		void visit(CallSuperInstruction* callSuperInstruction);
		void visit(CallPropertyInstruction* callPropertyInstruction); 
		void visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction); 
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
		void visit(InlineBeginInstruction* inlineBeginInstruction);
		

	};
}