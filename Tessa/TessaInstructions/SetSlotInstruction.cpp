
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	SetSlotInstruction::SetSlotInstruction(uint32_t slotNumber, int32_t slotOffset, TessaInstruction* receiverObject, TessaInstruction* valueToSet, Traits* slotTraits, TessaVM::BasicBlock* insertAtEnd) 
		: SlotAccessInstruction(slotNumber, slotOffset, receiverObject, insertAtEnd)
	{
		this->valueToSet = valueToSet;
		this->slotTraits = slotTraits;
	}

	void SetSlotInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s SetSlot %s.slot %d (Offset %d) = %s (Type %s)", this->getPrintPrefix().c_str(),
			getReceiverObject()->getOperandString().c_str(), 
			getSlotNumber(), 
			getSlotOffset(),
			valueToSet->getOperandString().c_str(),
			getType()->toString().data()
			);
		printf("%s\n", buffer);
	}

	void SetSlotInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void SetSlotInstruction::setValueToSet(TessaInstruction* valueToset) {
		this->valueToSet = valueToSet;
	}

	TessaInstruction* SetSlotInstruction::getValueToSet() {
		return valueToSet;
	}

	Traits*	SetSlotInstruction::getSlotTraits() {
		return this->slotTraits;
	}

	SetSlotInstruction*	SetSlotInstruction::clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*) originalToCloneMap->get(this->getReceiverObject());
		TessaInstruction* valueToSetClone = (TessaInstruction*) originalToCloneMap->get(this->getValueToSet());
		SetSlotInstruction* setSlotClone = new (gc) SetSlotInstruction(this->getSlotNumber(), getSlotOffset(), receiverClone, valueToSetClone, slotTraits, insertCloneAtEnd);
		setSlotClone->setType(this->getType());
		return setSlotClone;
	}

	List<TessaValue*, LIST_GCObjects>* SetSlotInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getReceiverObject());
		operandList->add(getValueToSet());
		return operandList;
	}
}