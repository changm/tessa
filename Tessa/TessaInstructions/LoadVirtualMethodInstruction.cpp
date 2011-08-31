#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	LoadVirtualMethodInstruction::LoadVirtualMethodInstruction(MMgc::GC* gc, TessaInstruction* receiverObject, int methodId, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		TessaAssert(receiverObject != NULL);
		_receiverObject = receiverObject;
		_methodId = methodId;

		_loadedMethodEnv = new (gc) TessaValue(TypeFactory::getInstance()->pointerType());
		_loadedMethodInfo = new (gc) TessaValue(TypeFactory::getInstance()->pointerType());
	}

	void LoadVirtualMethodInstruction::print() {
		printf("%s LoadVirtualMethod %s[method %d] (Env %s, Info %s)\n", getPrintPrefix().c_str(), _receiverObject->getOperandString().c_str(), _methodId,
			_loadedMethodEnv->toString().data(), _loadedMethodInfo->toString().data()
			);
	}

	void LoadVirtualMethodInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	int LoadVirtualMethodInstruction::getMethodId() {
		return _methodId;
	}

	TessaInstruction* LoadVirtualMethodInstruction::getReceiverObject() {
		return _receiverObject;
	}

	LoadVirtualMethodInstruction* LoadVirtualMethodInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaInstruction* receiverObjectClone = (TessaInstruction*) originalToCloneMap->get(_receiverObject);
		LoadVirtualMethodInstruction* clone = new (gc) LoadVirtualMethodInstruction(gc, receiverObjectClone, _methodId, insertCloneAtEnd);

		/*
		TessaValue* clonedLoadedMethodEnv = new (gc) TessaValue(TypeFactory::getInstance()->pointerType());
		TessaValue* clonedLoadedMethodInfo = new (gc) TessaValue(TypeFactory::getInstance()->pointerType());
		clone->_loadedMethodEnv = clonedLoadedMethodEnv;
		clone->_loadedMethodInfo = clonedLoadedMethodInfo;
		*/

		/***
		 * Since the LoadVirtualMethodInstruction outputs more than one value
		 * we have to manually stuff the map with the cloned values as well.
		 */
		originalToCloneMap->put(getLoadedMethodInfo(), clone->_loadedMethodInfo);
		originalToCloneMap->put(getLoadedMethodEnv(), clone->_loadedMethodEnv);

		clone->setType(this->getType());
		return clone;
	}

	TessaValue*	LoadVirtualMethodInstruction::getLoadedMethodEnv() {
		return _loadedMethodEnv;
	}

	TessaValue*	LoadVirtualMethodInstruction::getLoadedMethodInfo() {
		return _loadedMethodInfo;
	}

	bool LoadVirtualMethodInstruction::isLoadVirtualMethod() {
		return true;
	}

	List<TessaValue*, LIST_GCObjects>* LoadVirtualMethodInstruction::getOperands(MMgc::GC* gc) {
		List<TessaValue*, LIST_GCObjects>* operandList = new (gc) List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(_loadedMethodInfo);
		operandList->add(_loadedMethodEnv);
		return operandList;
	}
}