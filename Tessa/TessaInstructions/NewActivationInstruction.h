
namespace TessaInstructions {
	class NewActivationInstruction : public TessaInstruction {
	public:
		NewActivationInstruction(TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isNewActivation();
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc);
		bool hasSideEffect() { return true; }
	};
}