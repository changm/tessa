#include "TessaInstructionheader.h"
#include "BasicBlock.h"	// Need to include to avoid circular dependencies

namespace TessaInstructions {
	ParameterInstruction::ParameterInstruction(TessaInstruction* defaultValue, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction()
	{
		setDefaultValue(defaultValue);

		/***
		 * Parameter instructions are special for basic blocks and must be recognized as such
		 * if we insert at end via the TEssaInstruction constructor
		 * we lose the fact that this instruction is a parameter one
		 */
		insertAtEnd->addInstruction(this);
	}

	bool ParameterInstruction::isParameter() {
		return true;
	}

	void ParameterInstruction::setDefaultValue(TessaInstruction* defaultValue) {
		this->defaultValue = defaultValue;
	}

	TessaInstruction* ParameterInstruction::getDefaultValue() {
		return this->defaultValue;
	}

	void ParameterInstruction::print() {
		char buffer[128];
		TessaInstruction* realInstruction = this->resolve();
		VMPI_snprintf(buffer, sizeof(buffer), "%s Parameter -> %s (Type %s)", this->getPrintPrefix().c_str(), realInstruction->getOperandString().c_str(), 
			getType()->toString().data());
		printf("%s\n", buffer);
	}

	void ParameterInstruction::visit(TessaVisitorInterface *tessaVisitor) {
		return tessaVisitor->visit(this);
	}

	bool ParameterInstruction::producesValue() {
		return true;
	}

	List<TessaValue*, LIST_GCObjects>* ParameterInstruction::getOperands(MMgc::GC* gc) {
		TessaInstruction* resolvedInstruction = this->resolve();
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(resolvedInstruction);
		return operandList;
	}

	ParameterInstruction* ParameterInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		ParameterInstruction* clonedInstruction = new (gc) ParameterInstruction(this->defaultValue, insertCloneAtEnd);
		TessaInstruction* forwardInstruction = getForwardedInstruction();
		if (forwardInstruction != NULL) {
			forwardInstruction = forwardInstruction->resolve();
		}

		clonedInstruction->setForwardedInstruction((TessaInstruction*) originalToCloneMap->get(forwardInstruction));
		clonedInstruction->setType(this->getType());
		return clonedInstruction;
	}
}