#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ConstructPropertyInstruction::ConstructPropertyInstruction(TessaInstruction *receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, const Multiname *multiname, TessaVM::BasicBlock* insertAtEnd) 
		: ConstructInstruction(receiverObject, resultTraits, arguments, multiname, insertAtEnd)
	{
	}

	void ConstructPropertyInstruction::print() {
		ConstructInstruction::print();
	}

	bool ConstructPropertyInstruction::isConstructProperty() {
		return true;
	}

	void ConstructPropertyInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	ConstructPropertyInstruction* ConstructPropertyInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*)originalToCloneMap->get(this->getReceiverObject());
		ArrayOfInstructions* argumentsClone = (ArrayOfInstructions*) originalToCloneMap->get(this->getArguments());
		ConstructPropertyInstruction* constructPropertyClone = new (gc) ConstructPropertyInstruction(receiverClone, getResultTraits(), argumentsClone, getPropertyMultiname(), insertCloneAtEnd);
		constructPropertyClone->setType(this->getType());

		if (this->isEarlyBound()) {
			setEarlyBindPointersForClone(constructPropertyClone);
		}

		return constructPropertyClone;
	}
}