
namespace TessaInstructions {
	class WithInstruction : public ScopeInstruction {
	private:
		TessaInstruction* withObject;

	public:
		WithInstruction(TessaInstruction* withObject, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		TessaInstruction* getWithObject();
	};
}