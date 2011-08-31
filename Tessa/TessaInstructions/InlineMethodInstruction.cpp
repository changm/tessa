#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	InlineMethodInstruction::InlineMethodInstruction(TessaInstruction* receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, BasicBlock* insertAtEnd) :
		CallInstruction(receiverObject, resultTraits, arguments, methodId, methodInfo, insertAtEnd) {
	}

	bool InlineMethodInstruction::isInlineMethod() {
		return true;
	}

	void InlineMethodInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		return tessaVisitor->visit(this);
	}
}