
#include "TessaInstructionHeader.h"

namespace TessaInstructions { 

	CallStaticInstruction::CallStaticInstruction(TessaInstruction* receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd)
		: CallInstruction(receiverObject, resultTraits, arguments, methodId, methodInfo, insertAtEnd)
	{
		TessaAssert(false);

	}

	void CallStaticInstruction::print() {
		CallInstruction::print();
	}

	void CallStaticInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	CallStaticInstruction* CallStaticInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		ArrayOfInstructions* clonedArguments = (ArrayOfInstructions*) originalToCloneMap->get(this->getArguments());
		TessaInstruction* clonedReceiverObject = (TessaInstruction*) originalToCloneMap->get(this->getReceiverObject());	
		CallStaticInstruction*	clonedCallInstruction = new (gc) CallStaticInstruction(clonedReceiverObject, getResultTraits(), clonedArguments, getMethodId(), getMethodInfo(), insertCloneAtEnd);
		clonedCallInstruction->setType(this->getType());
		return clonedCallInstruction;
	}

	bool CallStaticInstruction::isCallStatic() {
		return true;
	}
}
