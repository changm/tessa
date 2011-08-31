
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	HasMorePropertiesRegisterInstruction::HasMorePropertiesRegisterInstruction(TessaInstruction* registerInstruction) {
		this->registerInstruction = registerInstruction;
	}

	void HasMorePropertiesRegisterInstruction::print() {
		printf("%s HasMorePropertiesIndex Register %s\n", this->getPrintPrefix().c_str(),
			registerInstruction->getOperandString().c_str()
			);
	}

	void HasMorePropertiesRegisterInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaInstruction* HasMorePropertiesRegisterInstruction::getRegisterInstruction() {
		return this->registerInstruction;
	}

	bool HasMorePropertiesRegisterInstruction::producesValue() {
		return true;
	}
}