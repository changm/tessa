
namespace TessaInstructions {
	class UnconditionalBranchInstruction : public BranchInstruction {
	private:
		TessaVM::BasicBlock* _branchTarget;

	public:
		UnconditionalBranchInstruction(TessaVM::BasicBlock* branchTarget, TessaVM::BasicBlock* insertAtEnd);
		UnconditionalBranchInstruction(TessaVM::BasicBlock* insertAtEnd);

		TessaVM::BasicBlock*	getBranchTarget();
		void				setBranchTarget(TessaVM::BasicBlock* branchTarget);
		void				print();
		void				visit(TessaVisitorInterface* tessaVisitor);
		bool				isConditionalBranch();
		bool				isUnconditionalBranch();

		UnconditionalBranchInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		void				forceAbstract() { }
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc);
	};
}