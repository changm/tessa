
namespace TessaInstructions {
	class ArraySetElementInstruction : public ArrayAccessInstruction {
	private:
		TessaInstruction* valueToSet;

	public:
		ArraySetElementInstruction(TessaInstruction *receiverObject, TessaInstruction* index, TessaInstruction *valueToSet);
	};
}