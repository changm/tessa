
/***
 * Corresponds to the ABC Opcode hasnext
 */

#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	HasMorePropertiesObjectInstruction::HasMorePropertiesObjectInstruction(TessaInstruction* objectInstruction) {
		this->objectInstruction = objectInstruction;
	}

	TessaInstruction* HasMorePropertiesObjectInstruction::getObjectInstruction() {
		return this->objectInstruction;
	}

	void HasMorePropertiesObjectInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void HasMorePropertiesObjectInstruction::print() {
		printf("%s HasMorePropertiesObject %s\n", this->getPrintPrefix().c_str(), objectInstruction->getOperandString().c_str());
	}
}