#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ScopeInstruction::ScopeInstruction(TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{

	}

	bool ScopeInstruction::modifiesScopeStack() {
		return true;
	}
}