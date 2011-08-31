

namespace TessaInstructions {
	class InitPropertyInstruction : public SetPropertyInstruction {
	private:

	public:
		InitPropertyInstruction(TessaInstruction* receiverInstruction, TessaInstruction* propertyKey, TessaInstruction *valueToSet, const Multiname* multiname, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd);
		bool isInitProperty();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
	};
}