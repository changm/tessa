
/***
 * Represents the object result in the AVM2 opcode hasnext. 
 * THIS IS A TERRIBLE INSTRUCTION, please replace and never use other than
 * for hasnext
 */
namespace TessaInstructions {
	class HasMorePropertiesObjectInstruction : public TessaInstruction {
	private:
		TessaInstruction* objectInstruction;

	public:
		HasMorePropertiesObjectInstruction(TessaInstruction* objectInstruction);
		void visit(TessaVisitorInterface* tessaVisitor);
		void print();
		TessaInstruction* getObjectInstruction();
	};
}