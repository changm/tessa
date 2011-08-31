#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ConstantValueInstruction::ConstantValueInstruction(ConstantValue* constantValue, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->_constantValue = constantValue;
	}

	void ConstantValueInstruction::print() {
		string value = _constantValue->toString();
		printf("%s ConstantValueInstruction %s\n", getPrintPrefix().c_str(), value.c_str());
	}

	void ConstantValueInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	ConstantValue* ConstantValueInstruction::getConstantValue() {
		return _constantValue;
	}

	bool ConstantValueInstruction::producesValue() {
		return true;
	}

	bool ConstantValueInstruction::isConstantValue() {
		return true;
	}

	ConstantValueInstruction* ConstantValueInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		ConstantValueInstruction* clonedConstant = new (gc) ConstantValueInstruction(_constantValue, insertCloneAtEnd);
		clonedConstant->setType(this->getType());
		return clonedConstant;
	}

	List<TessaValue*, LIST_GCObjects>* ConstantValueInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(this);
		return operandList;
	}
}