#include "TessaInstructionHeader.h"

namespace TessaInstructions { 
	ConstructSuperInstruction::ConstructSuperInstruction(TessaInstruction *receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, MethodInfo* methodInfo, BasicBlock* insertAtEnd) 
		: ConstructInstruction(receiverObject, receiverTraits, arguments, NULL, insertAtEnd)
	{
		this->_methodInfo = methodInfo;
	}

	MethodInfo* ConstructSuperInstruction::getMethodInfo() {
		return this->_methodInfo;
	}

	void ConstructSuperInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s ConstructSuper %s.(%s)", this->getPrintPrefix().c_str(),
			this->getReceiverObject()->getOperandString().c_str(), this->getArguments()->getOperandString().c_str());
		printf("%s\n", buffer);
	}

	bool ConstructSuperInstruction::isConstructSuper() {
		return true;
	}

	bool ConstructSuperInstruction::isSuper() {
		return true;
	}

	void ConstructSuperInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	ConstructSuperInstruction* ConstructSuperInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*)originalToCloneMap->get(this->getReceiverObject());
		ArrayOfInstructions* argumentsClone = (ArrayOfInstructions*) originalToCloneMap->get(this->getArguments());
		ConstructSuperInstruction* constructSuperClone = new (gc) ConstructSuperInstruction(receiverClone, getResultTraits(), argumentsClone, _methodInfo, insertCloneAtEnd);
		constructSuperClone->setType(this->getType());
		return constructSuperClone;
	}
}