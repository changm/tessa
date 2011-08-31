
/*
namespace TessaVM {
	class BasicBlock;
}

namespace TessaInstructions {
	using namespace TessaVM;

	class LabelInstruction : public TessaInstruction {
	private:
		TessaVM::BasicBlock* _basicBlock;	// Have to fully qualify sinces name collides with llvm::BasicBlock
		int _labelId;

	public:
		LabelInstruction();
		LabelInstruction(TessaVM::BasicBlock* basicBlock, int labelId);
		bool	isLabel();
		TessaVM::BasicBlock* getBasicBlock();
		int		getLabelId();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		LabelInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap);
	};
}
*/