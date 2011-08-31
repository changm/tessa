#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	CallInstruction::CallInstruction(TessaInstruction* receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->_receiverObject = receiverObject;
		this->_arguments = arguments;
		this->_methodInfo = methodInfo;
		this->_methodId = methodId;
		this->_resultTraits = resultTraits;
	}

	CallInstruction::CallInstruction(TessaInstruction* functionObject, TessaInstruction *receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->_receiverObject = receiverObject;
		this->_arguments = arguments;
		this->_functionObject = functionObject;
		this->_resultTraits = resultTraits;
	}

	CallInstruction::CallInstruction(AbcOpcode abcOpcode, TessaInstruction* functionObject, ArrayOfInstructions* arguments, TessaVM::BasicBlock* insertAtEnd)
		: TessaInstruction(insertAtEnd)
	{
		this->_arguments = arguments;
		this->_functionObject = functionObject;
		this->_receiverObject = functionObject;
		this->_abcOpcode = abcOpcode;
	}

	void CallInstruction::print() {
		char buffer[512];
		char typeOfCall[64];
		char receiverString[64];
		char methodName[64];

#ifdef DEBUG
		if (_methodInfo != NULL) {
			avmplus::Stringp methodNameStringP = _methodInfo->getMethodName();
			StUTF8String cString(methodNameStringP);
			VMPI_snprintf(methodName, sizeof(methodName), "%s", cString.c_str());
		} else {
			VMPI_snprintf(methodName, sizeof(methodName), "%s", "");
		}
#endif

		if (this->isConstructProperty()) {
			VMPI_snprintf(typeOfCall, sizeof(typeOfCall), "%s", "Construct Property");
		} else if (this->isConstruct()) {
			VMPI_snprintf(typeOfCall, sizeof(typeOfCall), "%s", "Construct");
		} else if (this->isCallSuper())  {
			VMPI_snprintf(typeOfCall, sizeof(typeOfCall), "%s", "CallSuper");
		} else if (this->isCallInterface()) {
			VMPI_snprintf(typeOfCall, sizeof(typeOfCall), "%s", "Call Interface");
		} else if (this->isInlineMethod()) {
			VMPI_snprintf(typeOfCall, sizeof(typeOfCall), "%s", "Inline Method");
		} else {
			VMPI_snprintf(typeOfCall, sizeof(typeOfCall), "%s", "Call");
		}

		if (this->hasAbcOpcode() && this->isStaticCall()) {
			sprintf(receiverString, "Static ");
		} else if (this->isDynamicMethod()) {
			TessaAssert(_functionObject != NULL);
			sprintf(receiverString, "%s", _functionObject->getOperandString().data());
		} else {
			TessaAssert(receiverString != NULL);
			sprintf(receiverString, "%s", _receiverObject->getOperandString().data());
		}

		if (!isInlined()) {
			VMPI_snprintf(buffer, sizeof(buffer), "%s %s %s(%s) - %s - (Type %s)", getPrintPrefix().c_str(), typeOfCall,
				receiverString, _arguments->getOperandString().c_str(),
				methodName,
				getType()->toString().data());
		} else {
			TessaInstruction* resolvedInstruction = this->resolve();
			VMPI_snprintf(buffer, sizeof(buffer), "%s Inlined Returned Value -> %s (Type %s)", getPrintPrefix().c_str(), 
				resolvedInstruction->getOperandString().data(), getType()->toString().data());
		}

		printf("%s\n", buffer);
	}

	void CallInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaInstruction* CallInstruction::getFunctionObject() {
		TessaAssert(isDynamicMethod());
		return _functionObject;
	}

	TessaInstruction* CallInstruction::getReceiverObject() {
		return _receiverObject;
	}

	ArrayOfInstructions* CallInstruction::getArguments() {
		return _arguments;
	}

	bool CallInstruction::isDynamicMethod() {
		return _functionObject != NULL;
	}

	MethodInfo* CallInstruction::getMethodInfo() {
		return _methodInfo;
	}

	uintptr_t CallInstruction::getMethodId() {
		return _methodId;
	}

	/***
	 * arguments contains this pointer, but the actual number of args to the param is arguments - 1
	 * because we don't count the "this" pointer
	 */
	uint32_t CallInstruction::getNumberOfArgs() {
		return _arguments->size() - 1;
	}

	bool CallInstruction::producesValue() {
		return true;
	}

	AbcOpcode CallInstruction::getAbcOpcode() {
		return this->_abcOpcode;
	}

	void CallInstruction::setAbcOpcode(AbcOpcode opcode) {
		this->_abcOpcode = opcode;
	}

	bool CallInstruction::hasAbcOpcode() {
		return _abcOpcode != NULL;
	}

	bool CallInstruction::isStaticCall() {
		return (_receiverObject == NULL) && (_functionObject == NULL); 
	}

	Traits* CallInstruction::getResultTraits() {
		return this->_resultTraits;
	}

	bool CallInstruction::isCallInterface() {
		return false;
	}

	bool CallInstruction::isCall() {
		return true;
	}

	bool CallInstruction::isResolved() {
		return _methodInfo != NULL;
	}

	bool CallInstruction::isInlined() {
		return this->resolve() != this;
	}

	CallInstruction* CallInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		//TessaAssert(!isInlined());
		CallInstruction* clonedCallInstruction;

		ArrayOfInstructions* clonedArguments = (ArrayOfInstructions*) originalToCloneMap->get(this->getArguments());

		if (isDynamicMethod()) {
			TessaInstruction* clonedFunctionObject = (TessaInstruction*) originalToCloneMap->get(this->getFunctionObject());
			TessaInstruction* receiverObject = NULL;
			if (this->_receiverObject != NULL) {
				receiverObject = (TessaInstruction*) originalToCloneMap->get(this->getReceiverObject());
			}

			clonedCallInstruction = new (gc) CallInstruction(clonedFunctionObject, receiverObject, _resultTraits, clonedArguments, insertCloneAtEnd);
		} else if (hasAbcOpcode()) {
			TessaAssert(false);
			return NULL;
		} else {
			TessaInstruction* clonedReceiverObject = (TessaInstruction*) originalToCloneMap->get(this->getReceiverObject());	
			TessaAssert(_methodInfo != NULL);
			clonedCallInstruction = new (gc) CallInstruction(clonedReceiverObject, _resultTraits, clonedArguments, getMethodId(), getMethodInfo(), insertCloneAtEnd);
		}

		clonedCallInstruction->setType(this->getType());

		if (isInlined()) {
			TessaInstruction* clonedReturnValue = (TessaInstruction*) originalToCloneMap->get(this->resolve());
			clonedCallInstruction->setForwardedInstruction(clonedReturnValue);
		}
		return clonedCallInstruction;
	}

	List<TessaValue*, LIST_GCObjects>* CallInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(this->_arguments);
		if (getReceiverObject() != NULL) {
			operandList->add(getReceiverObject());
		}

		if (isDynamicMethod()) {
			AvmAssert(getFunctionObject() != NULL);
			operandList->add(getFunctionObject());
		}

		if (this->isInlined()) {
			operandList->add(this->resolve());
		}
		return operandList;
	}
}