
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	NewArrayInstruction::NewArrayInstruction(List<TessaInstruction*, avmplus::LIST_GCObjects>* arrayElements, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->arrayElements = arrayElements;
	}

	void NewArrayInstruction::print() {
		printf("%s NewArrayInstruction (Type %s)\n", this->getPrintPrefix().c_str(), 
			getType()->toString().data());
	}

	void NewArrayInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	void NewArrayInstruction::addElement(TessaInstruction* element) {
		arrayElements->add(element);
	}

	TessaInstruction* NewArrayInstruction::getElement(uint32_t index) {
		return arrayElements->get(index);
	}

	List<TessaInstruction*, avmplus::LIST_GCObjects> * NewArrayInstruction::getArrayElements() {
		return arrayElements;
	}

	int NewArrayInstruction::numberOfElements() {
		return arrayElements->size();
	}

	bool NewArrayInstruction::producesValue() {
		return true;
	}

	bool NewArrayInstruction::isNewArray() {
		return true;
	}

	List<TessaValue*, LIST_GCObjects>* NewArrayInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		List<TessaInstruction*, LIST_GCObjects>* arrayElements = getArrayElements();
		for (uint32_t i = 0; i < arrayElements->size(); i++) {
			operandList->add(arrayElements->get(i));
		}
		return operandList;
	}
}