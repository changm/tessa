
#include "TessaInstructionHeader.h"

namespace TessaInstructions {

	HasMorePropertiesInstruction::HasMorePropertiesInstruction(HasMorePropertiesObjectInstruction* objectInstruction, HasMorePropertiesRegisterInstruction* registerInstruction) {
		this->objectInstruction = objectInstruction;
		this->registerInstruction = registerInstruction;
	}

	void HasMorePropertiesInstruction::print() {
		printf("%s HasMorePropertiesInstruction Object %s Register %s\n", this->getPrintPrefix().c_str(),
			this->objectInstruction->getOperandString().c_str(),
			this->registerInstruction->getOperandString().c_str());
	}

	void HasMorePropertiesInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	HasMorePropertiesObjectInstruction* HasMorePropertiesInstruction::getObjectIndex() {
		return this->objectInstruction;
	}

	HasMorePropertiesRegisterInstruction* HasMorePropertiesInstruction::getRegisterIndex() {
		return this->registerInstruction;
	}

	bool HasMorePropertiesInstruction::producesValue() {
		return true;
	}

	/**
	 * if true, this is the avm2 hasnext2 opcode
	 * otherwise, is the hasnext
	 */
	bool HasMorePropertiesInstruction::modifiesObject() {
		return (registerInstruction != NULL) && (objectInstruction != NULL);
	}
}