
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	InitPropertyInstruction::InitPropertyInstruction(TessaInstruction* receiverInstruction, TessaInstruction* propertyKey, TessaInstruction *valueToSet, const Multiname* multiname, Traits* indexTraits, TessaVM::BasicBlock* insertAtEnd) 
		: SetPropertyInstruction(receiverInstruction, propertyKey, valueToSet, multiname, indexTraits, insertAtEnd) 
	{

	}
		/*
	InitPropertyInstruction::InitPropertyInstruction(TessaInstruction *receiverInstruction, TessaInstruction *valueToInit, const Multiname* multiname, TessaInstruction* namespaceInstruction, TessaVM::BasicBlock* insertAtEnd) :	
		PropertyAccessInstruction(receiverInstruction, multiname, NULL, insertAtEnd) 
	{
		this->valueToInit = valueToInit;

		if (namespaceInstruction != NULL) {
			TessaAssert(multiname->isRtns() || multiname->isRtname()); 
		}

		this->namespaceInstruction = namespaceInstruction;
		}
	*/

	bool InitPropertyInstruction::isInitProperty() {
		return true;
	}

	/*
	TessaInstruction* InitPropertyInstruction::getNamespaceInstruction() {
		return namespaceInstruction;
	}
	*/

	void InitPropertyInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void InitPropertyInstruction::print() {
		SetPropertyInstruction::print();
		/*
		char buffer[128];
		char propertyKeyString[32];

		if (propertyName->isRtname() || propertyName->isAnyName()) {
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", "runtime multiname");
		} else {
			avmplus::Stringp propertyNameString = propertyName->getName();
			StUTF8String utf8String(propertyNameString);
			const char* stringValue = utf8String.c_str();
			VMPI_snprintf(propertyKeyString, sizeof(propertyKeyString), "%s", stringValue);
		}

		VMPI_snprintf(buffer, sizeof(buffer), "%s InitProperty %s.%s = %s", this->getPrintPrefix().c_str(),
			receiverInstruction->getOperandString().c_str(), propertyKeyString, valueToInit->getOperandString().c_str());
		printf("%s\n", buffer);
		*/
	}

	/*
	TessaInstruction* InitPropertyInstruction::getValueToInit() {
		return valueToInit;
	}
	*/
}