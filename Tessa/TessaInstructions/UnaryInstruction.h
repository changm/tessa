
namespace TessaInstructions {
	class UnaryInstruction : public TessaInstruction {
	private:
		TessaUnaryOp		opcode;
		TessaInstruction*	operand;

	public:
		UnaryInstruction(TessaUnaryOp opcode, TessaInstruction* operand, TessaVM::BasicBlock* insertAtEnd);
		TessaInstruction*	getOperand();
		TessaUnaryOp		getOpcode();
		void				setOperand(TessaInstruction* operand);
		void				setOpcode(TessaUnaryOp op);
		bool				isUnary();
		void				print(); 
		void				visit(TessaVisitorInterface* tessaVisitor); 
		bool				producesValue();
		static TessaUnaryOp		getUnaryOpcodeFromAbcOpcode(AbcOpcode opcode); 
		UnaryInstruction*	clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}