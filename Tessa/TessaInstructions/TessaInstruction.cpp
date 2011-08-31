
#include "TessaInstructionHeader.h"
#include "BasicBlock.h"			// To avoid circular dependencies

namespace TessaInstructions {
	TessaInstruction::TessaInstruction() 
		: TessaValue()
	{
	}

	TessaInstruction::TessaInstruction(BasicBlock* basicBlockToInsert) 
		: TessaValue()
	{
		basicBlockToInsert->addInstruction(this);
	}

	bool TessaInstruction::isInstruction() {
		return true;
	}

	bool TessaInstruction::modifiesMemory() {
		return false;
	}

	bool TessaInstruction::isCondition() {
		return false;
	}

	bool TessaInstruction::isBinary() {
		return false;
	}

	bool TessaInstruction::isUnary() {
		return false;
	}

	bool TessaInstruction::isLabel() {
		return false;
	}

	bool TessaInstruction::isPhi() {
		return false;
	}

	bool TessaInstruction::isArrayAccess() {
		return false;
	}

	bool TessaInstruction::isReturn() {
		return false;
	}


	bool TessaInstruction::isBranch() {
		return false;
	}

	bool TessaInstruction::isConditionalBranch() {
		return false;
	}

	bool TessaInstruction::isUnconditionalBranch() {
		return false;
	}

	bool TessaInstruction::isParameter() {
		return false;
	}

	bool TessaInstruction::isCall() {
		return false;
	}

	bool TessaInstruction::isCallStatic() {
		return false;
	}

	bool TessaInstruction::isCallVirtual() {
		return false;
	}

	bool TessaInstruction::isInlineMethod() {
		return false;
	}

	bool TessaInstruction::isConstruct() {
		return false;
	}

	bool TessaInstruction::isNewObject() {
		return false;
	}

	bool TessaInstruction::isNewArray() {
		return false;
	}

	bool TessaInstruction::isConstructProperty() {
		return false;
	}

	bool TessaInstruction::isCoerce() {
		return false;
	}

	bool TessaInstruction::isConvert() {
		return false;
	}

	bool TessaInstruction::modifiesScopeStack() {
		return false;
	}

	bool TessaInstruction::isGetProperty() {
		return false;
	}

	bool TessaInstruction::isSetProperty() {
		return false;
	}

	bool TessaInstruction::isInitProperty() {
		return false;
	}

	bool TessaInstruction::isPropertyAccess() {
		return false;
	}

	bool TessaInstruction::isLocalVariableAccess() {
		return false;
	}

	bool TessaInstruction::isSlotAccess() {
		return false;
	}

	bool TessaInstruction::isConstantValue() {
		return false;
	}

	bool TessaInstruction::isSuper() {
		return false;
	}

	bool TessaInstruction::isConstructSuper() {
		return false;
	}

	bool TessaInstruction::isCallSuper() {
		return false;
	}

	bool TessaInstruction::isThis() {
		return false;
	}

	bool TessaInstruction::isTypeOf() {
		return false;
	}

	bool TessaInstruction::isSwitch() {
		return false;
	}

	bool TessaInstruction::isCase() {
		return false;
	}

	bool TessaInstruction::isLoadVirtualMethod() {
		return false;
	}

	bool TessaInstruction::isNewActivation() {
		return false;
	}

	bool TessaInstruction::isArrayOfInstructions() {
		return false;
	}

	bool TessaInstruction::hasSideEffect() {
		return false;
	}

	TessaInstruction* TessaInstruction::clone() {
		TessaAssert (false);
		printf("Wrong");
		return this;
	}

	void TessaInstruction::print() {
		TessaAssertMessage(false, "Should never print base tessa instruction");
	}

	string TessaInstruction::getPrintPrefix() {
		// For some reason can't add ":" string at the end of the first line
		char buffer[32];
		VMPI_snprintf(buffer, sizeof(buffer), "%c%d :", ReferenceChar, getValueId());
		return buffer;
	}

	string TessaInstruction::getOperandString() {
		char buffer[32];
		VMPI_snprintf(buffer, sizeof(buffer), "%c%d", ReferenceChar, getValueId());
		return buffer;
	}

	BasicBlock* TessaInstruction::getInBasicBlock() {
		return inBasicBlock;
	}

	void TessaInstruction::setInBasicBlock(BasicBlock* basicBlock) {
		this->inBasicBlock = basicBlock;
	}

	/***
	 * follows the forward chain to the end
	 */
	TessaInstruction* TessaInstruction::resolve() {
		if (forwardInstruction == NULL) {
			return this;
		} else {
			return forwardInstruction->resolve();
		}
	}

	void TessaInstruction::setForwardedInstruction(TessaInstruction* instruction) {
		this->forwardInstruction = instruction;
	}

	/***
	 * Follows the forward chain only ONCE
	 */
	TessaInstruction* TessaInstruction::getForwardedInstruction() {
		return this->forwardInstruction;
	}

	bool TessaInstruction::isBlockTerminator() {
		return isReturn() || isBranch() || isSwitch();
	}

	std::string	TessaInstruction::toString() {
		return getOperandString();
	}

	TessaInstruction* TessaInstruction::getClonedValue(TessaValue* originalValue, MMgc::GCHashtable* originalToCloneMap) {
		TessaInstruction* clonedValue = (TessaInstruction*) originalToCloneMap->get(originalValue);
		AvmAssert(clonedValue != NULL);
		return clonedValue;
	}

	TessaInstruction* TessaInstruction::clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd) {
		TessaAssertMessage(false, "Should not be cloning in TessaInstructions");
		return this;
	}

	List<TessaValue*, LIST_GCObjects>* TessaInstruction::getOperands(MMgc::GC* gc) {
		AvmAssertMsg(false, "No operands on basic instruction\n");
		return NULL;
	}

	List<TessaInstruction*, LIST_GCObjects>* TessaInstruction::getOperandsAsInstructions(MMgc::GC* gc) {
		List<TessaInstruction*, LIST_GCObjects>* instructionList = new (gc) List<TessaInstruction*, LIST_GCObjects>(gc);
		List<TessaValue*, LIST_GCObjects>* operandList = getOperands(gc);

		for (uint32_t i = 0; i < operandList->size(); i++) {
			TessaValue* currentValue = operandList->get(i);
			if (currentValue->isInstruction()) {
				instructionList->add(currentValue->toInstruction());
			} else {
				AvmAssert(false);
			}
		}

		return instructionList;
	}
};