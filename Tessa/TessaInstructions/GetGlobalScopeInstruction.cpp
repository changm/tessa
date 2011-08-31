
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	GetGlobalScopeInstruction::GetGlobalScopeInstruction(TessaVM::BasicBlock* insertAtEnd) 
		: GetScopeObjectInstruction(0, false, insertAtEnd)
	{

	}

	void GetGlobalScopeInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void GetGlobalScopeInstruction::print() {
		printf("%s GetGlobalScope\n", this->getPrintPrefix().c_str());
	}

	GetGlobalScopeInstruction*	GetGlobalScopeInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		GetGlobalScopeInstruction* cloned = new (gc) GetGlobalScopeInstruction(insertCloneAtEnd);
		cloned->setType(this->getType());
		return cloned;
	}
}