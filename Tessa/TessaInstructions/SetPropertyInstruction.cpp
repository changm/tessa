
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	SetPropertyInstruction::SetPropertyInstruction(TessaInstruction *receiverInstruction, TessaInstruction* propertyKey, TessaInstruction *valueToSet, 
		const Multiname* multiname, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd) :	
		PropertyAccessInstruction(receiverInstruction, propertyKey, multiname, indexTraits, insertAtEnd) 
	{
		TessaAssert(valueToSet != NULL);
//		this->_propertyKey = propertyKey;
		this->_valueToSet = valueToSet;
	}

	bool SetPropertyInstruction::isSetProperty() {
		return true;
	}

	void SetPropertyInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void SetPropertyInstruction::print() {
		char buffer[128];
		char propertyKeyString[32];
		char instructionType[32];
		TessaInstruction* propertyKey = getPropertyKey();

		if (this->isInitProperty()) {
			VMPI_snprintf(instructionType, sizeof(instructionType), "%s", "InitProperty");
		} else {
			VMPI_snprintf(instructionType, sizeof(instructionType), "%s", "SetProperty");
		}

		if (_propertyKey != NULL) {
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", propertyKey->getOperandString().c_str());
		} else {
			avmplus::Stringp propertyNameString = this->_propertyName->getName();
			StUTF8String utf8String(propertyNameString);
			const char* stringValue = utf8String.c_str();
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", stringValue);
		}

		VMPI_snprintf(buffer, sizeof(buffer), "%s %s %s.%s = %s", this->getPrintPrefix().c_str(),
			instructionType, getReceiverInstruction()->getOperandString().c_str(), propertyKeyString, _valueToSet->getOperandString().c_str());
		printf("%s\n", buffer);
	}

	TessaInstruction* SetPropertyInstruction::getValueToSet() {
		return _valueToSet;
	}

	SetPropertyInstruction*	SetPropertyInstruction::clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*) originalToCloneMap->get(this->getReceiverInstruction());
		TessaInstruction* valueClone = (TessaInstruction*) originalToCloneMap->get(_valueToSet);
		TessaInstruction* propertyClone = NULL;
		if (_propertyKey != NULL) {
			propertyClone = (TessaInstruction*) originalToCloneMap->get(getPropertyKey());
		}

		SetPropertyInstruction* clonedSetProperty = new (gc) SetPropertyInstruction(receiverClone, propertyClone, valueClone, this->getPropertyName(), this->getIndexTraits(), insertCloneAtEnd);
		if (this->usePropertyCache) {
			clonedSetProperty->setCacheSlot = this->setCacheSlot;
			clonedSetProperty->setCacheHandlerOffset = this->setCacheHandlerOffset;
			clonedSetProperty->usePropertyCache = true;
		}

		clonedSetProperty->indexTraits = this->indexTraits;
		clonedSetProperty->valueTraits = this->valueTraits;
		clonedSetProperty->setType(this->getType());
		clonedSetProperty->objectType = this->objectType;
		clonedSetProperty->indexType = this->indexType;
		return clonedSetProperty;
	}

	List<TessaValue*, LIST_GCObjects>* SetPropertyInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getReceiverInstruction());
		if (getPropertyKey() != NULL) {
			operandList->add(getPropertyKey());
		}
		operandList->add(getValueToSet());
		return operandList;
	}
}