#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	FindPropertyInstruction::FindPropertyInstruction(int scopeDepth, const Multiname* name, bool strict, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->_scopeDepth = scopeDepth;
		this->_name = name;
		this->_isFindDef = false;
		this->useFindDefCache = false;
	}

	FindPropertyInstruction::FindPropertyInstruction(const Multiname *name, uint32_t nameIndex, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->_name = name;
		this->_nameIndex = nameIndex;
		this->_isFindDef = true;
		this->useFindDefCache = false;
	}

	void FindPropertyInstruction::print() {
		avmplus::Stringp propertyName = this->_name->getName();
		StUTF8String utf8String(propertyName);
		const char* stringValue = utf8String.c_str();

		char buffer[512];
		VMPI_snprintf(buffer, sizeof(buffer), "%s FindProperty %s (Type %s)\n", getPrintPrefix().c_str(), 
			stringValue, getType()->toString().data());
		printf("%s", buffer);
	}

	uint32_t FindPropertyInstruction::getNameIndex() {
		return _nameIndex;
	}

	bool FindPropertyInstruction::isFindDefinition() {
		return this->_isFindDef;
	}

	void FindPropertyInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	const Multiname* FindPropertyInstruction::getMultiname() {
		return this->_name;
	}

	bool FindPropertyInstruction::isStrict() {
		return _strictLookup;
	}

	int FindPropertyInstruction::getScopeDepth() {
		return _scopeDepth;
	}

	void FindPropertyInstruction::setCacheSlot(int cacheSlot) {
		this->useFindDefCache = true;
		this->_cacheSlot = cacheSlot;
	}

	int FindPropertyInstruction::getCacheSlot() {
		return this->_cacheSlot;
	}

	FindPropertyInstruction* FindPropertyInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		if (_isFindDef) {
			TessaAssert(useFindDefCache);
			FindPropertyInstruction* clonedFindDefinition = new (gc) FindPropertyInstruction(_name, _nameIndex, insertCloneAtEnd);
			clonedFindDefinition->setCacheSlot(_cacheSlot);
			clonedFindDefinition->setType(this->getType());
			return clonedFindDefinition;
		} else {
			TessaAssert(false);
		}
	}

	List<TessaValue*, LIST_GCObjects>* FindPropertyInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		operandList->add(this);
		return operandList;
	}
}