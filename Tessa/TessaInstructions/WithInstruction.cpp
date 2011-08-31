
#include "TessaInstructionHeader.h"

namespace TessaInstructions {

	WithInstruction::WithInstruction(TessaInstruction* withObject, TessaVM::BasicBlock* insertAtEnd) 
		: ScopeInstruction(insertAtEnd)
	{
		this->withObject = withObject;
	}

	void WithInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s WithInstruction %s", this->getPrintPrefix().c_str(),
			this->withObject->getOperandString().c_str());
		printf("%s\n", buffer);
	}

	void WithInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaInstruction* WithInstruction::getWithObject() {
		return this->withObject;
	}
}