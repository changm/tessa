
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	SlotAccessInstruction::SlotAccessInstruction(int32_t slotNumber, int32_t slotOffset, TessaInstruction* receiverObject, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->receiverObject = receiverObject;
		this->slotNumber = slotNumber;
		this->slotOffset = slotOffset;
	}

	void SlotAccessInstruction::setSlotNumber(int32_t slot) {
		this->slotNumber = slot;
	}

	int32_t SlotAccessInstruction::getSlotNumber() {
		return slotNumber;
	}

	int32_t SlotAccessInstruction::getSlotOffset() {
		return this->slotOffset;
	}

	bool SlotAccessInstruction::isSlotAccess() {
		return true;
	}

	TessaInstruction* SlotAccessInstruction::getReceiverObject() {
		return receiverObject;
	}

	void SlotAccessInstruction::setReceiverObject(TessaInstruction* newReceiverObject) {
		TessaAssert(newReceiverObject != NULL);
		this->receiverObject = newReceiverObject;
	}
}