
/***
 * This is a terrible instruction but required by the AVM opcode hasnext2 and hasnext
 * The opcode changes two separate values. It changes the object that can be iterated on as well as the register 
 * HasMorePropertiesObjectInstruction is used to change the object. Terrible terrible terrible. 
 * However, SSA limits us to one value per instruction
 * Never use this outside of ABC backwards compatability
 */
namespace TessaInstructions {
	class HasMorePropertiesInstruction : public TessaInstruction {
	private:
		HasMorePropertiesObjectInstruction* objectInstruction;
		HasMorePropertiesRegisterInstruction* registerInstruction;

	public:
		HasMorePropertiesInstruction(HasMorePropertiesObjectInstruction* objectInstruction, HasMorePropertiesRegisterInstruction* registerInstruction);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		HasMorePropertiesObjectInstruction* getObjectIndex();
		HasMorePropertiesRegisterInstruction* getRegisterIndex();
		bool producesValue(); 
		bool modifiesObject();
	};
}