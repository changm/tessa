#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ConditionInstruction::ConditionInstruction(TessaBinaryOp op, TessaValue* leftOperand, TessaValue* rightOperand, TessaVM::BasicBlock* insertAtEnd) 
		: BinaryInstruction(op, leftOperand, rightOperand, insertAtEnd) 
	{
		this->setType(TypeFactory::getInstance()->boolType());
	}

	bool ConditionInstruction::isCondition() {
		return true;
	}

	void ConditionInstruction::print() {
		BinaryInstruction::print();
	}

	void ConditionInstruction::visit(TessaVisitorInterface *tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaValue* ConditionInstruction::getClonedValue(TessaValue* operand, MMgc::GCHashtable* originalToCloneMap) {
		TessaValue* clonedOperand =  (TessaValue*) originalToCloneMap->get(operand);

		if (clonedOperand == NULL) {
			AvmAssert(operand->isConstantValue());
			clonedOperand = operand;
		}

		return clonedOperand;
	}

	ConditionInstruction* ConditionInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaValue* clonedLeftOperand =  getClonedValue(getLeftOperand(), originalToCloneMap);
		TessaValue* clonedRightOperand =  getClonedValue(getRightOperand(), originalToCloneMap);

		TessaAssert(clonedRightOperand != NULL);
		TessaAssert(clonedLeftOperand != NULL);

		ConditionInstruction* clonedInstruction = new (gc) ConditionInstruction(this->getOpcode(), clonedLeftOperand, clonedRightOperand, insertCloneAtEnd);
		clonedInstruction->setType(this->getType());
		return clonedInstruction;
	}
}