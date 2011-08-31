
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	GetPropertyInstruction::GetPropertyInstruction(TessaInstruction* receiverInstruction, TessaInstruction* propertyKey, const Multiname* propertyName, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd) :	
		PropertyAccessInstruction(receiverInstruction, propertyKey, propertyName, indexTraits, insertAtEnd) 
	{
		TessaAssert(propertyName != NULL);
		this->_propertyName = propertyName;
		this->usePropertyCache = false;
	}

	const Multiname* GetPropertyInstruction::getPropertyMultiname() {
		return _propertyName;
	}

	void GetPropertyInstruction::print() {
		char buffer[128];
		char propertyKeyString[32];

		if (getPropertyKey() != NULL) {
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", getPropertyKey()->getOperandString().c_str());
		} else if (_propertyName->isRtname() || _propertyName->isAnyName()) {
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", "runtime multiname");
		} else {
			avmplus::Stringp propertyNameString = this->_propertyName->getName();
			StUTF8String utf8String(propertyNameString);
			const char* stringValue = utf8String.c_str();
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", stringValue);
		}

		VMPI_snprintf(buffer, sizeof(buffer), "%s GetProperty %s.%s (Type %s)", this->getPrintPrefix().c_str(),
			_receiverInstruction->getOperandString().c_str(), propertyKeyString,
			getType()->toString().data());
		printf("%s\n", buffer);
	}

	void GetPropertyInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool GetPropertyInstruction::isGetProperty() {
		return true;
	}

	bool GetPropertyInstruction::producesValue() {
		return true;
	}

	GetPropertyInstruction* GetPropertyInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* clonedReceiver = getClonedValue(getReceiverInstruction(), originalToCloneMap);
		TessaInstruction* clonedPropertyKey = NULL;

		if (getPropertyKey() != NULL) {
			clonedPropertyKey = getClonedValue(getPropertyKey(), originalToCloneMap);
		}
		
		GetPropertyInstruction* clonedGetProperty = new (gc) GetPropertyInstruction(clonedReceiver, clonedPropertyKey, getPropertyMultiname(), getIndexTraits(), insertCloneAtEnd);
		if (usePropertyCache) {
			clonedGetProperty->usePropertyCache = true;
			clonedGetProperty->getCacheGetHandlerOffset = this->getCacheGetHandlerOffset;
			clonedGetProperty->getCacheSlot = this->getCacheSlot;
		}

		clonedGetProperty->objectTraits = this->objectTraits;
		clonedGetProperty->resultTraits = this->resultTraits;
		clonedGetProperty->setType((this->getType()));
		clonedGetProperty->objectType = this->objectType;
		clonedGetProperty->indexType = this->indexType;
		return clonedGetProperty;
	}

	List<TessaValue*, LIST_GCObjects>* GetPropertyInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getReceiverInstruction());
		if (getPropertyKey() != NULL) {
			operandList->add(getPropertyKey());
		}
		return operandList;
	}
}