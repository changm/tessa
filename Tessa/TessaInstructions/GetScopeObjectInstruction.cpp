#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	GetScopeObjectInstruction::GetScopeObjectInstruction(int32_t scopeIndex, bool lookInOuterScope, TessaVM::BasicBlock* insertAtEnd) 
		: ScopeInstruction(insertAtEnd)
	{
		this->scopeIndex = scopeIndex;
		this->lookInOuterScope = lookInOuterScope;
	}

	void GetScopeObjectInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s GetScopeObject %d", this->getPrintPrefix().c_str(), scopeIndex);
		printf("%s\n", buffer);
	}

	void GetScopeObjectInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		return tessaVisitor->visit(this);
	}

	int32_t GetScopeObjectInstruction::getScopeIndex() {
		return scopeIndex;
	}

	bool GetScopeObjectInstruction::isOuterScope() {
		return lookInOuterScope;
	}

	GetScopeObjectInstruction*	GetScopeObjectInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		GetScopeObjectInstruction* clonedGetScopeObject = new (gc) GetScopeObjectInstruction(scopeIndex, lookInOuterScope, insertCloneAtEnd);
		clonedGetScopeObject->setType(this->getType());
		return clonedGetScopeObject;
	}

	List<TessaValue*, LIST_GCObjects>* GetScopeObjectInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(this);
		return operandList;
	}
}