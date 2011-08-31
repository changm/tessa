#include "TessaInstructionHeader.h"
/***
 * MUST BE DECLARED IN THE SAME ORDER AS THE ACTUAL BINARY OPS ENUM
 */
#define stringify( name ) # name
const char *TessaBinaryOpNames[] = 
{
	stringify(ADD), 
	stringify(SUBTRACT), 
	stringify(MULTIPLY), 
	stringify(DIVIDE), 
	stringify(MOD), 
	stringify(EQUAL), 
	stringify(NOT_EQUAL), 
	stringify(LESS_THAN), 
	stringify(NOT_LESS_THAN),
	stringify(LESS_EQUAL_THAN), 
	stringify(NOT_LESS_EQUAL_THAN),
	stringify(GREATER_THAN), 
	stringify(NOT_GREATER_THAN),
	stringify(GREATER_EQUAL_THAN),	
	stringify(NOT_GREATER_EQUAL_THAN),
	stringify(BITWISE_AND), 
	stringify(BITWISE_OR), 
	stringify(BITWISE_XOR), 
	stringify(BITWISE_LSH), 
	stringify(BITWISE_RSH), 
	stringify(BITWISE_URSH),
	stringify(IFTRUE),
	stringify(IFFALSE),
	stringify(STRICT_EQUAL),
	stringify(STRICT_NOT_EQUAL),
};
#undef stringify


namespace TessaInstructions {
	BinaryInstruction::BinaryInstruction(TessaBinaryOp op, TessaValue* leftOperand, TessaValue* rightOperand, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->op = op;
		this->_leftOperand = leftOperand;
		this->_rightOperand = rightOperand;
	}

	TessaBinaryOp BinaryInstruction::getBinaryOpcodeFromAbcOpcode(AbcOpcode opcode) {
		switch (opcode) {
			case OP_lshift:
				return BITWISE_LSH;
			case OP_rshift:
				return BITWISE_RSH;
			case OP_urshift:
				return BITWISE_URSH;
			case OP_increment:
			case OP_increment_i:
			case OP_add:
			case OP_add_i:
				return ADD;
			case OP_decrement:
			case OP_decrement_i:
			case OP_subtract_i:
			case OP_subtract:
				return SUBTRACT;
			case OP_modulo:
				return MOD;
	        case OP_divide:
				return DIVIDE;
			case OP_multiply:
	        case OP_multiply_i:
				return MULTIPLY;
			case OP_bitand:
				return BITWISE_AND;
			case OP_bitxor:
				return BITWISE_XOR;
			case OP_bitor:
				return BITWISE_OR;
			case OP_lessthan:
			case OP_iflt:
				return LESS_THAN;
			case OP_lessequals:
            case OP_ifle:
				return LESS_EQUAL_THAN;
			case OP_greaterthan:
            case OP_ifgt:
				return GREATER_THAN;
			case OP_greaterequals:
            case OP_ifge:
				return GREATER_EQUAL_THAN;
			case OP_equals:
            case OP_ifeq:
				return EQUAL;
            case OP_ifne:
				return NOT_EQUAL;
			case OP_ifnlt:
				return NOT_LESS_THAN;
            case OP_ifnle:
				return NOT_LESS_EQUAL_THAN;
			case OP_ifngt:
				return NOT_GREATER_THAN;
            case OP_ifnge:
				return NOT_GREATER_EQUAL_THAN;
			case OP_iftrue:
				return IFTRUE;
			case OP_iffalse:
				return IFFALSE;
			case OP_strictequals:
			case OP_ifstricteq:
				  return STRICT_EQUAL;
	        case OP_ifstrictne:
				return STRICT_NOT_EQUAL;
			default:
				TessaAssertMessage(false, "Unknown Binary Op");
				break;
		}

		TessaAssert(false);
		return ADD;
	}

	bool BinaryInstruction::isBinary() {
		return true;
	}

	TessaBinaryOp BinaryInstruction::getOpcode() {
		return this->op;
	}

	void BinaryInstruction::setLeftOperand(TessaValue* newLeftOperand) {
		TessaAssert(newLeftOperand != NULL);
		_leftOperand = newLeftOperand;
	}

	void BinaryInstruction::setRightOperand(TessaValue* newRightOperand) {
		TessaAssert(newRightOperand != NULL);
		_rightOperand = newRightOperand;
	}

	TessaValue* BinaryInstruction::getLeftOperand() {
		return this->_leftOperand->resolve();
	}

	TessaValue* BinaryInstruction::getRightOperand() {
		return this->_rightOperand->resolve();
	}

	void BinaryInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void BinaryInstruction::print() {
		TessaAssert(op < ((sizeof(TessaBinaryOpNames) / sizeof(char*)) ));
		char instructionType[32];
		if (this->isCondition()) {
			VMPI_snprintf(instructionType, sizeof(instructionType), "ConditionInstruction");
		} else {
			VMPI_snprintf(instructionType, sizeof(instructionType), "BinaryInstruction");
		}

		char buffer[512];
		VMPI_snprintf(buffer, sizeof(buffer), "%s %s %s %s %s (Type %s) \n", 
			getPrintPrefix().c_str(), 
			instructionType, TessaBinaryOpNames[op], 
			_leftOperand->getOperandString().c_str(), _rightOperand->getOperandString().c_str(),
			getType()->toString().data()
			);

		printf("%s", buffer);
	}

	BinaryInstruction* BinaryInstruction::clone(MMgc::GC *gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaValue* clonedLeftOperand = (TessaValue*) originalToCloneMap->get(this->_leftOperand);
		TessaValue* clonedRightOperand = (TessaValue*) originalToCloneMap->get(this->_rightOperand);
		BinaryInstruction* clonedInstruction = new (gc) BinaryInstruction(this->op, clonedLeftOperand, clonedRightOperand, insertCloneAtEnd);
		clonedInstruction->setType(this->getType());
		return clonedInstruction;
	}

	List<TessaValue*, LIST_GCObjects>* BinaryInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operands = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operands->add(_leftOperand);
		operands->add(_rightOperand);
		return operands;
	}
}