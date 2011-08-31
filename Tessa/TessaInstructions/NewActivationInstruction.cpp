
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	NewActivationInstruction::NewActivationInstruction(TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{

	}

	void NewActivationInstruction::print() {
		printf("%s NewActivation (Type %s)\n", this->getPrintPrefix().c_str(),
			getType()->toString().data());
	}

	void NewActivationInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool NewActivationInstruction::isNewActivation() {
		return true;
	}

	List<TessaValue*, LIST_GCObjects>* NewActivationInstruction::getOperands(MMgc::GC* gc) {
		return new (gc) List<TessaValue*, LIST_GCObjects>(gc);
	}
}