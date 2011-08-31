#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	InlineBeginInstruction::InlineBeginInstruction(LoadVirtualMethodInstruction* loadVirtualMethod, Traits* resultTraits, int methodId, int callerMaxScopeSize, int inlineeScopeSize, MethodInfo* inlinedMethod, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		_loadVirtualMethod = loadVirtualMethod;
		_resultTraits = resultTraits;
		_methodId = methodId;
		_scopeStackIncreaseSize = inlineeScopeSize;
		_callerScopeSize = callerMaxScopeSize;

		TessaAssert(inlinedMethod != NULL);
		_inlinedMethod = inlinedMethod;
	}

	void InlineBeginInstruction::print() {
		char buffer[128];
		VMPI_snprintf(buffer, sizeof(buffer), "%s InlineBegin LoadVirtual %s", 
			this->getPrintPrefix().c_str(), _loadVirtualMethod->getOperandString().data());
		printf("%s\n",  buffer);
	}

	void InlineBeginInstruction::visit(TessaVisitorInterface* visitor) {
		visitor->visit(this);
	}

	LoadVirtualMethodInstruction* InlineBeginInstruction::getLoadedMethod() {
		return _loadVirtualMethod;
	}

	InlineBeginInstruction*	InlineBeginInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		LoadVirtualMethodInstruction* clonedReceiver = (LoadVirtualMethodInstruction*) originalToCloneMap->get(_loadVirtualMethod);
		InlineBeginInstruction* inlineBeginInstruction = new (gc) InlineBeginInstruction(clonedReceiver, _resultTraits, _methodId, _callerScopeSize, _scopeStackIncreaseSize, _inlinedMethod, insertCloneAtEnd);
		inlineBeginInstruction->setType(getType());
		return inlineBeginInstruction;
	}

	Traits*	InlineBeginInstruction::getResultTraits() {
		return _resultTraits;
	}

	int	InlineBeginInstruction::getmethodId() {
		return _methodId;
	}

	int InlineBeginInstruction::getIncreaseInScopeStack() {
		return _scopeStackIncreaseSize;
	}

	List<TessaValue*, LIST_GCObjects>* InlineBeginInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(_loadVirtualMethod);
		return operandList;
	}
}