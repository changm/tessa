#include "TessaInstructionHeader.h"

namespace TessaInstructions { 

	CallVirtualInstruction::CallVirtualInstruction(TessaInstruction* receiverObject, LoadVirtualMethodInstruction* methodToCall, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd)
		: CallInstruction(receiverObject, resultTraits, arguments, methodId, methodInfo, insertAtEnd)
	{
		TessaAssert(receiverObject == methodToCall->getReceiverObject());
		_methodToCall = methodToCall;
	}

	void CallVirtualInstruction::print() {
		CallInstruction::print();
	}

	void CallVirtualInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	CallVirtualInstruction* CallVirtualInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		ArrayOfInstructions* clonedArguments = (ArrayOfInstructions*) originalToCloneMap->get(this->getArguments());
		TessaInstruction* clonedReceiverObject = (TessaInstruction*) originalToCloneMap->get(this->getReceiverObject());	
		LoadVirtualMethodInstruction* loadedMethodClone = (LoadVirtualMethodInstruction*) originalToCloneMap->get(_methodToCall);

		CallVirtualInstruction*	clonedCallInstruction = new (gc) CallVirtualInstruction(clonedReceiverObject, loadedMethodClone, getResultTraits(), clonedArguments, getMethodId(), getMethodInfo(), insertCloneAtEnd);
		clonedCallInstruction->setType(this->getType());
		if (isInlined()) {
			TessaInstruction* clonedReturnValue = (TessaInstruction*) originalToCloneMap->get(this->resolve());
			clonedCallInstruction->setForwardedInstruction(clonedReturnValue);
		}
		return clonedCallInstruction;
	}

	bool CallVirtualInstruction::isCallVirtual() {
		return true;
	}

	LoadVirtualMethodInstruction* CallVirtualInstruction::getLoadedMethodToCall() {
		return _methodToCall;
	}
}