
namespace TessaInstructions {
	class NextNameInstruction : public TessaInstruction {
	private:
		TessaInstruction* receiverObject;
		TessaInstruction* registerFile;

	public:
		NextNameInstruction(TessaInstruction* receiverObject, TessaInstruction* registerFile);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool producesValue();
		TessaInstruction* getBaseObject();
		TessaInstruction* getRegisterFile();

	};

}