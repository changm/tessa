#include "TessaInstructionheader.h"

namespace TessaInstructions {
	ReturnInstruction::ReturnInstruction(BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		isVoidReturn = true;
	}

	ReturnInstruction::ReturnInstruction(TessaInstruction* valueToReturn, BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		setValueToReturn(valueToReturn);
		isVoidReturn = false;
	}

	TessaInstruction* ReturnInstruction::getValueToReturn() {
		return _valueToReturn;
	}

	/***
	 * Sets the value to return. In addition, sets the type to return of this instruction
	 * to the same type as the value to return
	 */
	void ReturnInstruction::setValueToReturn(TessaInstruction *valueToReturn) {
		this->_valueToReturn = valueToReturn;
		this->setType(valueToReturn->getType());
	}

	bool ReturnInstruction::isReturn() {
		return true;
	}

	bool ReturnInstruction::getIsVoidReturn() {
		return isVoidReturn;
	}

	void ReturnInstruction::setReturnVoid(bool value) {
		isVoidReturn = value;
	}

	void ReturnInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void ReturnInstruction::print() {
		char buffer[128];
		char typeOfReturn[64];

		if (isVoidReturn) {
			VMPI_snprintf(typeOfReturn, sizeof(typeOfReturn), "%s", "ReturnVoid");
		} else {
			VMPI_snprintf(typeOfReturn, sizeof(typeOfReturn), "%s %s (Type %s)", "Return", _valueToReturn->getOperandString().c_str(),
				getType()->toString().data());
		}


		VMPI_snprintf(buffer, sizeof(buffer), "%s %s \n", getPrintPrefix().c_str(), typeOfReturn);
		printf("%s", buffer);
	}

	ReturnInstruction* ReturnInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaAssert(_valueToReturn != NULL);
		ReturnInstruction* returnInstruction = new (gc) ReturnInstruction(_valueToReturn, insertCloneAtEnd);
		returnInstruction->setType(this->getType());
		returnInstruction->_valueToReturn = (TessaInstruction*) originalToCloneMap->get(this->_valueToReturn);
		return returnInstruction;
	}

	List<TessaValue*, LIST_GCObjects>* ReturnInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
//		operandList->add(getValueToReturn()->resolve());
		operandList->add(getValueToReturn());
		return operandList;
	}
}