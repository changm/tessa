
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	CallPropertyInstruction::CallPropertyInstruction(TessaInstruction* receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, const Multiname* propertyName, TessaVM::BasicBlock* insertAtEnd) 
		: CallInstruction(receiverObject, receiverTraits, arguments, NULL, NULL, insertAtEnd)
	{
		this->_propertyName = propertyName;
		this->_isSlotBound = false;
	}

	CallPropertyInstruction::CallPropertyInstruction(GetSlotInstruction* functionValue, TessaInstruction* receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, const Multiname* propertyName, TessaVM::BasicBlock* insertAtEnd)
		: CallInstruction(receiverObject, receiverTraits, arguments, NULL, NULL, insertAtEnd)
	{
		this->_propertyName = propertyName;
		this->_functionValue = functionValue;
	}

	void CallPropertyInstruction::print() {
		StUTF8String utf8String(_propertyName->getName());
		const char* stringValue = utf8String.c_str();

		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s CallProperty %s %s(%s) (Type %s)", this->getPrintPrefix().c_str(),
			stringValue, this->getReceiverObject()->getOperandString().c_str(), 
			this->getArguments()->getOperandString().c_str(), getType()->toString().data());
		printf("%s\n", buffer);
	}

	void CallPropertyInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	const Multiname* CallPropertyInstruction::getProperty() {
		return _propertyName;
	}

	bool CallPropertyInstruction::hasValidCacheSlot() {
		return cacheSlot != NULL;
	}

	CallPropertyInstruction* CallPropertyInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*) originalToCloneMap->get(getReceiverObject());
		ArrayOfInstructions* argumentsClone = (ArrayOfInstructions*) originalToCloneMap->get(getArguments());
		CallPropertyInstruction* clonedCallProperty = new (gc) CallPropertyInstruction(receiverClone, getResultTraits(), argumentsClone, _propertyName, insertCloneAtEnd);

		if (hasValidCacheSlot()) {
			clonedCallProperty->cacheSlot = this->cacheSlot;
			clonedCallProperty->cacheHandlerOffset = this->cacheHandlerOffset;
		}

		clonedCallProperty->setType(this->getType());
		clonedCallProperty->_functionValue = (GetSlotInstruction*) originalToCloneMap->get(this->_functionValue);
		return clonedCallProperty;
	}

	bool CallPropertyInstruction::isSlotBound() {
		return this->_functionValue != NULL;
	}

	GetSlotInstruction* CallPropertyInstruction::getFunctionValue() {
		TessaAssert(isSlotBound());
		return this->_functionValue;
	}
}