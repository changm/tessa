#include "TessaInstructionheader.h"

namespace TessaInstructions {
	TypeOfInstruction::TypeOfInstruction(TessaInstruction* objectToTest, TessaInstruction* typeToCompare, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->lateCheck = true;
		this->objectToTest = objectToTest;
		this->typeToCompare = typeToCompare;
	}

	TypeOfInstruction::TypeOfInstruction(TessaInstruction* objectToTest, const Multiname* multiname, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->objectToTest = objectToTest;
		this->lateCheck = false;
		this->multiname = multiname;
	}

	bool TypeOfInstruction::isLateCheck() {
		return this->lateCheck;
	}
	
	void TypeOfInstruction::print() {
		TessaAssert(lateCheck);
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s TypeOf %s == %s", this->getPrintPrefix().c_str(),
			this->objectToTest->getOperandString().c_str(), this->typeToCompare->getOperandString().c_str());
		printf("%s\n", buffer);
	}

	void TypeOfInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaInstruction* TypeOfInstruction::getObjectToTest() {
		return this->objectToTest;
	}

	TessaInstruction* TypeOfInstruction::getTypeToCompare() {
		return this->typeToCompare;
	}

	// Multiname is only applicable in late type checks
	const Multiname* TypeOfInstruction::getMultiname() {
		TessaAssert(lateCheck);
		return multiname;
	}

	bool TypeOfInstruction::producesValue() {
		return true;
	}

	List<TessaValue*, LIST_GCObjects>* TypeOfInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getObjectToTest());
		operandList->add(getTypeToCompare());
		return operandList;
	}
}