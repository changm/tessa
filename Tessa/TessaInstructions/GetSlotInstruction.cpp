#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	GetSlotInstruction::GetSlotInstruction(int32_t slotNumber, int32_t slotOffset, TessaInstruction* receiverInstruction, TessaVM::BasicBlock* insertAtEnd) 
		: SlotAccessInstruction(slotNumber, slotOffset, receiverInstruction, insertAtEnd)
	{

	}

	void GetSlotInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s GetSlot %s.slot %d (Offset %d) (Type %s)", this->getPrintPrefix().c_str(), 
			getReceiverObject()->getOperandString().c_str(),
			this->getSlotNumber(),
			this->getSlotOffset(), 
			getType()->toString().data());
		printf("%s\n",  buffer);
	}

	void GetSlotInstruction::visit(TessaVisitorInterface* visitor) {
		visitor->visit(this);
	}

	bool GetSlotInstruction::producesValue() {
		return true;
	}

	GetSlotInstruction*	GetSlotInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* clonedReceiver = (TessaInstruction*) (originalToCloneMap->get(this->getReceiverObject()));
		TessaAssert(clonedReceiver != NULL);
		GetSlotInstruction* clonedGetSlot = new (gc) GetSlotInstruction(this->getSlotNumber(), getSlotOffset(), clonedReceiver, insertCloneAtEnd);
		clonedGetSlot->setType(this->getType());
		return clonedGetSlot;
	}

	List<TessaValue*, LIST_GCObjects>* GetSlotInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getReceiverObject());
		return operandList;
	}
}