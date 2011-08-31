#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	BranchInstruction::BranchInstruction(TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
	}

	bool BranchInstruction::isBranch() {
		return true;
	}
}