#include "TessaInstructionheader.h"

namespace TessaInstructions {
	ConstructInstruction::ConstructInstruction(TessaInstruction *receiverObject, Traits* classTraits, ArrayOfInstructions* arguments, const Multiname* multiname, TessaVM::BasicBlock* insertAtEnd)
		: CallInstruction(receiverObject, classTraits, arguments, NULL, NULL, insertAtEnd)
	{
		this->propertyMultiname = multiname;
	}

	bool ConstructInstruction::isConstruct() {
		return true;
	}

	void ConstructInstruction::print() {
		CallInstruction::print();
	}

	const Multiname* ConstructInstruction::getPropertyMultiname() {
		return propertyMultiname;
	}

	void ConstructInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool ConstructInstruction::producesValue() {
		return true;
	}

	bool ConstructInstruction::isEarlyBound() {
		int argCount = this->getNumberOfArgs();
		Traits* instanceTraits = NULL;
		Traits* classTraits = this->getResultTraits();
        if (classTraits) {
            instanceTraits = classTraits->itraits;
            if (instanceTraits && !instanceTraits->hasCustomConstruct) {
                // Cannot resolve signatures now because that could cause a premature verification failure,
                // one that should occur in the class's script-init.
                // If it's already resolved then we're good to go.
                if (instanceTraits->init && instanceTraits->init->isResolved() && instanceTraits->init->getMethodSignature()->argcOk(argCount)) {
					return true;
                }
            }
        }

		return false;
	}

	int ConstructInstruction::getSlotId() {
		return this->slotId;
	}

	void ConstructInstruction::setSlotId(int slotId) {
		this->slotId = slotId;
	}

	int ConstructInstruction::getConstructorIndex() {
		return this->constructorIndex;
	}

	void ConstructInstruction::setConstructorIndex(int constructorIndex) {
		this->constructorIndex = constructorIndex;
	}

	void ConstructInstruction::setObjectTraits(Traits* objectTraits) {
		this->objectTraits = objectTraits;	
	}

	Traits* ConstructInstruction::getObjectTraits() {
		return this->objectTraits;
	}

	void ConstructInstruction::setEarlyBindPointersForClone(ConstructInstruction* clonedConstructInstruction) {
		clonedConstructInstruction->setSlotId(this->slotId);
		clonedConstructInstruction->setConstructorIndex(this->constructorIndex);
		clonedConstructInstruction->setObjectTraits(this->objectTraits);
	}

	ConstructInstruction* ConstructInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*)originalToCloneMap->get(this->getReceiverObject());
		ArrayOfInstructions* argumentsClone = (ArrayOfInstructions*) originalToCloneMap->get(this->getArguments());
		ConstructInstruction* constructClone = new (gc) ConstructInstruction(receiverClone, this->getResultTraits(), argumentsClone, this->propertyMultiname, insertCloneAtEnd);

		if (this->isEarlyBound()) {
			setEarlyBindPointersForClone(constructClone);
		}

		constructClone->setType(this->getType());
		return constructClone;
	}
}