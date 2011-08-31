#include "TessaInstructionheader.h"

/***
 * MUST BE DECLARED IN THE SAME ORDER AS THE ACTUAL UNARY OPS ENUM
 */
#define stringify( name ) # name
const char *TessaUnaryOpNames[] = 
{
	stringify(BITWISE_NOT), 
	stringify(NOT), 
	stringify(NEGATE),
	stringify(MINUS)
};
#undef stringify

namespace TessaInstructions {
	UnaryInstruction::UnaryInstruction(TessaUnaryOp opcode, TessaInstruction* operand, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->setOpcode(opcode);
		this->setOperand(operand);
	}

	TessaInstruction* UnaryInstruction::getOperand() {
		return this->operand;
	}

	TessaUnaryOp UnaryInstruction::getOpcode() {
		return this->opcode;
	}

	void UnaryInstruction::setOperand(TessaInstruction* operand) {
		this->operand = operand;
	}

	void UnaryInstruction::setOpcode(TessaUnaryOp op) {
		this->opcode = op;
	}

	bool UnaryInstruction::isUnary() {
		return true;
	}

	TessaUnaryOp UnaryInstruction::getUnaryOpcodeFromAbcOpcode(AbcOpcode opcode) {
		switch (opcode) {
			case OP_bitnot: 
				return BITWISE_NOT;
			case OP_not: 
				return NOT;
			case OP_negate:
				return NEGATE;
			default:
				TessaAssertMessage(false, "Unknown Unary Op");
				break;
		}

		TessaAssert(false);
		return NOT;
	}

	void UnaryInstruction::print() {
		TessaAssert(opcode < ((sizeof(TessaUnaryOpNames) / sizeof(char*)) ));
		char buffer[512];
		VMPI_snprintf(buffer, sizeof(buffer), "%s UnaryInstruction %s %s -> (Type %s) \n", getPrintPrefix().c_str(), TessaUnaryOpNames[opcode], 
			operand->getOperandString().c_str(),
			getType()->toString().data()
			);
		printf("%s", buffer);
	}

	void UnaryInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool UnaryInstruction::producesValue() {
		return true;
	}

	UnaryInstruction* UnaryInstruction::clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* clonedOperand = (TessaInstruction*) originalToCloneMap->get(operand);
		UnaryInstruction* clonedUnary = new (gc) UnaryInstruction(this->getOpcode(), clonedOperand, insertCloneAtEnd);
		clonedUnary->setType(this->getType());
		return clonedUnary;
	}

	List<TessaValue*, LIST_GCObjects>* UnaryInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getOperand());
		return operandList;
	}
}