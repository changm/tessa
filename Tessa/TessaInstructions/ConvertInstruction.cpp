
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ConvertInstruction::ConvertInstruction(TessaTypes::Type* typeToConvertTo, TessaInstruction* instructionToConvert, TessaVM::BasicBlock* insertAtEnd) 
		: CoerceInstruction(typeToConvertTo, instructionToConvert, insertAtEnd)
	{
	}

	bool ConvertInstruction::isConvert() {
		return true;
	}

	bool ConvertInstruction::producesValue() {
		return true;
	}

	void ConvertInstruction::print() {
		CoerceInstruction::print();
	}

	void ConvertInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	ConvertInstruction* ConvertInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* clonedInstructionToConvert = (TessaInstruction*) originalToCloneMap->get(this->getInstructionToCoerce());
		return new (gc) ConvertInstruction(this->getTypeToCoerce(), clonedInstructionToConvert, insertCloneAtEnd);
	}
}
