
namespace TessaInstructions {
	class CaseInstruction : public TessaInstruction {
	private:
		TessaInstruction*	_caseValue;
		SwitchInstruction*	_switchInstruction;
		TessaVM::BasicBlock*	_targetBlock;

	public:
		CaseInstruction(TessaInstruction* caseValue, TessaVM::BasicBlock* targetBlock, TessaVM::BasicBlock* insertAtEnd);
		bool isCase();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		SwitchInstruction*	getSwitchInstruction();
		void				setSwitchInstruction(SwitchInstruction* switchInstruction); 
		TessaVM::BasicBlock*	getTargetBlock();
		TessaInstruction*	getCaseValue();
		void				setTargetBlock(TessaVM::BasicBlock* caseTarget);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}