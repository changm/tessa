
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	CallInterfaceInstruction::CallInterfaceInstruction(TessaInstruction* receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, 
		uint32_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd) 
		: CallInstruction(receiverObject, receiverTraits, arguments, methodId, methodInfo, insertAtEnd)
	{

	}

	void CallInterfaceInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool CallInterfaceInstruction::isCallInterface() {
		return true;
	}

	CallInterfaceInstruction* CallInterfaceInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* clonedReceiver = (TessaInstruction*) originalToCloneMap->get(getReceiverObject());
		ArrayOfInstructions* clonedArguments = (ArrayOfInstructions*) originalToCloneMap->get(getArguments());
		AvmAssert (clonedArguments != NULL);
		AvmAssert(clonedArguments != NULL);

		CallInterfaceInstruction* clonedCallInterface = new (gc) CallInterfaceInstruction(clonedReceiver, getResultTraits(), clonedArguments, getMethodId(), getMethodInfo(), insertCloneAtEnd);
		clonedCallInterface->setType(this->getType());
		return clonedCallInterface;
	}
}