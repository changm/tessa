
namespace TessaInstructions {
	class PhiInstruction : public TessaInstruction {
	private:
		// From and values must be the same length. An index into one represents the value/location from the other
		List<TessaInstruction*, avmplus::LIST_GCObjects>* _values;
		List<TessaVM::BasicBlock*, avmplus::LIST_GCObjects>* _from;

	public:
		PhiInstruction(TessaVM::BasicBlock* insertInFront);
		PhiInstruction(MMgc::GC* gc, TessaVM::BasicBlock* insertInFront);
		void					addOperand(TessaVM::BasicBlock* comingFrom, TessaInstruction* phiValue);
		void					removeOperand(TessaVM::BasicBlock* incomingEdge);
		TessaVM::BasicBlock*	getIncomingEdge(int index);
		TessaInstruction*		getOperand(TessaVM::BasicBlock* comingFrom);
		void					setOperand(TessaVM::BasicBlock* comingFrom, TessaInstruction* newOperand);
		bool				containsEdge(TessaVM::BasicBlock* incomingEdge);

		bool				isPhi();
		void				print();
		void				visit(TessaVisitorInterface* tessaVisitor);
		int					numberOfOperands();
		PhiInstruction*		clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc);
	};
}