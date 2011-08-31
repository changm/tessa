
namespace TessaInstructions {
	class ReturnInstruction : public TessaInstruction {
	private:
		bool	isVoidReturn;

	public:
		TessaInstruction*	_valueToReturn;
		ReturnInstruction(TessaVM::BasicBlock* insertAtEnd);
		ReturnInstruction(TessaInstruction* valueToReturn, TessaVM::BasicBlock* insertAtEnd);
		TessaInstruction*	getValueToReturn();
		void				setValueToReturn(TessaInstruction *valueToReturn);
		bool				isReturn();
		bool				getIsVoidReturn();
		void				setReturnVoid(bool value);
		void				visit(TessaVisitorInterface* tessaVisitor);
		void				print();
		ReturnInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
		bool hasSideEffect() { return true; }
	};
}