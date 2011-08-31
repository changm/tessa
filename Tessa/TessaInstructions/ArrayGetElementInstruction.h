namespace TessaInstructions {
	class ArrayGetElementInstruction : public ArrayAccessInstruction {
	private:
	public:
		ArrayGetElementInstruction(TessaInstruction* receiverObject, TessaInstruction* index);
	};
}
