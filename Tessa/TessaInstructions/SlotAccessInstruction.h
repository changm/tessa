
namespace TessaInstructions {

	class SlotAccessInstruction : public TessaInstruction {
	private:
		int32_t slotNumber;
		int32_t slotOffset;
		TessaInstruction*	receiverObject;

	public:
		SlotAccessInstruction(int32_t slotNumber, int32_t slotOffset, TessaInstruction* receiverObject, TessaVM::BasicBlock* insertAtEnd);
		void setSlotNumber(int32_t slot);
		int32_t getSlotNumber();
		int32_t getSlotOffset();
		bool isSlotAccess();
		TessaInstruction* getReceiverObject();
		void setReceiverObject(TessaInstruction* newReceiverObject);
		bool hasSideEffect() { return true; }
	};
}