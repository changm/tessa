
namespace TessaInstructions {
	class ClassFieldAccessInstruction : public PropertyAccessInstruction {
	private:
		bool isSetFieldAccess;
	public:
		ClassFieldAccessInstruction(TessaInstruction* receiverObject, const Multiname* field, TessaVM::BasicBlock* insertAtEnd); 

		bool isSetField();
		bool isGetField();
		bool isClassFieldAccess();
		virtual void forceAbstract() = 0;
	};

}