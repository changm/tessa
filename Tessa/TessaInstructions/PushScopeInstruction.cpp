#include "TessaInstructionHeader.h"

namespace TessaInstructions {

	PushScopeInstruction::PushScopeInstruction(TessaInstruction* scopeObject, TessaVM::BasicBlock* insertAtEnd) 
		: ScopeInstruction(insertAtEnd)
	{
		this->_scopeObject = scopeObject;
	}

	void PushScopeInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s PushScope %s", this->getPrintPrefix().c_str(),
			this->_scopeObject->getOperandString().c_str());
		printf("%s\n", buffer);
	}

	void PushScopeInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	TessaInstruction* PushScopeInstruction::getScopeObject() {
		return _scopeObject;
	}

	PushScopeInstruction* PushScopeInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		PushScopeInstruction* clonedInstruction = new (gc) PushScopeInstruction(_scopeObject, insertCloneAtEnd);
		clonedInstruction->_scopeObject = (TessaInstruction*) originalToCloneMap->get(_scopeObject);
		clonedInstruction->setType(this->getType());
		return clonedInstruction;
	}

	List<TessaValue*, LIST_GCObjects>* PushScopeInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operands = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operands->add(getScopeObject());
		return operands;
	}
}