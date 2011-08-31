#ifndef __avmplus_TessaInliner_
#define __avmplus_TessaInliner_

namespace TessaVisitors {
	using namespace TessaInstructions;
	using namespace avmplus;

	class TessaInliner : public TessaVisitorInterface {
	private:
		ASFunction*	_methodToInline;
		ASFunction* _inlineeMethod;
		avmplus::AvmCore*	_core;
		MMgc::GC*	_gc;
		MMgc::GCHashtable _originalToCloneMap;
		bool		_inlinedAMethod;
		Verifier*	_verifier;
		int			_inlineCount;
		void		consistencyCheck();
		void		verbosePrint(std::string stringToPrint); 
		BasicBlock* getClonedReturnBlock(); 

	public:
		TessaInliner(avmplus::AvmCore* core); 
		void visit(TessaInstruction* tessaInstruction); 
		void visit(FindPropertyInstruction* findPropertyInstruction); 
		void visit(ConstantValueInstruction* constantValueInstruction);
		void visit(ArrayOfInstructions* arrayOfInstructions); 
		void visit(ConstructInstruction* constructInstruction);
		void visit(NewObjectInstruction* newObjectInstruction); 
		void visit(ConstructPropertyInstruction* constructPropertyInstruction);
		void visit(ConstructSuperInstruction* constructSuperInstruction); 
		void visit(CallVirtualInstruction* callVirtualInstruction);
		void visit(CallStaticInstruction* callStaticInstruction); 
		void visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction); 
		void visit(CallInstruction* callInstruction); 
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
		bool shouldInlineMethod(CallInstruction* callInstruction);
		void visit(InlineBeginInstruction* inlineBeginInstruction);

		TessaInstruction*	getClonedInstruction(TessaInstruction* originalInstruction);
		void mapMethodParametersToValues(ArrayOfInstructions* instanceArguments); 
		void mergeClonedFirstBasicBlockIntoCall(BasicBlock* callInstructionBlock);

		TessaInstruction* insertInlineGuard(BasicBlock* callBlock, BasicBlock* mergeBlock, BasicBlock* returnBlockOfInlinedMethod, 
			BasicBlock* firstBlockOfInlinedMethod, CallVirtualInstruction* originalCall, TessaInstruction* returnValueOfInlinedMethod);
		void createClonedBasicBlocks(List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocksToInline); 
		void updateClonedPhiInstructions(List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocksToInline); 
		TessaInstruction* createClonedInstructions();
		TessaInstruction* cloneSingleInstruction(TessaInstruction* currentInstruction, BasicBlock* clonedBlock);

		TessaInstruction* inlineMethod(ASFunction* methodToInline, CallVirtualInstruction* callInstruction);
		BasicBlock* splitCallInstructionBlock(CallInstruction* callInstruction);
		void insertJumpFromInlineReturnBlockToMergeBlock(BasicBlock* returnBlockOfInlinedMethod, BasicBlock* afterCallBasicBlock); 
		void findInlineOpportunities(ASFunction* methodToAnalyze, Verifier* verifier);

		void updateCallerPhiInstructions(TessaVM::BasicBlock* originalCallBlock, List<BasicBlock*, LIST_GCObjects>* originalPredecessors, 
			List<BasicBlock*, LIST_GCObjects>* originalSuccessors, TessaVM::BasicBlock* mergeBlock); 
		void updatePhiOperands(BasicBlock* phisInBlock,TessaVM::BasicBlock* originalIncomingEdge,TessaVM::BasicBlock* updatedIncomingEdge);
	};
};

#endif	// class declaration