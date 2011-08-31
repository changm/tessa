
#include <set>

namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;

	class DeadCodeElimination : public TessaVisitorInterface { 
	private:
		// Data
		avmplus::AvmCore* _core;
		MMgc::GC* _gc;
		set<TessaValue*> _liveValues;

	private:
		avmplus::GCSortedMap<TessaValue*, List<TessaValue*, LIST_GCObjects>*, avmplus::LIST_GCObjects>* _useDefinitionChain;
		
		void addUse(TessaValue* use, TessaValue* definition);
		List<TessaValue*, LIST_GCObjects>* getUses(TessaValue* definition);

		bool removeDeadInstructions(ASFunction* function);
		bool isDeadCode(TessaValue* value);
		void removeInstruction(TessaInstruction* instruction);
		void addValues(List<TessaValue*, LIST_GCObjects>* operands);
		void addValue(TessaValue* value); 
		bool isAlive(TessaValue* value);
		List<TessaInstruction*, avmplus::LIST_GCObjects>* cloneInstructionList(List<TessaInstruction*, avmplus::LIST_GCObjects>* instructionsToClone); 

		void markUsedCode(ASFunction* function);
		void addOperands();

	public:
		DeadCodeElimination(avmplus::AvmCore* core, MMgc::GC* gc);
		void eliminateDeadCode(ASFunction* function); 

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
		
		bool isFunctionParameter(TessaInstruction* currentInstruction, BasicBlock* basicBlock);
	};
}