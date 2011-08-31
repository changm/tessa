namespace TessaInstructions {
	class BinaryInstruction : public TessaInstruction {
	private:
		TessaBinaryOp op;

	public:
		TessaValue * _leftOperand;
		TessaValue* _rightOperand;

		BinaryInstruction(TessaBinaryOp op, TessaValue* leftOperand, TessaValue* rightOperand, TessaVM::BasicBlock* insertAtEnd); 
		static TessaBinaryOp	getBinaryOpcodeFromAbcOpcode(AbcOpcode opcode);
		TessaBinaryOp	getOpcode();
		TessaValue*	getLeftOperand();
		TessaValue*	getRightOperand();

		void	setLeftOperand(TessaValue* newLeftOperand); 
		void	setRightOperand(TessaValue* newRightOperand); 

		bool producesValue();
		bool isBinary();
		void visit(TessaVisitorInterface* tessaVisitor); 
		void print(); 
		BinaryInstruction*	clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc);
	};

}