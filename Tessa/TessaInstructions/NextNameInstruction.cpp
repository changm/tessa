#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	NextNameInstruction::NextNameInstruction(TessaInstruction* receiverObject, TessaInstruction* registerFile) {
		this->receiverObject = receiverObject;
		this->registerFile = registerFile;
	}

	void NextNameInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s NextName %s.%s", getPrintPrefix().c_str(),
			receiverObject->getOperandString().c_str(), registerFile->getOperandString().c_str());
		printf("%s\n", buffer);
	}

	void NextNameInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool NextNameInstruction::producesValue() {
		return true;
	}

	TessaInstruction* NextNameInstruction::getBaseObject() {
		return receiverObject;
	}

	TessaInstruction* NextNameInstruction::getRegisterFile() {
		return registerFile;
	}
}