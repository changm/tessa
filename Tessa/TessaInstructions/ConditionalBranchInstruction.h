
namespace TessaInstructions {
	using namespace TessaVM;

	class ConditionalBranchInstruction : public BranchInstruction {
	private:
		TessaInstruction*	_branchCondition;
		TessaVM::BasicBlock*	_trueTarget;
		TessaVM::BasicBlock*	_falseTarget;

	public:
		ConditionalBranchInstruction(TessaInstruction* branchCondition, TessaVM::BasicBlock* trueTarget, TessaVM::BasicBlock* falseTarget, TessaVM::BasicBlock* insertAtEnd);
		ConditionalBranchInstruction(TessaInstruction* branchCondition, TessaVM::BasicBlock* insertAtEnd);

		void				setBranchCondition(TessaInstruction* branchCondition); 
		TessaInstruction*	getBranchCondition(); 

		void				setTrueTarget(TessaVM::BasicBlock* trueBasicBlock);
		void				setFalseTarget(TessaVM::BasicBlock* falseBasicBlock);
		TessaVM::BasicBlock*			getTrueTarget();
		TessaVM::BasicBlock*			getFalseTarget();

		void				print();
		void				visit(TessaVisitorInterface* tessaVisitor);
		 
		bool				isConditionalBranch();
		bool				isUnconditionalBranch();
		ConditionalBranchInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 

		void				forceAbstract() { }
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}