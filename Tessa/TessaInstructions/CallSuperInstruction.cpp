#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	// Used when we can early bind what the "super" is
	CallSuperInstruction::CallSuperInstruction(TessaInstruction *receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, const Multiname* multiname, BasicBlock* insertAtEnd)
		: CallInstruction(receiverObject, receiverTraits, arguments, methodId, methodInfo, insertAtEnd)
	{
		TessaAssert(multiname != NULL);
		this->multiname = multiname;
		this->earlyBound = true;
	}

	// The generic call super
	CallSuperInstruction::CallSuperInstruction(TessaInstruction *receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, const Multiname* multiname, BasicBlock* insertAtEnd) 
		: CallInstruction(receiverObject, receiverTraits, arguments, 0, NULL, insertAtEnd)
	{
		TessaAssert(multiname != NULL);
		this->earlyBound = false;
		this->multiname = multiname;
	}

	bool CallSuperInstruction::isEarlyBound() {
		return earlyBound;
	}

	bool CallSuperInstruction::isSuper() {
		return true;
	}

	void CallSuperInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void CallSuperInstruction::print() {
		CallInstruction::print();
	}

	bool CallSuperInstruction::isCallSuper() {
		return true;
	}

	const Multiname* CallSuperInstruction::getMultiname() {
		TessaAssert(multiname != NULL);
		return this->multiname;
	}

	CallSuperInstruction* CallSuperInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverClone = (TessaInstruction*) originalToCloneMap->get(getReceiverObject());
		ArrayOfInstructions* argumentsClone = (ArrayOfInstructions*) originalToCloneMap->get(getArguments());
		CallSuperInstruction* clonedCallSuper;

		if (isEarlyBound()) {
			clonedCallSuper = new (gc) CallSuperInstruction(receiverClone, getResultTraits(), argumentsClone, getMethodId(), getMethodInfo(), multiname, insertCloneAtEnd);
		} else {
			clonedCallSuper = new (gc) CallSuperInstruction(receiverClone, getResultTraits(), argumentsClone, multiname, insertCloneAtEnd);
		}

		clonedCallSuper->setType(this->getType());
		return clonedCallSuper;
	}
}