
/***
 * Represents the Register result in the AVM2 opcode hasnext. 
 * THIS IS A TERRIBLE INSTRUCTION, please replace and never use other than
 * for hasnext
 */
namespace TessaInstructions {
	class HasMorePropertiesRegisterInstruction : public TessaInstruction {
	private:
		TessaInstruction* registerInstruction;

	public:
		HasMorePropertiesRegisterInstruction(TessaInstruction* registerInstruction);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		TessaInstruction* getRegisterInstruction();
		bool producesValue();
	};
}