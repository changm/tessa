#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ClassFieldAccessInstruction::ClassFieldAccessInstruction(TessaInstruction* receiverObject, const Multiname* field, TessaVM::BasicBlock* insertAtEnd) 
		: PropertyAccessInstruction(receiverObject, NULL, field, NULL, insertAtEnd)
	{
		AvmAssert(false);
	}

	bool ClassFieldAccessInstruction::isGetField() {
		return !isSetField();
	}

	bool ClassFieldAccessInstruction::isSetField() {
		return this->isSetFieldAccess;
	}
}