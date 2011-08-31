#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	using namespace TessaVisitors;

	ArrayOfInstructions::ArrayOfInstructions(MMgc::GC* gc, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		instructions = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects> (gc);
	}

	void ArrayOfInstructions::addInstruction(TessaInstruction* instruction) {
		instructions->add(instruction);
	}

	void ArrayOfInstructions::print() {
		std::string outputString;
		outputString += getPrintPrefix();
		outputString += " ArrayOfInstructions [";

		std::string arrayElements;
		for (uint32_t i = 0; i < instructions->size(); i++) {
			outputString += " ";
			outputString += instructions->get(i)->getOperandString();
			outputString += " ";
		}

		outputString += "]";

		printf("%s\n", outputString.c_str());
	}

	void ArrayOfInstructions::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	uint32_t ArrayOfInstructions::size() {
		return instructions->size();
	}

	TessaInstruction* ArrayOfInstructions::getInstruction(uint32_t index) {
		TessaAssert(index <= size());
		return instructions->get(index);
	}

	void ArrayOfInstructions::setInstruction(int index, TessaInstruction* instruction) {
		instructions->set(index, instruction);
	}

	List<TessaInstruction*, avmplus::LIST_GCObjects>* ArrayOfInstructions::getInstructions() {
		return instructions;
	}

	bool ArrayOfInstructions::isArrayOfInstructions() {
		return true;
	}

	ArrayOfInstructions* ArrayOfInstructions::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		ArrayOfInstructions* clonedArray = new (gc) ArrayOfInstructions(gc, insertCloneAtEnd);
		for (uint32_t i = 0; i < size(); i++) {
			TessaInstruction* clonedOperand = (TessaInstruction*) originalToCloneMap->get(this->getInstruction(i));
			TessaAssert(clonedOperand != NULL);
			clonedArray->addInstruction(clonedOperand);
		}

		return clonedArray;
	}

	List<TessaValue*, LIST_GCObjects>* ArrayOfInstructions::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		for (uint32_t i = 0; i < size(); i++) {
			//TessaInstruction* operand = (TessaInstruction*) getInstruction(i); ->resolve();
			TessaInstruction* operand = (TessaInstruction*) getInstruction(i); 
			operandList->add(operand);
		}

		return operandList;
	}
}