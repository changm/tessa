#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	PopScopeInstruction::PopScopeInstruction(TessaVM::BasicBlock* insertAtEnd) 
		: ScopeInstruction(insertAtEnd)
	{

	}

	void PopScopeInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void PopScopeInstruction::print() {
		printf("%s PopScope\n", this->getPrintPrefix().c_str());
	}

	PopScopeInstruction* PopScopeInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		return new (gc) PopScopeInstruction(insertCloneAtEnd);
	}
}