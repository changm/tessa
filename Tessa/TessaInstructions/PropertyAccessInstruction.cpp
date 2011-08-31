
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	PropertyAccessInstruction::PropertyAccessInstruction(TessaInstruction* receiverInstruction, TessaInstruction* propertyKey, const Multiname* propertyName, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		TessaAssert(receiverInstruction != NULL);
		TessaAssert(propertyName != NULL);
		this->_receiverInstruction = receiverInstruction;
		this->_propertyName = propertyName;
		this->indexTraits = indexTraits;
		this->_propertyKey = propertyKey;
	}

	bool PropertyAccessInstruction::isPropertyAccess() {
		return true;
	}

	TessaInstruction* PropertyAccessInstruction::getReceiverInstruction() {
		return this->_receiverInstruction;
	}

	const Multiname* PropertyAccessInstruction::getPropertyName() {
		return this->_propertyName;
	}

	void PropertyAccessInstruction::setReceiverInstruction(TessaInstruction* newReceiverInstruction) {
		TessaAssert(newReceiverInstruction != NULL);
		this->_receiverInstruction = newReceiverInstruction;
	}
	
	void PropertyAccessInstruction::setPropertyName(const Multiname* propertyName) {
		TessaAssert(_propertyName != NULL);
		this->_propertyName = propertyName;
	}

	Traits* PropertyAccessInstruction::getIndexTraits() {
		return this->indexTraits;
	}

	void PropertyAccessInstruction::setPropertyKey(TessaInstruction* newPropertyKey) {
		_propertyKey = newPropertyKey;
	}
}