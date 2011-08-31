#include "TessaInstructionheader.h"

namespace TessaInstructions {
	CoerceInstruction::CoerceInstruction(TessaTypes::Type* typeToConvertTo, TessaInstruction* instructionToCoerce, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		TessaAssert(instructionToCoerce != NULL);
		this->setType(typeToConvertTo);
		this->_instructionToCoerce = instructionToCoerce;
		this->useCoerceObjAtom = false;
	}

	CoerceInstruction::CoerceInstruction(const Multiname* multinameToCoerce, TessaInstruction* instructionToCoerce, TessaTypes::Type* typeToConvertTo, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		TessaAssert(instructionToCoerce != NULL);
		this->_instructionToCoerce = instructionToCoerce;
		this->multinameToCoerce = multinameToCoerce;
		this->setType(typeToConvertTo);
		this->useCoerceObjAtom = false;
	}

	bool CoerceInstruction::isCoerce() {
		return true;
	}

	TessaTypes::Type* CoerceInstruction::getTypeToCoerce() {
		return this->getType();
	}

	bool CoerceInstruction::producesValue() {
		return true;
	}

	void CoerceInstruction::print() {
		char buffer[128];
		char typeOfInstruction[32];
		char coerceResult[32];

		if (this->isConvert()) {
			VMPI_snprintf(typeOfInstruction, sizeof(typeOfInstruction), "ConvertInstruction");
		} else {
			VMPI_snprintf(typeOfInstruction, sizeof(typeOfInstruction), "CoerceInstruction");
		}

		if (this->isMultinameCoerce()) {
			avmplus::Stringp coerceType = this->multinameToCoerce->getName();
			StUTF8String utf8String(coerceType );
			const char* stringValue = utf8String.c_str();
			VMPI_snprintf(coerceResult, sizeof(coerceResult), "%s", stringValue);
		} else {
			VMPI_snprintf(coerceResult, sizeof(coerceResult), "%s", this->getTypeToCoerce()->toString().data());
		}

		VMPI_snprintf(buffer, sizeof(buffer), "%s %s (%s) -> %s", this->getPrintPrefix().c_str(),
			typeOfInstruction, _instructionToCoerce->getOperandString().c_str(), coerceResult);

		printf("%s\n", buffer);
	}

	void CoerceInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaInstruction* CoerceInstruction::getInstructionToCoerce() {
		return this->_instructionToCoerce;
	}

	const Multiname* CoerceInstruction::getMultinameToCoerce() {
		return multinameToCoerce;
	}

	void CoerceInstruction::setTypeToCoerce(TessaTypes::Type* newType) {
		AvmAssert(newType != NULL);
		setType(newType);
	}

	bool CoerceInstruction::isMultinameCoerce() {
		if (multinameToCoerce == NULL) {
			return false;
		} else {
			return true;
		}
	}

	CoerceInstruction* CoerceInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		CoerceInstruction* clonedCoerce = new (gc) CoerceInstruction(getType(), _instructionToCoerce, insertCloneAtEnd);
		clonedCoerce->_instructionToCoerce = (TessaInstruction*) originalToCloneMap->get(_instructionToCoerce);
		return clonedCoerce;
	}

	List<TessaValue*, LIST_GCObjects>* CoerceInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(getInstructionToCoerce());
		return operandList;
	}
}